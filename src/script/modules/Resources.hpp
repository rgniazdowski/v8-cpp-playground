#pragma once
#ifndef FG_INC_SCRIPT_MODULES_RESOURCES
#define FG_INC_SCRIPT_MODULES_RESOURCES

#include <script/modules/Timers.hpp>
#include <resource/Resource.hpp>
//#include <resource/ResourceGroup.hpp>
#include <resource/ZipFileResource.hpp>
#include <util/Logger.hpp>
#include <util/Callbacks.hpp>

#include <v8pp/module.hpp>
#include <v8pp/convert.hpp>
#include <v8pp/class.hpp>

namespace v8pp::detail
{
    template <>
    struct is_string<util::NamedHandle> : std::true_type
    {
    };
} //> namespace v8pp::detail

namespace script::modules
{
    class Resources : public ::script::InternalModule
    {
        using base_type = ::script::InternalModule;
        friend class ::script::ScriptManager;

    public:
        Resources(v8::Isolate *isolate);
        virtual ~Resources();

    public:
        bool initialize(void) override;
        bool instantiateGlobals(LocalContext &context) override;
        bool registerModule(LocalObject &exports) override;

        static void requestResource(FunctionCallbackInfo const &args);
        static void getResource(FunctionCallbackInfo const &args);
        static void disposeResource(FunctionCallbackInfo const &args);

    protected:
        v8pp::module m_module;
        v8pp::module m_resourceTypes;

        v8pp::class_<resource::Resource> m_class_base;
        // v8pp::class_<resource::ResourceGroup> m_class_resourceGroup;
        v8pp::class_<resource::ZipFileResource> m_class_resourceZipFile;
    }; //# class Resources

} //> namespace script::modules

#endif //> FG_INC_SCRIPT_MODULES_EVENTS