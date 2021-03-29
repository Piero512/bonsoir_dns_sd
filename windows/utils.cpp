#include "utils.hpp"
#include <string>
#include <sstream>
#include <map>
#include <flutter/standard_message_codec.h>

namespace Utils{
template <typename T>
    std::string coalesceArrays(std::vector<T> vector)
    {
        std::stringstream listbuffer;
        listbuffer << "[";
        for (auto &listval : vector)
        {
            listbuffer << std::to_string(listval);
            listbuffer << ",";
        }
        listbuffer << "]";
        return listbuffer.str();
    }

    std::string coalesceValue(const flutter::EncodableValue &val)
    {
        switch (val.index())
        {
        case 0: // NULL
            return "NULL";
        case 1: // bool
            return "true";
        case 2: // int32_t
        case 3: // int64_t
            return std::to_string(const_cast<flutter::EncodableValue&>(val).LongValue());
        case 4: // double
            return std::to_string(std::get<double>(val));
        case 5: // std::string
            return std::get<std::string>(val);
        case 6: // std::vector<uint8_t>
            return coalesceArrays(std::get<std::vector<uint8_t>>(val));
        case 7: // std::vector<int32_t>
            return coalesceArrays(std::get<std::vector<int32_t>>(val));
        case 8: // std::vector<int64_t>
            return coalesceArrays(std::get<std::vector<int64_t>>(val));
        case 9: // std::vector<double>
            return coalesceArrays(std::get<std::vector<double>>(val));
        case 10: // EncodableList
        {
            std::stringstream listbuffer;
            listbuffer << "[";
            for (auto &listval : std::get<flutter::EncodableList>(val))
            {
                listbuffer << coalesceValue(listval);
                listbuffer << ",";
            }
            listbuffer << "]";
            return listbuffer.str();
        }
        case 11: // EncodableMap
        {
            std::stringstream listbuffer;
            listbuffer << "{";
            for (auto &listval : std::get<flutter::EncodableMap>(val))
            {
                listbuffer << coalesceValue(const_cast<flutter::EncodableValue &>(listval.first));
                listbuffer << " : ";
                listbuffer << coalesceValue(const_cast<flutter::EncodableValue &>(listval.second));
            }
            listbuffer << "}";
            return listbuffer.str();
        }
        case 12: // CustomEncodableValue
        default:
            return "Unknown value!";
        }
    }

    std::map<std::string, std::string> extract_str_map(const flutter::EncodableMap &map)
    {
        std::map<std::string, std::string> toReturn;
        for (const auto &entry : map)
        {
            std::string first;
            std::string second;
            if (std::holds_alternative<std::string>(entry.first))
            {
                first = std::get<std::string>(entry.first);
            }
            else
            {
                first = coalesceValue(entry.first);
            }
            if (std::holds_alternative<std::string>(entry.second)){
                second = std::get<std::string>(entry.second);
            } else {
                second = coalesceValue(entry.second);
            }
            toReturn.emplace(first,second);
        }
        return toReturn;
    }
}
