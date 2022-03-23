#pragma once
#ifndef FG_INC_SCRIPT_RESOURCE
#define FG_INC_SCRIPT_RESOURCE

#include <resource/Resource.hpp>
#include <util/UniversalId.hpp>
#include <util/File.hpp>

namespace script
{
    class ScriptResource : public resource::Resource
    {
        using self_type = ScriptResource;
        using base_type = resource::Resource;

    public:
        inline static const uint32_t SelfResourceId = util::UniversalId<self_type>::id();

    public:
        ScriptResource() : base_type()
        {
            this->m_resType = SelfResourceId;
        }

        ScriptResource(std::string_view path) : base_type(path)
        {
            this->m_resType = SelfResourceId;
        }

        virtual ~ScriptResource()
        {
            self_type::dispose();
            base_type::clear();
        }

    public:
        bool create(void) override
        {
            if (m_isReady)
                return true;
            // loading a script resource for now means just reading the file and holding text
            util::File file(getFilePath());
            auto content = file.load();
            if (!content)
                return false; // unable to create
            m_script.clear();
            m_script.append(content);
            delete content;
            m_isReady = true;
            return true;
        }

        bool recreate(void) override
        {
            dispose();
            m_isReady = false;
            return create();
        }

        void dispose(void) override
        {
            m_script.clear();
            m_isReady = false;
            return;
        }

        virtual bool isDisposed(void) const
        {
            return m_script.empty();
        }

        const std::string &getContent(void) const { return m_script; }

    protected:
        std::string m_script;
    }; //# class ScriptResource
} //> namespace script

#endif //> FG_INC_SCRIPT_RESOURCE
