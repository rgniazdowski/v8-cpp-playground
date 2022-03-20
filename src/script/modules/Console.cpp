#include <script/modules/Console.hpp>
#include <util/Logger.hpp>
#include <iostream>

bool script::modules::Console::initialize(void)
{
    auto isolate = m_module.isolate();
    m_module.function("log", &Console::log);
    m_module.function("info", &Console::info);
    m_module.function("warn", &Console::warn);
    m_module.function("error", &Console::error);
    m_module.function("debug", &Console::debug);
    m_module.function("trace", &Console::trace);
    // TODO
    /*
    0: "debug"
    1: "error"
    2: "info"
    3: "log"
    4: "warn"
    5: "dir"
    6: "dirxml"
    7: "table"
    8: "trace"
    9: "group"
    10: "groupCollapsed"
    11: "groupEnd"
    12: "clear"
    13: "count"
    14: "countReset"
    15: "assert"
    16: "profile"
    17: "profileEnd"
    18: "time"
    19: "timeLog"
    20: "timeEnd"
    21: "timeStamp"
    22: "context"
    23: "memory"
    */
    m_init = true;
    return true;
} //> initialize()
//>---------------------------------------------------------------------------------------

bool script::modules::Console::instantiateGlobals(LocalContext &context)
{
    auto isolate = context->GetIsolate();
    context->Global()->Set(context, v8pp::to_v8(isolate, "console"), m_module.new_instance());
    return true;
} //> instantiateGlobals(...)
//>---------------------------------------------------------------------------------------

bool script::modules::Console::registerModule(LocalObject &exports)
{
    /* NOTHING TO EXPORT */
    return false;
} //> registerModule(...)
//>---------------------------------------------------------------------------------------

void script::modules::Console::wrapper(logger::Level level, FunctionCallbackInfo const &args)
{
    std::string str;
    argsToString(args, str);
    logger::PrintLog(level, "%s", str.c_str());
} //> log(...)
//>---------------------------------------------------------------------------------------

void script::modules::Console::log(FunctionCallbackInfo const &args)
{
    v8::HandleScope handle_scope(args.GetIsolate());
    std::string str;
    argsToString(args, str);
    std::cout << str << std::endl; // without any prefixes or timestamps - just print
} //> info(...)
//>---------------------------------------------------------------------------------------
