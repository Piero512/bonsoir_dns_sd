#pragma once
#include "action.hpp"
#include "dns_sd.h"

class BonsoirDiscovery: public BonsoirAction{
    std::string type;
    std::map<std::string,DNSServiceRef> pendingResolvers;
    ~BonsoirDiscovery() {
        for(auto iter = pendingResolvers.begin(); iter != pendingResolvers.end();){
            DNSServiceRefDeallocate(iter->second);
            iter = pendingResolvers.erase(iter);
        }
    }
    BonsoirDiscovery(int64_t id, HWND parent, std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> channel, std::string type, bool printLogs = true);

    static void DiscoveryCallback(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* serviceName, const char* regtype, const char* replyDomain, void* context);

    static void ResolveCallback(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullName, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context);

};