#include "include/bonsoir_dns_sd/bonsoir_dns_sd_plugin.h"

// This must be included before many other Windows headers.
#include <Winsock2.h>
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <variant>
#include <thread>
#include <iostream>
#include "actions/action.hpp"
#include "actions/broadcast.hpp"
#include "actions/discovery.hpp"
#include "service/service.hpp"
#include "utils.hpp"
#include "dns_sd.h"
#include "constants.h"

typedef enum {
  BONSOIR_BROADCAST,
  BONSOIR_DISCOVERY
} BonsoirEventType;

namespace {
  using namespace Utils;

class BonsoirDnsSdPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
  flutter::PluginRegistrarWindows * registrar;
  int topwindowid;
  std::map<int64_t, std::unique_ptr<BonsoirAction>> activeActions;
  #ifdef WIN32
  std::map<SOCKET,DNSServiceRef> mDNSResponderConnections;
  #else
  std::map<int, DNSServiceRef> mDNSResponderConnections;
  #endif
  std::vector<struct pollfd> poll_struct;

  BonsoirDnsSdPlugin();
  bool poll = true;

  virtual ~BonsoirDnsSdPlugin();


  void operator()(){
    while(poll){
      auto fd_count = WSAPoll(poll_struct.data(), static_cast<ULONG>(poll_struct.size()), 500);
      if(fd_count > 0) {
        auto pollfd_iter = poll_struct.begin();
        for(; fd_count > 0 && pollfd_iter != poll_struct.end();){
          auto retEvents = pollfd_iter->revents;
          auto ref = mDNSResponderConnections.find(pollfd_iter->fd);
          assert(ref != mDNSResponderConnections.end());
          if(retEvents & POLLIN){
            auto error = DNSServiceProcessResult(ref->second);
            if(error != kDNSServiceErr_NoError){
              std::cerr << "Received error while retrieving result from DNSServiceProcessResult" << std::to_string(error) << std::endl;
              // TODO: Check if should discard the socket and disconnect.
              DNSServiceRefDeallocate(ref->second);
              closesocket(pollfd_iter->fd);
              pollfd_iter = poll_struct.erase(pollfd_iter);
            }
          } else if (retEvents & POLLERR){
            std::cerr << "Received error on mDNS" << std::endl;
            DNSServiceRefDeallocate(ref->second);
            closesocket(pollfd_iter->fd);
            pollfd_iter = poll_struct.erase(pollfd_iter);
          }
        }
      } else if (fd_count == SOCKET_ERROR) {
        std::cerr << "Received error from WinSockets: " << WSAGetLastError() << std::endl;
        // TODO: What to do? 
      }
    } 
  }

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  static std::optional<int64_t> isValidMessage(const flutter::MethodCall<flutter::EncodableValue>& method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result);
  std::optional<LRESULT> BonsoirDnsSdPlugin::WindowMessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

std::optional<int64_t> BonsoirDnsSdPlugin::isValidMessage(const flutter::MethodCall<flutter::EncodableValue>& method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result){
  auto args = method_call.arguments();
    if(args == nullptr){
      result->Error("MissingBody", "The call is missing the body of the message");
    } else {
      auto data = std::get_if<flutter::EncodableMap>(args);
      if(data){
        auto id = extractValueOrDefault<int64_t>(*data, "id", -1);
        if(id != -1){
          return id;
        }
        result->Error("MissingFields", "The body doesn't contain a valid ID number");
        return std::nullopt;
      } else {
        result->Error("ParseError","The body is not a map!");
      }
    }
    return std::nullopt;
}

std::optional<LRESULT> BonsoirDnsSdPlugin::WindowMessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
  if(message == WM_MDNS_CALLBACK){
      auto id = static_cast<int64_t>(wparam);
      auto action_iter = activeActions.find(id);
      if(action_iter != activeActions.cend()){
        auto& action_ptr = action_iter->second;
        auto sent = action_ptr->sendPendingMessages();
        if(action_ptr->printLogs){
          std::cout << "BonsoirBroadcast: Sent " << sent << " events to Dart";
        }
        return 0;
      } else {
        return -1;
      }
  }
  return std::nullopt;
}

