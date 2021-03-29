#pragma once

#include <flutter/standard_message_codec.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include "../service/service.hpp"
#include "action.hpp"
#include "dns_sd.h"

class BonsoirBroadcast: public BonsoirAction 
{
public:
    BonsoirService service;
    BonsoirBroadcast(int64_t id, HWND parent, BonsoirService service, std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> channel, bool printLogs = true);
    static void BroadcastCallback(DNSServiceRef ref, DNSServiceFlags flags, DNSServiceErrorType errorCode, const char* name, const char* regType, const char* domain, void* context);
};