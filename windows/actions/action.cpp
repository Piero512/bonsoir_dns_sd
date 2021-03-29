#include "action.hpp"

using namespace flutter;
int BonsoirAction::sendPendingMessages(){
    // TODO: Acquire mutex to make producer thread pause on mDNSResponder callback.
    int sent = 0;
    for(auto iter = pendingMessages.begin(); iter != pendingMessages.end(); ){
        eventSink->Success(*iter);
        sent++;
        iter = pendingMessages.erase(iter);
    }
    return sent;
}

std::unique_ptr<StreamHandlerError<EncodableValue>> BonsoirAction::OnListenInternal(const EncodableValue* args, std::unique_ptr<EventSink<EncodableValue>>&& events){
    if(eventSink != nullptr){
        eventSink->EndOfStream();
        std::cerr << "Warning: subscribing again. This will close the last subscription" << std::endl;
    } 
    eventSink.reset(events.release());
    return nullptr;
}

std::unique_ptr<StreamHandlerError<EncodableValue>> BonsoirAction::OnCancelInternal(const EncodableValue* arguments){
    if(eventSink != nullptr){
        eventSink->EndOfStream();
        if(printLogs){
            std::cout << "BonsoirAction: canceling stream" << std::endl;
        }
    }
    return nullptr;
}