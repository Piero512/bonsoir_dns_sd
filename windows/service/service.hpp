#pragma once

#include <string>
#include <map>
#include <flutter/standard_method_codec.h>

class BonsoirService
{
public:
    std::string name;
    std::string type;
    int64_t port;
    std::map<std::string, std::string> attributes;

    BonsoirService(const flutter::EncodableMap &value);

    flutter::EncodableMap toEncodableMap();
};