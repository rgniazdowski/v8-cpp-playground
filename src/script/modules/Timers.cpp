#include <script/modules/Timers.hpp>
#include <util/Logger.hpp>
#include <event/EventManager.hpp>
#include <script/ScriptCallback.hpp>
#include <script/ScriptManager.hpp>
#include <iostream>

bool script::modules::Timers::initialize(void)
{
    m_init = true;
    return true;
} //> initialize()
//>---------------------------------------------------------------------------------------

bool script::modules::Timers::instantiateGlobals(LocalContext &context)
{
    auto isolate = context->GetIsolate();
    auto global = context->Global();
    global->Set(context, v8pp::to_v8(isolate, "setInterval"), v8pp::wrap_function(isolate, "setInterval", &Timers::setInterval));
    global->Set(context, v8pp::to_v8(isolate, "setTimeout"), v8pp::wrap_function(isolate, "setTimeout", &Timers::setTimeout));
    global->Set(context, v8pp::to_v8(isolate, "clearInterval"), v8pp::wrap_function(isolate, "clearInterval", &Timers::clearTimeout));
    global->Set(context, v8pp::to_v8(isolate, "clearTimeout"), v8pp::wrap_function(isolate, "clearTimeout", &Timers::clearTimeout));
    return true;
} //> instantiateGlobals(...)
//>---------------------------------------------------------------------------------------

bool script::modules::Timers::registerModule(LocalObject &exports)
{
    /* NOTHING TO EXPORT */
    return false;
} //> registerModule(...)
//>---------------------------------------------------------------------------------------

void script::modules::Timers::setTimerWrapper(int repeats, FunctionCallbackInfo const &args)
{
    if (args.Length() < 1 || !s_pScriptMgr)
    {
        args.GetReturnValue().Set(-1); //#FIXME
        return;
    }
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto context = isolate->GetCurrentContext();
    if (repeats < -1)
        repeats = -1;
    auto managerRegistry = base::ManagerRegistry::instance();
    auto eventMgr = managerRegistry->get<event::EventManager>();
    auto &handler = args[0];
    if (!handler->IsFunction() && !handler->IsString())
    {
        args.GetReturnValue().Set(-1); //#FIXME
        return;
    }
    int64_t timeout = 0;
    std::vector<LocalValue> localHandlerArgs;
    if (args.Length() > 1 && (args[1]->IsNumber() || args[1]->IsInt32()))
    {
        if (!args[1]->IntegerValue(context).To(&timeout))
            timeout = 0; // reset
    }
    if (args.Length() > 2 && handler->IsFunction())
    {
        int localHandlerArgc = args.Length() >= 2 ? args.Length() - 2 : 0;
        if (localHandlerArgc > 0)
            localHandlerArgs.reserve(localHandlerArgc);
        // need to process arguments
        for (int idx = 2; idx < args.Length(); idx++)
        {
            localHandlerArgs.push_back(args[idx]);
        }
    }
    ScriptCallback *callback;
    if (handler->IsFunction())
    {
        auto function = handler.As<v8::Function>();
        callback = s_pScriptMgr->createScriptTimerCallback(function, localHandlerArgs);
    }
    else if (handler->IsString())
    {
        auto expression = handler.As<v8::String>();
        callback = s_pScriptMgr->createScriptCallback(expression);
    }
    auto id = eventMgr->addInterval((int)timeout, callback, repeats);
    args.GetReturnValue().Set(id);
    localHandlerArgs.clear();
} //> setTimerWrapper(...)
//>---------------------------------------------------------------------------------------

void script::modules::Timers::clearTimeout(unsigned int handle)
{
    auto managerRegistry = base::ManagerRegistry::instance();
    auto eventMgr = managerRegistry->get<event::EventManager>();
    eventMgr->removeTimer(handle);
} //> clearTimeout(...)
//>---------------------------------------------------------------------------------------
