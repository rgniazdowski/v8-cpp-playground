#pragma once
#ifndef FG_SCRIPT_CALLBACK
#define FG_SCRIPT_CALLBACK

#include <util/Bindings.hpp>
#include <util/Callbacks.hpp>
//#include <util/Handle.hpp>
#include <script/V8.hpp>

namespace script
{
    inline const util::CallbackType CALLBACK_SCRIPT = 3;

    class ScriptManager;

    /**
     * @brief Script callback should point to either a piece of code to execute (evaluate),
     * but this will be tricky to store input arguments (or even support wrapped args for it),
     * or a JS function object that has to be executed in a different place.
     * This is an additional complication - when ScriptCallback is called it does not execute
     * the code at all - just adds it to a special queue that's processed by the Script Manager.
     * It's important to execute this kind of code in a dedicated thread with isolate and
     * context selected and entered.
     */
    class ScriptCallback : public util::MethodCallback<ScriptManager>
    {
    public:
        friend class ScriptManager;
        using UserClass = ScriptManager;
        using base_type = util::MethodCallback<ScriptManager>;
        using self_type = ScriptCallback;

        // This is the only compatible wrapper for handling script callbacks.
        // unpack_caller helper from utils already has a specialization to pass-through
        // wrapped arguments vector if the type matches, so it's possible to have universal
        // callback for many different input types (events, custom timer parameters etc.)
        using ScriptMethod = bool (UserClass::*)(const util::WrappedArgs &);

    protected:
        ScriptCallback() : base_type() { m_type = CALLBACK_SCRIPT; }
        ScriptCallback(UserClass *pObject) : base_type(pObject) { m_type = CALLBACK_SCRIPT; }
        ScriptCallback(util::BindInfo *pBinding) : base_type(pBinding) { m_type = CALLBACK_SCRIPT; }
        ScriptCallback(util::BindInfo *pBinding, UserClass *pObject) : base_type(pBinding, pObject) { m_type = CALLBACK_SCRIPT; }

    public:
        virtual ~ScriptCallback()
        {
            m_nativeFunction.Reset();
        }

        virtual bool operator()(void) const override
        {
            if (!m_binding || !m_pObject)
                return false;
            // It's ok for ScriptCallback to just use external() wrap even if it does not use
            // getIdentifier (returns always 0) and this object will never be registered
            // inside of GlobalObjectRegistry (it checks for base class 'ObjectWithIdentifier')
            // In this case the point is to temporarily add pointer to this callback.
            util::WrappedArgs _args = {util::WrappedValue::external(this, 0)};
            auto out = !!util::BindingHelper::callWrapped(m_pObject, m_binding.get(), _args);
            util::reset_arguments(_args);
            return out;
        }

        virtual bool operator()(const util::WrappedArgs &args) const
        {
            if (!m_binding || !m_pObject)
                return false;
            util::WrappedArgs _args(args);
            _args.emplace_back(util::WrappedValue::external(this, 0));
            auto out = !!util::BindingHelper::callWrapped(m_pObject, m_binding.get(), _args);
            auto back = _args.back();
            delete back;
            _args.clear(); // do not use reset_arguments here - args is a copy
            return out;
        }

        void setNativeFunction(v8::Isolate *isolate, const LocalFunction &function)
        {
            if (!isolate && !m_pObject)
                return;
            if (!m_nativeFunction.IsEmpty())
                m_nativeFunction.Reset();
            m_nativeFunction = PersistentFunction::Persistent(isolate, function);
            m_script.clear();
        }

        void setScriptSource(std::string_view source)
        {
            if (!m_nativeFunction.IsEmpty())
                m_nativeFunction.Reset();
            m_script.clear();
            m_script.reserve(source.length() + 1);
            m_script.append(source.data());
        }

        inline bool isScriptEval(void) const { return m_script.length() > 0 && m_nativeFunction.IsEmpty(); }

        inline bool isFunction(void) const { return m_script.empty() && !m_nativeFunction.IsEmpty(); }

        inline uint64_t getIdentifier(void) const { return 0; /* ignored */ }

    protected:
        static self_type *create(ScriptMethod methodMember, UserClass *pObject = nullptr)
        {
            // Binding is done automatically to a special method present on ScriptManager.
            // This method will automatically decide what to do with this callback and
            // how to process wrapped args that need to be passed on input.
            // In reality the input wrapped args are already customized to hold V8 specific
            // values and these will get passed into the native function upon calling.
            return new self_type(new util::BindInfoMethod<UserClass, bool, const util::WrappedArgs &>("callback", methodMember, {}), pObject);
        }

    protected:
        PersistentFunction m_nativeFunction;
        std::string m_script;
    }; //# class ScriptCallback

} //> namespace script

namespace util
{
    inline bool isScriptCallback(Callback *pCallback) { return pCallback != nullptr && pCallback->getType() == script::CALLBACK_SCRIPT; }
} //> namespace util

#endif //> FG_SCRIPT_CALLBACK
