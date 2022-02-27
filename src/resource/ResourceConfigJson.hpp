#pragma once
#ifndef FG_INC_RESOURCE_CONFIG_JSON
#define FG_INC_RESOURCE_CONFIG_JSON

#include <util/Tag.hpp>
#include <resource/ResourceConfig.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace resource
{
    void to_json(json &output, const ResourceHeader &input)
    {
        auto type = util::Tag<void>::name(input.type);
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

    void from_json(const json &input, ResourceHeader &output)
    {
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
            output.type = util::Tag<void>::id(input.at("type").get<std::string_view>());
        if (input.contains("quality"))
        {
            auto quality = input.at("quality").get<std::string_view>();
            if (isQualityTextValid(quality))
                output.quality = getQualityFromText(quality);
        }
    } //# from_json ResourceHeader
    //#-----------------------------------------------------------------------------------

    void to_json(json &output, const ResourceConfig &input)
    {
    } //# to_json ResourceConfig
    //#-----------------------------------------------------------------------------------

    void from_json(const json &input, ResourceConfig &output)
    {
        if (!input.is_object())
            return; // cannot do anything
        auto &items = input.items();
        for (auto &item : items)
        {
        }
    } //# from_json ResourceConfig
    //#-----------------------------------------------------------------------------------

} //> namespace resource

#endif //> FG_INC_RESOURCE_CONFIG_JSON