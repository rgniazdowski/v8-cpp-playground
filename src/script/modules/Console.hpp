#pragma once
#ifndef FG_INC_SCRIPT_MODULES_CONSOLE
#define FG_INC_SCRIPT_MODULES_CONSOLE

#include <script/InternalModule.hpp>
#include <util/Logger.hpp>

#include <v8pp/module.hpp>

namespace script::modules
{
    class Console : public ::script::InternalModule
    {
        using base_type = ::script::InternalModule;

    public:
        Console(v8::Isolate *isolate) : base_type(isolate, "console"), m_module(isolate)
        {
            setMode(BuiltinGlobalsOnly);
        }
        virtual ~Console() {}

    public:
        bool initialize(void) override;
        bool instantiateGlobals(LocalContext &context) override;
        bool registerModule(LocalObject &exports) override;

    public:
        static void wrapper(logger::Level level, FunctionCallbackInfo const &args);
        static void log(FunctionCallbackInfo const &args);
        static void info(FunctionCallbackInfo const &args) { Console::wrapper(logger::Level::Info, args); }
        static void warn(FunctionCallbackInfo const &args) { Console::wrapper(logger::Level::Warning, args); }
        static void error(FunctionCallbackInfo const &args) { Console::wrapper(logger::Level::Error, args); }
        static void debug(FunctionCallbackInfo const &args) { Console::wrapper(logger::Level::Debug, args); }
        static void trace(FunctionCallbackInfo const &args) { Console::wrapper(logger::Level::Trace, args); }

    protected:
        v8pp::module m_module;
    }; //# class Console
} //> namespace script::modules

#endif //> FG_INC_SCRIPT_MODULES_CONSOLE