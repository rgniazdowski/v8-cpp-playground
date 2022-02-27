#pragma once
#ifndef FG_INC_RESOURCE_CONFIG
#define FG_INC_RESOURCE_CONFIG

#include <unordered_map>
#include <Quality.hpp>

namespace resource
{
    using ResourceType = unsigned int;
    using QualityFileMapping = std::unordered_map<Quality, std::string>;

    struct ResourceHeader
    {
        std::string name;
        std::string flags;
        std::string config;
        ResourceType type;
        Quality quality;
        QualityFileMapping fileMapping;

        ResourceHeader() : name(), flags(), config(), type(0), quality(Quality::UNIVERSAL), fileMapping() {}
        ResourceHeader(const ResourceHeader &orig)
        {
            name = orig.name;
            flags = orig.flags;
            config = orig.config;
            type = orig.type;
            quality = orig.quality;
            fileMapping = orig.fileMapping;
        }
        ~ResourceHeader()
        {
            type = 0;
            quality = Quality::UNIVERSAL;
            fileMapping.clear();
        }
    }; //# struct ResourceHeader

    struct ResourceConfig
    {
        using Mapping = std::unordered_map<std::string, ResourceHeader>;
        Mapping mapping;
        ResourceHeader header;
        ResourceConfig() : mapping(), header() {}
        ResourceConfig(const ResourceConfig &orig)
        {
            mapping = orig.mapping;
            header = orig.header;
        }
        ~ResourceConfig() { mapping.clear(); }
    }; //# struct ResourceConfig

} //> namespace resource

#endif //> FG_INC_RESOURCE_CONFIG