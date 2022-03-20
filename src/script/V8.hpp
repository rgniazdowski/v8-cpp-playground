#pragma once
#ifndef FG_INC_SCRIPT_V8
#define FG_INC_SCRIPT_V8

#include <libplatform/libplatform.h>
#include <v8.h>

#include <util/Bindings.hpp>
#include <v8pp/class.hpp>
#include <resource/GlobalObjectRegistry.hpp>

namespace script
{
    using Utf8String = v8::String::Utf8Value;

    using GlobalObject = v8::Global<v8::Object>;
    using GlobalFunction = v8::Global<v8::Function>;
    using GlobalValue = v8::Global<v8::Value>;
    using GlobalProxy = v8::Global<v8::Proxy>;
    using GlobalContext = v8::Global<v8::Context>;

    using LocalObject = v8::Local<v8::Object>;
    using LocalFunction = v8::Local<v8::Function>;
    using LocalValue = v8::Local<v8::Value>;
    using LocalProxy = v8::Local<v8::Proxy>;
    using LocalString = v8::Local<v8::String>;
    using LocalScript = v8::Local<v8::Script>;
    using LocalContext = v8::Local<v8::Context>;

    using PersistentObject = v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>>;
    using PersistentFunction = v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>;
    using PersistentValue = v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>>;
    using PersistentProxy = v8::Persistent<v8::Proxy, v8::CopyablePersistentTraits<v8::Proxy>>;
    using PersistentContext = v8::Persistent<v8::Context>;

    using FunctionCallbackInfo = v8::FunctionCallbackInfo<v8::Value>;

    inline LocalValue *argsToPointer(FunctionCallbackInfo const &args)
    {
        const auto argc = args.Length();
        auto argv = new LocalValue[argc];
        for (int idx = 0; idx < argc; idx++)
        {
            argv[idx] = args[idx];
        } //# for each original argument
        return argv;
    }

    inline LocalValue *argsToPointer(v8::Isolate *isolate, std::vector<PersistentValue> const &args)
    {
        const int argc = static_cast<int>(args.size());
        auto argv = new LocalValue[argc];
        for (int idx = 0; idx < argc; idx++)
        {
            argv[idx] = args[idx].Get(isolate);
        } //# for each original argument
        return argv;
    }

    LocalValue *argsToPointer(v8::Isolate *isolate, util::WrappedArgs const &args, util::WrappedArgs &registeredArgs, int numArgs = -1);

    void unregisterArgs(v8::Isolate *isolate, const util::WrappedArgs &registeredArgs);

    void argsToString(FunctionCallbackInfo const &args, std::string &output);

    struct WrappedValueConverter
    {
        virtual LocalValue convert(v8::Isolate *isolate, const util::WrappedValue &wrapped) = 0;
        virtual bool isRegistered(v8::Isolate *isolate, const util::WrappedValue &wrapped) const { return false; }
        virtual bool registerWithV8(v8::Isolate *isolate, const util::WrappedValue &wrapped) const { return false; }
        virtual bool unregister(v8::Isolate *isolate, const util::WrappedValue &wrapped) const { return false; }
    };

    template <typename V8Type, util::WrappedValue::Type ValueType, typename UserType = void>
    struct WrappedValueConverterSpec : public WrappedValueConverter
    {
        static_assert(std::is_base_of_v<v8::Primitive, V8Type>,
                      "V8Type template parameter type needs to be derived from V8::Primitive");
        LocalValue convert(v8::Isolate *isolate, const util::WrappedValue &wrapped) override
        {
            if (!isolate || wrapped.isEmpty())
                return v8::Undefined(isolate);
            if (wrapped.getType() != ValueType || ValueType == util::WrappedValue::EXTERNAL)
                return v8::Undefined(isolate);
            const auto &value = wrapped.get<util::WrappedValue::primitive_type<ValueType>>();
            return v8pp::to_v8(isolate, value).As<v8::Value>();
            // return V8Type::New(isolate, wrapped.get<util::WrappedValue::primitive_type<ValueType>>()).As<v8::Value>();
        }
    };

    template <>
    struct WrappedValueConverterSpec<v8::String, util::WrappedValue::STRING, void> : public WrappedValueConverter
    {
        LocalValue convert(v8::Isolate *isolate, const util::WrappedValue &wrapped) override
        {
            if (!isolate || wrapped.isEmpty())
                return v8::Undefined(isolate);
            if (wrapped.getType() != util::WrappedValue::STRING)
                return v8::Undefined(isolate);
            return v8pp::to_v8(isolate, wrapped.get<std::string>()).As<v8::Value>();
        }
    };

