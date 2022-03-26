#pragma once
#ifndef FG_INC_RESOURCE_ZIP_FILE_RESOURCE
#define FG_INC_RESOURCE_ZIP_FILE_RESOURCE

#include <resource/Resource.hpp>
#include <util/ZipFile.hpp>
#include <util/File.hpp>

namespace resource
{
    class ZipFileResource : public resource::Resource
    {
    public:
        using base_type = resource::Resource;

        ZipFileResource() : base_type()
        {
            m_resType = util::UniversalId<ZipFileResource>::id();
        }

        ZipFileResource(std::string_view path) : base_type(path)
        {
            m_resType = util::UniversalId<ZipFileResource>::id();
        }

    public:
        virtual bool create(void) override { return true; };
        virtual bool recreate(void) override { return true; };
        virtual void dispose(void) override{};
        virtual bool isDisposed(void) const override { return false; };

    protected:
        util::File m_file;
    };
} //> namespace resource

#endif //> FG_INC_RESOURCE_ZIP_FILE_RESOURCE
