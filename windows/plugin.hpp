// Winsock must be included before Windows.h
#include <Winsock2.h>
#include <windows.h>

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

    class BonsoirDnsSdPlugin : public flutter::Plugin
    {
    public:
        static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
        flutter::PluginRegistrarWindows *registrar;
        int topwindowid;
        bool poll = true;
        std::map<int64_t, std::unique_ptr<BonsoirAction>> activeActions;
#ifdef WIN32
        std::map<SOCKET, DNSServiceRef> mDNSResponderConnections;
#else
        std::map<int, DNSServiceRef> mDNSResponderConnections;
#endif
        std::vector<struct pollfd> poll_struct;

        BonsoirDnsSdPlugin();
        

        virtual ~BonsoirDnsSdPlugin();

        void operator()();


    private:
        // Called when a method is called on this plugin's channel from Dart.
        void HandleMethodCall(
            const flutter::MethodCall<flutter::EncodableValue> &method_call,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
        static std::optional<int64_t> isValidMessage(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> &result);
        std::optional<LRESULT> BonsoirDnsSdPlugin::WindowMessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    };
