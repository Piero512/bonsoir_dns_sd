#pragma once
#include <map>
#include <string>
#include <flutter/standard_message_codec.h>

namespace Utils {
    template<typename T>
    std::string coalesceArrays(std::vector<T> vector);

    std::string coalesceValue(const flutter::EncodableValue &val);
    std::map<std::string, std::string> extract_str_map(const flutter::EncodableMap &map);

      
    template<typename T>
    inline T extractValueOrDefault(const flutter::EncodableMap &map, const std::string &key, T default_value){
        auto iter_to_element = map.find(flutter::EncodableValue(key));
        if (iter_to_element != map.cend() && !iter_to_element->second.IsNull() && std::holds_alternative<T>(iter_to_element->second))
        {
            return std::get<T>(iter_to_element->second);
        }
        return default_value;
    };

    inline int64_t extractIntValueOrDefault(const flutter::EncodableMap& map, const std::string& key, int64_t default_value = -1){
        auto iter_to_element = map.find(flutter::EncodableValue(key));
        if(iter_to_element != map.cend()){
            if(std::holds_alternative<int32_t>(iter_to_element->second)){
                return std::get<int32_t>(iter_to_element->second);
            } else if (std::holds_alternative<int64_t>(iter_to_element->second)){
                return std::get<int64_t>(iter_to_element->second);
            }
        }
        return default_value;
    }
    

};