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
        Callback(Callback &&other) noexcept : m_binding(std::move(other.m_binding)), m_type(other.m_type) { other.m_type = 0; }

    public:
        ~Callback() { m_type = CALLBACK_INVALID; }

        virtual bool operator()(void) const = 0;
        virtual bool operator()(const WrappedArgs &args) const = 0;

        inline CallbackType getType(void) const { return m_type; }

        const BindInfo *getBinding(void) const { return m_binding.get(); }
    }; //# class Callback
    //#-----------------------------------------------------------------------------------

    class FunctionCallback : public Callback
    {
    public:
        using base_type = Callback;
        using self_type = FunctionCallback;

    public:
        FunctionCallback() : Callback(CALLBACK_FUNCTION) {}
        FunctionCallback(BindInfo *pBinding) : Callback(CALLBACK_FUNCTION, pBinding) {}

        template <typename FunctionType>
        self_type &setup(FunctionType function,
                         const std::initializer_list<std::string> &argNames = {})
        {
            if (m_binding)
            {
                auto binding = m_binding.release();
                delete binding;
            }
            m_binding.reset(new BindInfoFunction<FunctionType>("callback", function, argNames));
            return *this;
        }

        template <typename FunctionType,
                  typename Traits = util::function_traits<FunctionType>,
                  typename StdFunction = typename Traits::std_function>
        self_type &setup(StdFunction &function,
                         const std::initializer_list<std::string> &argNames = {})
        {
            if (m_binding)
            {
                auto binding = m_binding.release();
                delete binding;
            }
            m_binding.reset(new BindInfoFunction<FunctionType>("callback", std::move(function), argNames));
            return *this;
        }

        virtual bool operator()(void) const
        {
            if (!m_binding)
                return false;
            return !!BindingHelper::callWrapped((void *)nullptr, m_binding.get(), WrappedArgs());
        }

        virtual bool operator()(const WrappedArgs &args) const
        {
            if (!m_binding)
                return false;
            return !!BindingHelper::callWrapped((void *)nullptr, m_binding.get(), args);
        }

        template <typename FunctionType>
        static self_type *create(FunctionType function,
                                 const std::initializer_list<std::string> &argNames = {})
        {
            return new self_type(new BindInfoFunction<FunctionType>("callback", function, argNames));
        }

        // template <typename FunctionType,
        //           typename Traits = util::function_traits<FunctionType>,
        //           typename StdFunction = typename Traits::std_function>
        // static self_type *create(StdFunction &function,
        //                          const std::initializer_list<std::string> &argNames = {})
        //{
        //     return new self_type(new BindInfoFunction<FunctionType>("callback", std::move(function), argNames));
        // }
    }; //# class FunctionCallback
    //#-----------------------------------------------------------------------------------

    template <typename UserClass>
    class MethodCallback : public Callback
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

        template <typename MethodType>
        self_type &method(MethodType methodMember,
                          const std::initializer_list<std::string> &argNames = {})
        {
            if (m_binding)
            {
                auto binding = m_binding.release();
                delete binding;
            }
            m_binding.reset(new BindInfoMethod<MethodType>("callback", methodMember, argNames));
            return *this;
        }

        virtual bool operator()(void) const
        {
            if (!m_binding || !m_pObject)
                return false;
            return !!BindingHelper::callWrapped(m_pObject, m_binding.get(), WrappedArgs());
        }

        virtual bool operator()(const WrappedArgs &args) const
        {
            if (!m_binding || !m_pObject)
                return false;
            return !!BindingHelper::callWrapped(m_pObject, m_binding.get(), args);
        }

        template <typename MethodType,
                  typename Traits = util::function_traits<MethodType>,
                  typename UserClass = typename Traits::class_type>
        static self_type *create(MethodType methodMember,
                                 UserClass *pObject = nullptr,
                                 const std::initializer_list<std::string> &argNames = {})
        {
            return new self_type(new BindInfoMethod<MethodType>("callback", methodMember, argNames), pObject);
        }
    }; //# class MethodCallback
    //#-----------------------------------------------------------------------------------

    inline bool isMethodCallback(Callback *pCallback) { return pCallback != nullptr && pCallback->getType() == CALLBACK_METHOD; }

    inline bool isFunctionCallback(Callback *pCallback) { return pCallback != nullptr && pCallback->getType() == CALLBACK_FUNCTION; }

} //> namespace util

#endif //> FG_INC_CALLBACKS