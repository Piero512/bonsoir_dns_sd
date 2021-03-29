#include "service.hpp"
#include <variant>
#include "../utils.hpp"

using namespace Utils;
using namespace flutter;
BonsoirService::BonsoirService(const flutter::EncodableMap &values_map)
{
        name = extractValueOrDefault<std::string>(values_map, "service.name", "");
        type = extractValueOrDefault<std::string>(values_map, "service.type", "");
        port = extractValueOrDefault<int64_t>(values_map, "port",-1);
        auto attr_iterator = values_map.find(flutter::EncodableValue("service.attributes"));
        if (attr_iterator != values_map.end())
        {
            auto attr_map = *attr_iterator;
            attributes = extract_str_map(std::get<flutter::EncodableMap>(attr_map.second));
        }
}

EncodableMap BonsoirService::toEncodableMap(){
    return EncodableMap{
        {"service.name", name},
        {"service.type", type},
        {"service.port", port}
    };
}