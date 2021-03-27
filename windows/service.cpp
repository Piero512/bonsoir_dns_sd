#include <flutter/standard_method_codec.h>
#include <variant>
#include <map>

class BonsoirService {
    std::string name;
    std::string type;
    int64_t port;
    std::map<std::string,std::string> attributes;

    BonsoirService(const flutter::EncodableValue& value){
        auto values_map = std::get_if<flutter::EncodableMap>(value);
        if(values_map){
            name = extract_value_or_null<std::string>(values_map, "name");
            type = extract_value_or_null<std::string>(values_map, "type");
            port = extract_value_or_null<int64_t>(values_map, "port") != nullptr ?? -1;
        }
        
    };

    template<typename T>
    static T extract_value_or_null(const flutter::EncodableMap &map, const T& key){
        auto iter_to_element = map.find(flutter::EncodableValue(key));
        if(iter_to_element != map.cend() && !iter_to_element->second.IsNull()){
            return std::get<T>(iter_to_element->second);
        }
        return nullptr;
    }
};