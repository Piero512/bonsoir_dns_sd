#include "include/bonsoir_dns_sd/bonsoir_dns_sd_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <variant>

namespace {

class BonsoirDnsSdPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  BonsoirDnsSdPlugin();

  virtual ~BonsoirDnsSdPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  static void DecodeInitializeMessage(const flutter::MethodCall<>& method_call);
};

// static
void BonsoirDnsSdPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "bonsoir_dns_sd",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<BonsoirDnsSdPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

BonsoirDnsSdPlugin::BonsoirDnsSdPlugin() {}

BonsoirDnsSdPlugin::~BonsoirDnsSdPlugin() {}

void BonsoirDnsSdPlugin::DecodeInitializeMessage(const flutter::MethodCall<>& method_call){
  
}

void BonsoirDnsSdPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if(method_call.method_name() == "broadcast.initialize"){
    auto args = method_call.arguments();
    if(args == nullptr){
      result->Error("MissingBody", "The call is missing the body of the message to initialize");
    } else {
      auto data = std::get_if<flutter::EncodableMap>(method_call.arguments());
      if(data){
      } else {
        result->Error("ParseError","The body is not a map!");
      }
    }
  } 
}

}  // namespace

void BonsoirDnsSdPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  BonsoirDnsSdPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
