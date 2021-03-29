#pragma once

#include <flutter/standard_message_codec.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include "dns_sd.h"

class BonsoirAction: public flutter::StreamHandler<flutter::EncodableValue> {
    public:
    int64_t id;
    bool printLogs;
    DNSServiceRef actionRef{};
    HWND parentWindow;
    std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> eventChannel;
    std::vector<flutter::EncodableValue> pendingMessages;
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> eventSink;

    virtual ~BonsoirAction(){
      if(actionRef != nullptr){
        DNSServiceRefDeallocate(actionRef);
      }
      if(eventSink != nullptr){
        eventSink->EndOfStream();
      }
    }
    
    BonsoirAction(int64_t id, HWND parentWindow, std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> channel, bool printLogs = true): 
    id(id),
    parentWindow(parentWindow),
    printLogs(printLogs),
    eventChannel(std::move(channel)){
        eventChannel->SetStreamHandler(std::unique_ptr<flutter::StreamHandler<flutter::EncodableValue>>(this));
    }

    int sendPendingMessages();

    std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> OnListenInternal(const flutter::EncodableValue* arguments,
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) override;

    std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> OnCancelInternal(
      const flutter::EncodableValue* arguments) override;
};