// static
void BonsoirDnsSdPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "fr.skyost.bonsoir",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<BonsoirDnsSdPlugin>();
  plugin->registrar = registrar;
  // TODO: Change this to a task queue when it exists on Flutter desktop. 
  auto registration_id = registrar->RegisterTopLevelWindowProcDelegate([&](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
    return plugin->WindowMessageHandler(hwnd, message, wparam, lparam);
  });
  plugin->topwindowid = registration_id;

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

BonsoirDnsSdPlugin::BonsoirDnsSdPlugin() {}

BonsoirDnsSdPlugin::~BonsoirDnsSdPlugin() {
  registrar->UnregisterTopLevelWindowProcDelegate(topwindowid);
}

void BonsoirDnsSdPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if(method_call.method_name() == "broadcast.initialize"){
    auto id = isValidMessage(method_call, result);
    if(id){
      auto data = std::get_if<flutter::EncodableMap>(method_call.arguments());
      auto svcstr = std::string("fr.skyost.bonsoir.broadcast.") + std::to_string(*id);
      auto printLogs = extractValueOrDefault<bool>(*data, "printLogs", false);
      BonsoirService serviceToAnnounce(*data);
      activeActions.emplace(*id,std::make_unique<BonsoirBroadcast>(*id, registrar->GetView()->GetNativeWindow(),std::move(serviceToAnnounce), std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(registrar->messenger(), svcstr, &flutter::StandardMethodCodec::GetInstance()), printLogs));
      result->Success();
    }
  } else if (method_call.method_name() == "broadcast.start"){
        auto id = isValidMessage(method_call, result);
    if (id && *id != -1 && activeActions.find(*id) != activeActions.end()) {
      DNSServiceRef ref;
      auto broadcast = dynamic_cast<BonsoirBroadcast*>(activeActions[*id].get());
      auto service = broadcast->service;
      auto error = DNSServiceRegister(&ref, kDNSServiceFlagsShareConnection, 0, service.name.c_str(), service.type.c_str(), nullptr, nullptr, static_cast<uint16_t>(service.port), 0, nullptr, BonsoirBroadcast::BroadcastCallback, broadcast);
      if(error == kDNSServiceErr_NoError){
        broadcast->actionRef = ref;
        auto fd = DNSServiceRefSockFD(ref);
        struct pollfd poll_ref;
        poll_ref.events = POLLIN | POLLERR;
        poll_ref.fd = fd;
        poll_struct.push_back(poll_ref);
        mDNSResponderConnections.insert(std::make_pair(fd, ref));
        result->Success();
      } else {
        result->Error("mDNSError", "mDNS has returned the error: " + std::to_string(error));
      }
    } else {
      result->Error("UnknownID", "There is no broadcast with that ID!");
    }
  } else if (method_call.method_name() == "broadcast.stop"){
    auto id = isValidMessage(method_call, result);
    if(id && *id != -1 && activeActions.find(*id) != activeActions.end()){
      auto bcast_iter = activeActions.find(*id);
      auto broadcast = dynamic_cast<BonsoirBroadcast*>(activeActions[*id].get());
      auto fd = DNSServiceRefSockFD(broadcast->actionRef);
      auto conn_iter =  mDNSResponderConnections.find(fd);
      assert(conn_iter != mDNSResponderConnections.end());
      assert(bcast_iter != activeActions.end());
      activeActions.erase(bcast_iter);
      mDNSResponderConnections.erase(conn_iter);
      result->Success();
    }
    result->NotImplemented();
  } else if(method_call.method_name() == "discovery.initialize"){

  } else {
    result->NotImplemented();
  }
}

}  // namespace

void BonsoirDnsSdPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  BonsoirDnsSdPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
