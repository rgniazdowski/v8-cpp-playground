#pragma once
#ifndef FG_INC_UTIL_JSON_FILE_HELPER
#define FG_INC_UTIL_JSON_FILE_HELPER

#include <util/File.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace util
{
    struct JsonFile
    {
    protected:
        JsonFile(){};

    public:
        static json loadInPlace(std::string_view filePath)
        {
            auto content = util::File::loadInPlace(filePath);
            if (!content)
                return nullptr;
            json jsonFile = json::parse(content);
            delete[] content;
            return jsonFile;
        }
    };
} //> namespace util

#endif //> FG_INC_UTIL_JSON_FILE_HELPER