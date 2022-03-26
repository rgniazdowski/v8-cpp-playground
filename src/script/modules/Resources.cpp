#include <script/modules/Resources.hpp>
#include <util/Logger.hpp>
#include <event/EventManager.hpp>
#include <resource/ResourceManager.hpp>

#include <magic_enum.hpp>
#include <iostream>

script::modules::Resources::Resources(v8::Isolate *isolate) : base_type(isolate, "resources-internal"),
                                                              m_module(isolate), m_resourceTypes(isolate),
                                                              m_class_base(isolate),
                                                              m_class_resourceZipFile(isolate)
{
    setMode(BuiltinGlobalsOnly);
}
//>---------------------------------------------------------------------------------------

script::modules::Resources::~Resources()
{
    auto isolate = m_class_base.isolate();
    removeClassObjects<resource::ZipFileResource>(isolate);
}
//>---------------------------------------------------------------------------------------

bool script::modules::Resources::initialize(void)
{
    auto isolate = m_module.isolate();
    setClassName(isolate, m_class_base, "Resource");
    setClassName(isolate, m_class_resourceZipFile, "ZipFile");
    auto managerRegistry = base::ManagerRegistry::instance();
    auto resourceMgr = managerRegistry->get<resource::ResourceManager>();
    auto factory = resourceMgr->getResourceFactory();
    auto &nameMap = factory->getNameMap();
    for (auto &it : nameMap)
        m_resourceTypes.const_(it.first, it.second);

    m_module.function("request", &Resources::requestResource);
    m_module.function("get", &Resources::getResource);
    m_module.function("dispose", &Resources::disposeResource);

    using GetFilePathFunc = std::string const &(resource::Resource::*)(void) const;
    using GetNameFunc = util::NamedHandle const &(resource::Resource::*)(void) const;

    m_class_base.function("getIdentifier", &resource::Resource::getIdentifier)
        .function("getFilePath", static_cast<GetFilePathFunc>(&resource::Resource::getFilePath))
        .function("getCurrentFilePath", &resource::Resource::getCurrentFilePath)
        .function("getLastAccess", &resource::Resource::getLastAccess)
        .function("getName", static_cast<GetNameFunc>(&resource::Resource::getName))
        .function("getResourceType", &resource::Resource::getResourceType)
        .function("getSize", &resource::Resource::getSize);

    m_class_resourceZipFile.inherit<resource::Resource>();

    registerExternalConverters<resource::ZipFileResource>();
    m_init = true;
    return true;
} //> initialize()
//>---------------------------------------------------------------------------------------

bool script::modules::Resources::instantiateGlobals(LocalContext &context)
{
    auto isolate = context->GetIsolate();
    auto global = context->Global();
    global->Set(context, v8pp::to_v8(isolate, "ResourceManager"), m_module.new_instance());
    global->Set(context, v8pp::to_v8(isolate, "ResourceType"), m_resourceTypes.new_instance());
    return true;
} //> instantiateGlobals(...)
//>---------------------------------------------------------------------------------------

bool script::modules::Resources::registerModule(LocalObject &exports)
{
    /* NOTHING TO EXPORT */
    return false;
} //> registerModule(...)
//>---------------------------------------------------------------------------------------

bool getNameOrIdentifier(v8::Isolate *isolate, script::LocalValue arg, std::string &outName, uint64_t &outIdentifier)
{
    if (arg->IsString())
    {
        outName = v8pp::from_v8<std::string>(isolate, arg);
        return true;
    }
    else if (arg->IsNumber() || arg->IsBigInt())
    {
        outIdentifier = v8pp::from_v8<uint64_t>(isolate, arg);
        return true;
    }
    return false;
}

void script::modules::Resources::requestResource(FunctionCallbackInfo const &args)
{
    if (args.Length() < 1)
    {
        args.GetReturnValue().SetNull();
        return;
    }
    if (!args[0]->IsString())
    {
        args.GetReturnValue().SetNull();
        return;
    }
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto managerRegistry = base::ManagerRegistry::instance();
    auto resourceMgr = managerRegistry->get<resource::ResourceManager>();
    auto factory = resourceMgr->getResourceFactory();
    std::string info = v8pp::from_v8<std::string>(isolate, args[0]);
    resource::ResourceType forcedType = resource::AUTO;
    if (args.Length() > 1)
    {
        auto second = args[1];
        if (second->IsNumber() || second->IsInt32() || second->IsUint32())
        {
            auto castMaybeValue = second->Uint32Value(context);
            if (!castMaybeValue.IsNothing())
            {
                auto castValue = castMaybeValue.ToChecked();
                if (factory->isRegistered(castValue))
                    forcedType = castValue;
            }
        }
        else if (second->IsString())
        {
            std::string resourceTypeName = v8pp::from_v8<std::string>(isolate, second);
            auto keyFromName = factory->getKeyTypeForName(resourceTypeName);
            forcedType = keyFromName;
        }
    }
    auto resource = resourceMgr->request(info, forcedType);
    if (resource)
    {
        auto resourceType = resource->getResourceType();
        auto converter = getWrappedValueConverter(resourceType);
        if (converter)
        {
            auto resourceObject = converter->registerWithV8(isolate, util::WrappedValue::wrapInPlace(resource, resourceType));
            args.GetReturnValue().Set(resourceObject);
            return;
        }
    }
    args.GetReturnValue().SetNull();
} //> requestResource(...)
//>---------------------------------------------------------------------------------------

void script::modules::Resources::getResource(FunctionCallbackInfo const &args)
{
    if (args.Length() < 1)
    {
        args.GetReturnValue().SetNull();
        return;
    }
    if (!args[0]->IsString() && !args[0]->IsNumber())
    {
        args.GetReturnValue().SetNull();
        return;
    }
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto managerRegistry = base::ManagerRegistry::instance();
    auto resourceMgr = managerRegistry->get<resource::ResourceManager>();
    resource::Resource *resource;
    auto resourceTag = args[0];
    std::string name;
    uint64_t identifier = 0;
    if (getNameOrIdentifier(isolate, resourceTag, name, identifier))
    {
        if (!name.empty())
            resource = resourceMgr->get(name);
        else if (identifier)
            resource = resourceMgr->get(identifier);
    }
    if (resource)
    {
        auto resourceType = resource->getResourceType();
        auto converter = getWrappedValueConverter(resourceType);
        if (converter)
        {
            auto resourceObject = converter->registerWithV8(isolate, util::WrappedValue::wrapInPlace(resource, resourceType));
            args.GetReturnValue().Set(resourceObject);
            return;
        }
    }
    args.GetReturnValue().SetNull();
} //> getResource(...)
//>---------------------------------------------------------------------------------------

void script::modules::Resources::disposeResource(FunctionCallbackInfo const &args)
{
    if (args.Length() < 1)
    {
        args.GetReturnValue().Set(false);
        return;
    }
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto managerRegistry = base::ManagerRegistry::instance();
    auto resourceMgr = managerRegistry->get<resource::ResourceManager>();
    bool status = false;
    auto resourceTag = args[0];
    std::string name;
    uint64_t identifier = 0;
    if (getNameOrIdentifier(isolate, resourceTag, name, identifier))
    {
        if (!name.empty())
            status = resourceMgr->dispose(name);
        else if (identifier)
            status = resourceMgr->dispose(identifier);
    }
    args.GetReturnValue().Set(status);
} //> disposeResource(...)
//>---------------------------------------------------------------------------------------
