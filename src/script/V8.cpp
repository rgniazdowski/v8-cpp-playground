#include <script/V8.hpp>
#include <util/Bindings.hpp>
#include <resource/GlobalObjectRegistry.hpp>

#include <unordered_map>

script::LocalValue *script::argsToPointer(v8::Isolate *isolate, util::WrappedArgs const &args, util::WrappedArgs &registeredArgs)
{
    const int argc = static_cast<int>(args.size());
    auto argv = new LocalValue[argc];
    for (int idx = 0; idx < argc; idx++)
    {
        auto pWrapped = args[idx];
        const auto &wrapped = *pWrapped;
        if (wrapped.isEmpty())
        {
            argv[idx] = v8::Undefined(isolate);
            continue;
        }
        // TODO
        // Need to use v8pp::to_v8<> in order to easily convert integral types and strings
        // However any EXTERNAL object will have to use:
        // * GlobalObjectRegistry if only identifier is specified and void pointer is not available
        //      > In case even if object is dereferenced from unique identifier, it still has to be
        //      > automatically casted to correct type in order to search for v8pp::class_<TYPE> registry
        //      > so it's possible to return it as an V8 object (pass it as argument).
        //      > Furthermore - v8pp::class_<TYPE> works only if reference_external() was used.
        // * Pointer is already available but need to cast it and wrap it using reference_external<>
        //      > The question is should it be only temporary? Just to pass the input argument
        //      > to the function and call unreference_external afterwards. How to know if object is temporary?
        //
        //      ? Object is temporary if it's not already registered? In V8PP or GlobalRegistry?
        //
        //      ? How to unpack wrapped args and know the real type? Including casting?
        //
        auto *converter = getWrappedValueConverter(wrapped.getPrimitiveTypeId()); // this will also call converter for EXTERNAL type
        if (!converter)
        {
            argv[idx] = v8::Undefined(isolate);
            continue;
        }
        auto registered = converter->isRegistered(isolate, wrapped);
        if (!registered && wrapped.isExternal())
        {
            converter->registerWithV8(isolate, wrapped);
            // add to the list for easy unregistration - wrapped value now works as a temporary
            registeredArgs.push_back(pWrapped);
        }
        auto value = converter->convert(isolate, wrapped);
    } //# for each original argument

    return argv;
}
//>---------------------------------------------------------------------------------------

void script::unregisterArgs(v8::Isolate *isolate, const util::WrappedArgs &registeredArgs)
{
    for (auto &it : registeredArgs)
    {
        const auto &wrapped = *it;
        if (wrapped.isEmpty() || !wrapped.isExternal())
            continue;
        auto *converter = getWrappedValueConverter(wrapped.getPrimitiveTypeId()); // this will also call converter for EXTERNAL type
        if (!converter)
            continue;
        auto registered = converter->isRegistered(isolate, wrapped);
        if (!registered)
            converter->unregister(isolate, wrapped);
    } //# for each original argument
}
//>---------------------------------------------------------------------------------------

static std::unordered_map<uint32_t, script::WrappedValueConverter *> s_wrappedValueConverters;

script::WrappedValueConverter *script::getWrappedValueConverter(uint32_t tid)
{
    auto it = s_wrappedValueConverters.find(tid);
    if (it == s_wrappedValueConverters.end())
        return nullptr;
    return it->second;
}
//>---------------------------------------------------------------------------------------

void script::registerWrappedValueConverter(uint32_t tid, WrappedValueConverter *converter)
{
    if (getWrappedValueConverter(tid) != nullptr)
        return; // already have it!
    s_wrappedValueConverters.emplace(tid, converter);
}
//>---------------------------------------------------------------------------------------

void script::unregisterWrappedValueConverters(void)
{
    for (auto &it : s_wrappedValueConverters)
    {
        delete it.second;
        it.second = nullptr;
    }
    s_wrappedValueConverters.clear();
}
//>---------------------------------------------------------------------------------------

struct XD : public util::ObjectWithIdentifier
{
    uint64_t getIdentifier() const override { return 0; }
};

void xdxd()
{
    script::registerWrappedValueConverter<v8::Number, util::WrappedValue::DOUBLE>();
    script::registerWrappedValueConverter<XD>();

    // script::getWrappedValueConverter()
}
