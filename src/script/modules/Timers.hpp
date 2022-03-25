#pragma once
#ifndef FG_INC_SCRIPT_MODULES_TIMERS
#define FG_INC_SCRIPT_MODULES_TIMERS

#include <script/InternalModule.hpp>
#include <util/Logger.hpp>

#include <v8pp/module.hpp>

namespace script
{
    class ScriptManager;
} //> namespace script

namespace script::modules
{
    class Timers : public ::script::InternalModule
    {
        using base_type = ::script::InternalModule;
        friend class ::script::ScriptManager;

    public:
        Timers(v8::Isolate *isolate) : base_type(isolate, "timers-internal")
        {
            setMode(BuiltinGlobalsOnly);
        }
        virtual ~Timers() {}

    public:
        bool initialize(void) override;
        bool instantiateGlobals(LocalContext &context) override;
        bool registerModule(LocalObject &exports) override;

    public:
        static void setTimerWrapper(int repeats, FunctionCallbackInfo const &args);

        inline static void setInterval(FunctionCallbackInfo const &args) { setTimerWrapper(-1, args); }
        inline static void setTimeout(FunctionCallbackInfo const &args) { setTimerWrapper(1, args); }

        static void clearTimeout(unsigned int handle);
    protected:
        inline static ScriptManager *s_pScriptMgr = nullptr;
    }; //# class Timers
} //> namespace script::modules

#endif //> FG_INC_SCRIPT_MODULES_TIMERS