    template <typename UserType>
    struct WrappedValueConverterSpec<v8::Object, util::WrappedValue::EXTERNAL, UserType> : public WrappedValueConverter
    {
        static_assert(std::is_void_v<UserType> == false,
                      "UserType cannot be void when specializing WrappedValueConverter with EXTERNAL value type");
        LocalValue convert(v8::Isolate *isolate, const util::WrappedValue &wrapped) override
        {
            if (!isolate || wrapped.isEmpty() || !wrapped.isExternal())
                return v8::Undefined(isolate);
            using user_type = std::decay_t<UserType>;
            using v8_class = v8pp::class_<user_type>;
            // this will call cast and convertToPointer which uses global object registry
            auto vptr = static_cast<const user_type *>(wrapped);
            if (!vptr)
                return v8::Undefined(isolate);
            LocalObject obj = v8_class::find_object(isolate, vptr);
            return obj;
        }
        bool isRegistered(v8::Isolate *isolate, const util::WrappedValue &wrapped) const override
        {
            if (!isolate || wrapped.isEmpty() || !wrapped.isExternal())
                return false;
            using user_type = std::decay_t<UserType>;
            using v8_class = v8pp::class_<user_type>;
            // this will call cast and convertToPointer which uses global object registry
            auto vptr = static_cast<const user_type *>(wrapped);
            if (!vptr)
                return false;
            LocalObject obj = v8_class::find_object(isolate, vptr);
            if (obj.IsEmpty())
                return false;
            return true;
        }
        bool registerWithV8(v8::Isolate *isolate, const util::WrappedValue &wrapped) const override
        {
            if (!isolate || wrapped.isEmpty() || !wrapped.isExternal())
                return false;
            using user_type = std::decay_t<UserType>;
            using v8_class = v8pp::class_<user_type>;
            // this will call cast and convertToPointer which uses global object registry
            auto vptr = static_cast<const user_type *>(wrapped);
            if (!vptr)
                return false;
            LocalObject obj = v8_class::find_object(isolate, vptr);
            if (!obj.IsEmpty())
                return false; // already registered
            v8_class::reference_external(isolate, const_cast<user_type *>(vptr));
            return true;
        }
        bool unregister(v8::Isolate *isolate, const util::WrappedValue &wrapped) const override
        {
            if (!isolate || wrapped.isEmpty() || !wrapped.isExternal())
                return false;
            using user_type = std::decay_t<UserType>;
            using v8_class = v8pp::class_<user_type>;
            // this will call cast and convertToPointer which uses global object registry
            auto vptr = const_cast<user_type *>(static_cast<const user_type *>(wrapped));
            if (!vptr)
                return false;
            LocalObject obj = v8_class::find_object(isolate, vptr);
            if (obj.IsEmpty())
                return false; // not registered - cannot unregister
            v8_class::unreference_external(isolate, vptr);
            return true;
        }
    };

    template <typename UserType>
    using WrappedValueConverterSpecExternal = WrappedValueConverterSpec<v8::Object, util::WrappedValue::EXTERNAL, UserType>;

    WrappedValueConverter *getWrappedValueConverter(uint32_t tid);

    inline WrappedValueConverter *getWrappedValueConverter(util::WrappedValue::Type valueType)
    {
        return getWrappedValueConverter(util::WrappedValue::getPrimitiveTypeId(valueType));
    }

    template <typename UserType>
    std::enable_if_t<std::is_void_v<UserType> == false, WrappedValueConverter> *getWrappedValueConverter(void)
    {
        using user_type = std::decay_t<UserType>;
        auto tid = util::UniversalId<user_type>::id(); // FIXME
        return getWrappedValueConverter(tid);
    }

    void registerWrappedValueConverter(uint32_t tid, WrappedValueConverter *converter);

    template <typename V8Type, util::WrappedValue::Type ValueType>
    inline void registerWrappedValueConverter(void)
    {
        auto converter = new WrappedValueConverterSpec<V8Type, ValueType, void>();
        auto tid = util::UniversalId<util::WrappedValue::primitive_type<ValueType>>::id(); // FIXME
        registerWrappedValueConverter(tid, converter);
    }

    template <typename UserType>
    inline std::enable_if_t<std::is_void_v<UserType> == false> registerWrappedValueConverter(void)
    {
        using user_type = std::decay_t<UserType>;
        WrappedValueConverterSpec<v8::Object, util::WrappedValue::EXTERNAL, UserType> *converter = new script::WrappedValueConverterSpecExternal<user_type>();
        auto tid = util::UniversalId<user_type>::id(); // FIXME
        registerWrappedValueConverter(tid, converter);
    }

    void unregisterWrappedValueConverters(void);

} //> namespace script

#endif //> FG_INC_SCRIPT_V8
