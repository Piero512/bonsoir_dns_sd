#include "broadcast.hpp"
#include "../constants.h"

using namespace flutter;

BonsoirBroadcast::BonsoirBroadcast(int64_t id, HWND parent, BonsoirService service, std::unique_ptr<EventChannel<EncodableValue>> channel, bool printLogs): 
    BonsoirAction(id, parent,std::move(channel), printLogs), service(service){
        
    }

void BonsoirBroadcast::BroadcastCallback(DNSServiceRef ref, DNSServiceFlags flags, DNSServiceErrorType errorCode, const char* name, const char* regType, const char* domain, void* context){
    auto bcast = static_cast<BonsoirBroadcast*>(context);
    if(errorCode == kDNSServiceErr_NoError){
        if(flags & kDNSServiceFlagsAdd){
            bcast->pendingMessages.emplace_back(EncodableMap{
                {"id", "broadcast_started"},
                {"service", bcast->service.toEncodableMap()}
            });
            if(bcast->printLogs){
                std::cout << "BonsoirBroadcast: Registered service " << name << "." << regType << "." << domain << " successfully" << std::endl;
            }
        } else {
            bcast->pendingMessages.emplace_back(EncodableMap{
                {"id", "broadcast_stopped"},
                {"service", bcast->service.toEncodableMap()}
            });
        }
        if(flags & ~kDNSServiceFlagsMoreComing){
            if(bcast->printLogs){
                std::cout << "BonsoirBroadcast: Sending message to parent window" << std::endl;
            }
            PostMessage(bcast->parentWindow, WM_MDNS_CALLBACK, bcast->id, 0);
        }
    } else {
        std::cerr << "Received error code from mDNS registration callback: " << std::to_string(errorCode) << std::endl;
        bcast->pendingMessages.emplace_back(EncodableMap{
                {"id","unknown"},
                {"service", bcast->service.toEncodableMap()}
            });
        PostMessage(bcast->parentWindow, WM_MDNS_CALLBACK, bcast->id, 0);
    }
}
            
