#pragma once
#ifndef FG_INC_RESOURCE_CONFIG_JSON
#define FG_INC_RESOURCE_CONFIG_JSON

#include <util/Vector.hpp>
#include <util/UniversalId.hpp>
#include <util/Util.hpp>
#include <resource/ResourceConfig.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace resource
{
    inline void to_json(json &output, const ResourceHeader &input)
    {
        auto type = util::UniversalId<void>::name(input.type);
        auto quality = getQualityName(input.quality);
        auto mapping = json::object();
        for (auto &it : input.fileMapping)
            mapping[getQualityName(it.first).data()] = it.second;
        output = {
            {"name", input.name},
            {"flags", input.flags},
            {"config", input.config},
            {"type", type},
            {"quality", quality},
            {"mapping", mapping}};
    } //# to_json ResourceHeader
    //#-----------------------------------------------------------------------------------

    inline void from_json(const json &input, ResourceHeader &output)
    {
        if (!input.is_object() || input.is_null())
            return;
        if (input.contains("mapping"))
        {
            auto &items = input.at("mapping").items();
            for (auto &it : items)
            {
                auto &key = it.key();
                if (isQualityTextValid(key))
                {
                    auto quality = getQualityFromText(key);
                    output.fileMapping.emplace(quality, it.value().get<std::string>());
                }
            }
        }
        if (input.contains("name"))
            input.at("name").get_to(output.name);
        if (input.contains("flags"))
            input.at("flags").get_to(output.flags);
        if (input.contains("config"))
            input.at("config").get_to(output.config);
        if (input.contains("type"))
            output.type = util::UniversalId<void>::id(input.at("type").get<std::string_view>());
        if (input.contains("quality"))
        {
            auto quality = input.at("quality").get<std::string_view>();
            if (isQualityTextValid(quality))
                output.quality = getQualityFromText(quality);
        }
    } //# from_json ResourceHeader
    //#-----------------------------------------------------------------------------------

    inline void to_json(json &output, const ResourceConfig &input)
    {
        auto type = util::UniversalId<void>::name(input.header.type);
        // on top convert only meaningful keys (name & type)
        // everything else comes from mapping
        output = {{"name", input.header.name},
                  {"type", type}};
        for (auto &it : input.mapping)
        {
            output[it.first] = it.second;
        } //> for each mapped resource header
    }     //# to_json ResourceConfig
    //#-----------------------------------------------------------------------------------

    inline void from_json(const json &input, ResourceConfig &output)
    {
        static util::StringVector acceptedKeys = {"name", "type"};
        static util::StringVector rejectedKeys = {"flags", "config", "quality", "mapping"};
        if (!input.is_object() || input.is_null())
            return; // cannot do anything
        auto &items = input.items();
        for (auto &it : items)
        {
            auto &key = it.key();
            if (acceptedKeys.contains(key))
            {
                if (strings::equals(key.c_str(), "name"))
                    output.header.name = it.value();
                else if (strings::equals(key.c_str(), "type"))
                    output.header.type = util::UniversalId<void>::id(it.value().get<std::string_view>());
            }
            else if (!rejectedKeys.contains(key))
            {
                output.mapping.emplace(key, it.value());
            }
        }
    } //# from_json ResourceConfig
    //#-----------------------------------------------------------------------------------

} //> namespace resource

#endif //> FG_INC_RESOURCE_CONFIG_JSON