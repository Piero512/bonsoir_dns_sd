#include "discovery.hpp"
#include <sstream>
using namespace flutter;


BonsoirDiscovery::BonsoirDiscovery(int64_t id,HWND parent, std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> channel, std::string type, bool printLogs): 
BonsoirAction(id,parent, std::move(channel), printLogs), type(type)
{

}

std::string buildSvcName(const char* serviceName, const char* regtype, const char* replyDomain){
    std::stringstream svcbuf;
    svcbuf << serviceName;
    svcbuf << ".";
    svcbuf << regtype;
    svcbuf << ".";
    svcbuf << replyDomain;
    return svcbuf.str();
}

void BonsoirDiscovery::DiscoveryCallback(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* serviceName, const char* regtype, const char* replyDomain, void* context){
    auto discover = static_cast<BonsoirDiscovery*>(context);
    if(errorCode == kDNSServiceErr_NoError){
        auto completeSvcName = buildSvcName(serviceName, regtype, replyDomain);
        if(flags & kDNSServiceFlagsAdd){
            if(discover->printLogs){
                std::cout << "BonsoirDiscovery: Service added: " << completeSvcName << std::endl;
            }
            auto service = EncodableMap{
                    {"service.name", serviceName},
                    {"service.type", regtype},
                    {"service.port", -1},
                };
            discover->pendingMessages.emplace_back(EncodableMap{
                {"id", "discovery_service_added"},
                {"service", service}
            });
            DNSServiceRef newResolveRef;
            auto err = DNSServiceResolve(&newResolveRef, 0, interfaceIndex, serviceName, regtype, replyDomain, BonsoirDiscovery::ResolveCallback, discover);
            if(err == kDNSServiceErr_NoError){
                discover->pendingResolvers.emplace(completeSvcName,newResolveRef);
                
            }
        } else {
            if(discover->printLogs){
                std::cout << "BonsoirDiscovery: Service removed: " << completeSvcName << std::endl;
            }
            auto iter = discover->pendingResolvers.find(completeSvcName);
            if(iter != discover->pendingResolvers.end()){
                DNSServiceRefDeallocate(iter->second);
                discover->pendingResolvers.erase(iter);
            }
        }
    }
}
void BonsoirDiscovery::ResolveCallback(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* fullName, const char* hosttarget, uint16_t port, uint16_t txtLen, const unsigned char* txtRecord, void* context){

}