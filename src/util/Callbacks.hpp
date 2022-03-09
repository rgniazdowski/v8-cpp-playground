#pragma once
#ifndef FG_INC_CALLBACKS
#define FG_INC_CALLBACKS

#include <util/CallbackHelper.hpp>
#include <util/Bindings.hpp>

namespace util
{
    using CallbackType = unsigned int;

    inline const CallbackType CALLBACK_INVALID = 0;
    inline const CallbackType CALLBACK_FUNCTION = 1;
    inline const CallbackType CALLBACK_METHOD = 2;

    class Callback
    {
    protected:
        /// Instead of holding pointer or dedicated std::function, wrap it around one
        /// binding info object, that can hold labdas, global & member functions and
        /// already has interface prepared
        std::unique_ptr<BindInfo> m_binding;
        CallbackType m_type;

    protected:
        Callback(CallbackType type) : m_type(type), m_binding() {}
        Callback(CallbackType type, BindInfo *pBinding) : m_type(type), m_binding(pBinding) {}
        Callback(Callback &&other) : m_binding(std::move(other.m_binding)), m_type(other.m_type) { other.m_type = 0; }

    public:
        virtual ~Callback() { m_type = CALLBACK_INVALID; }

        virtual bool operator()(void) const = 0;
        virtual bool operator()(const WrappedValue::Args &args) const = 0;

        inline CallbackType getType(void) const { return m_type; }

        const BindInfo *getBinding(void) const { return m_binding.get(); }
    }; //# class Callback
    //#-----------------------------------------------------------------------------------

    class FunctionCallback : public virtual Callback
    {
    public:
        using base_type = Callback;
        using self_type = FunctionCallback;

    public:
        FunctionCallback() : Callback(CALLBACK_FUNCTION) {}
        FunctionCallback(BindInfo *pBinding) : Callback(CALLBACK_FUNCTION, pBinding) {}

        template <typename ReturnType, typename... Args>
        self_type &setup(ReturnType (*function)(Args...),
                         const std::initializer_list<std::string> &argNames = {})
        {
            if (m_binding)
            {
                auto binding = m_binding.release();
                delete binding;
            }
            m_binding.reset(new BindInfoFunction<ReturnType, Args...>("callback", function, argNames));
            return *this;
        }

        template <typename ReturnType, typename... Args>
        self_type &setup(std::function<ReturnType(Args...)> &function,
                         const std::initializer_list<std::string> &argNames = {})
        {
            if (m_binding)
            {
                auto binding = m_binding.release();
                delete binding;
            }
            m_binding.reset(new BindInfoFunction<ReturnType, Args...>("callback", std::move(function), argNames));
            return *this;
        }

        virtual bool operator()(void) const
        {
            if (!m_binding)
                return false;
            return !!BindingHelper::callWrapped((void *)nullptr, m_binding.get(), WrappedValue::Args());
        }

        virtual bool operator()(const WrappedValue::Args &args) const
        {
            if (!m_binding)
                return false;
            return !!BindingHelper::callWrapped((void *)nullptr, m_binding.get(), args);
        }

        template <typename ReturnType, typename... Args>
        static self_type *create(ReturnType (*function)(Args...),
                                 const std::initializer_list<std::string> &argNames = {})
        {
            return new self_type(new BindInfoFunction<ReturnType, Args...>("callback", function, argNames));
        }

        template <typename ReturnType, typename... Args>
        static self_type *create(std::function<ReturnType(Args...)> &function,
                                 const std::initializer_list<std::string> &argNames = {})
        {
            return new self_type(new BindInfoFunction<ReturnType, Args...>("callback", std::move(function), argNames));
        }
    }; //# class FunctionCallback
    //#-----------------------------------------------------------------------------------

    template <typename UserClass>
    class MethodCallback : public virtual Callback
    {
    public:
        using base_type = Callback;
        using self_type = MethodCallback;

    protected:
        UserClass *m_pObject;

    public:
        MethodCallback() : Callback(CALLBACK_METHOD), m_pObject(nullptr) {}
        MethodCallback(UserClass *pObject) : Callback(CALLBACK_METHOD), m_pObject(pObject) {}
        MethodCallback(BindInfo *pBinding) : Callback(CALLBACK_METHOD, pBinding), m_pObject(nullptr) {}
        MethodCallback(BindInfo *pBinding, UserClass *pObject) : Callback(CALLBACK_METHOD, pBinding), m_pObject(pObject) {}

        UserClass *getObject(void) { return m_pObject; }

        const UserClass *getObject(void) const { return m_pObject; }

        void setObject(UserClass *pObject) { m_pObject = pObject; }

        template <typename UserClass, typename ReturnType, typename... Args>
        self_type &method(ReturnType (UserClass::*methodMember)(Args...),
                          const std::initializer_list<std::string> &argNames = {})
        {
            if (m_binding)
            {
                auto binding = m_binding.release();
                delete binding;
            }
            m_binding.reset(new BindInfoMethod<UserClass, ReturnType, Args...>("callback", methodMember, argNames));
            return *this;
        }

        virtual bool operator()(void) const
        {
            if (!m_binding || !m_pObject)
                return false;
            return !!BindingHelper::callWrapped(m_pObject, m_binding.get(), WrappedValue::Args());
        }

        virtual bool operator()(const WrappedValue::Args &args) const
        {
            if (!m_binding || !m_pObject)
                return false;
            return !!BindingHelper::callWrapped(m_pObject, m_binding.get(), args);
        }

        template <typename ReturnType, typename... Args>
        static self_type *create(ReturnType (UserClass::*methodMember)(Args...),
                                 UserClass *pObject = nullptr,
                                 const std::initializer_list<std::string> &argNames = {})
        {
            return new self_type(new BindInfoMethod<UserClass, ReturnType, Args...>("callback", methodMember, argNames), pObject);
        }
    }; //# class MethodCallback
    //#-----------------------------------------------------------------------------------

    inline bool isMethodCallback(Callback *pCallback) { return pCallback != nullptr && pCallback->getType() == CALLBACK_METHOD; }

    inline bool isFunctionCallback(Callback *pCallback) { return pCallback != nullptr && pCallback->getType() == CALLBACK_FUNCTION; }

} //> namespace util

#endif //> FG_INC_CALLBACKS