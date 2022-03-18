#include <script/ScriptManager.hpp>
#include <script/ScriptCallback.hpp>

script::ScriptManager::ScriptManager(char **argv) : manager_type(), m_argv(argv),
                                                    m_createParams(), m_isolate(nullptr),
                                                    m_platform(), m_contexts(), m_mutex()
{
    m_thread.setFunction([this]()
                         {
        v8::Locker locker(m_isolate);
        v8::Isolate::Scope isolate_scope(m_isolate);
        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(m_isolate);
        auto context = this->getContext("main");
        v8::Context::Scope context_scope(context);
        m_isolate->RunMicrotasks(); // run microtasks from previous frame
        this->processPendingCallbacks();
        return true; });
    // thread will wakeup by itself every 1ms (1000fps) to run V8 microtasks and have
    // callbacks triggered (from event thread)
    m_thread.setInterval(1);
    m_thread.setWakeable(true);
}
//>---------------------------------------------------------------------------------------

script::ScriptManager::~ScriptManager()
{
    self_type::destroy();
    script::unregisterWrappedValueConverters(); // no longer needed - might as well unregister all of them
}
//>---------------------------------------------------------------------------------------

bool script::ScriptManager::initialize(void)
{
    if (isInit())
        return true;
    v8::V8::InitializeICUDefaultLocation(m_argv[0]);
    v8::V8::InitializeExternalStartupData(m_argv[0]);
    m_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(m_platform.get());
    v8::V8::Initialize();
    // Create a new Isolate and make it the current one.
    m_createParams.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    m_isolate = v8::Isolate::New(m_createParams);
    m_isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    {
        v8::Isolate::Scope isolate_scope(m_isolate);
        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(m_isolate);
        // Create a new context.
        LocalContext context = v8::Context::New(m_isolate);
        m_contexts.emplace("main", WrappedContext(m_isolate, context));
        // main context is registered - now can create and manage all global objects before
        // entering (in another thread) a special event loop for microtasks and callbacks
    }
    m_init = true;
    m_thread.start();
    return true;
}
//>---------------------------------------------------------------------------------------

bool script::ScriptManager::destroy(void)
{
    if (!isInit())
        return false;
    signalThread();
    stopThread();
    m_contexts.clear(); // reset the context map completely
    // Dispose the isolate and tear down V8.
    m_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete m_createParams.array_buffer_allocator;
    m_isolate = nullptr;
    m_init = false;
    return true;
}
//>---------------------------------------------------------------------------------------

script::LocalContext script::ScriptManager::getContext(const std::string &name)
{
    auto it = m_contexts.find(name);
    if (it == m_contexts.end())
        return LocalContext();
    return it->second.local(m_isolate);
}
//>---------------------------------------------------------------------------------------

bool script::ScriptManager::scriptCallbackHandler(const util::WrappedArgs &args)
{
    if (args.empty())
        return false;        // cannot do anything!
    auto back = args.back(); // this is supposed to be ScriptCallback object external
    if (!back->isExternal())
        return false; // need to skip it
    if (!back->checkType<ScriptCallback>(true))
        return false; // external object is not valid
    ScriptCallback *callback = static_cast<ScriptCallback *>(back->getExternalPointer<void>());
    // Now need to duplicate the input arguments without the last script callback external.
    // After that, it'll be possible to queue this call so it's executed in script thread.
    // Script callback handler for sure is executed from one of the dedicated event threads.
    //
    // Arguments are copied over! (full duplication) in PendingCallback constructor
    // This includes also the extra ScriptCallback external, which will be skipped because
    // of the extra parameter that shows a lower number of input arguments for the event.
    {
        const std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_pendingCallbacks.emplace(args, callback, (int)(args.size() - 1));
    }
    m_thread.wakeup(); // signal the thread because there are new callbacks pending
    return true;
}
//>---------------------------------------------------------------------------------------

void script::ScriptManager::processPendingCallbacks(void)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    v8::HandleScope handle_scope(m_isolate);
    auto context = m_isolate->GetEnteredContext();
    while (true)
    {
        lock.lock();
        if (m_pendingCallbacks.empty())
            break;
        // move to new structure, will call destructor on next loop, releasing copied args vector
        PendingCallback top(std::move(m_pendingCallbacks.top()));
        m_pendingCallbacks.pop();
        // unlock so callback execution can actually add more pending callbacks on next loop
        lock.unlock();
        if (top.callback->isFunction())
        {
            auto function = top.callback->m_nativeFunction.Get(m_isolate);
            v8::TryCatch tryCatch(m_isolate);
            // convert wrapped args into an array of local values to be used as inputs to JS function
            // it requires a map/array of converters because type information is lost when values is wrapped
            util::WrappedArgs registeredArgs; // any external pointers that have been registered with V8 are placed here - these are temporary
            std::unique_ptr<LocalValue> argv(argsToPointer(m_isolate, top.args, registeredArgs, top.numArgs));
            // need to figure out if using 'this' as global is ok, or can it be undefined
            auto result = function->Call(context, context->Global(), static_cast<int>(top.args.size()), argv.get());
            unregisterArgs(m_isolate, registeredArgs); // release any temporary objects from V8 - any handles will be invalidated
            if (result.IsEmpty())
            {
                auto ex = tryCatch.Exception();
            }
        }
        else if (top.callback->isScriptEval())
        {
            auto source =
                v8::String::NewFromUtf8(m_isolate, top.callback->m_script.c_str(), v8::NewStringType::kNormal)
                    .ToLocalChecked();
            v8::TryCatch tryCatch(m_isolate);
            auto maybeScript = v8::Script::Compile(context, source);
            if (maybeScript.IsEmpty())
                continue;
            auto script = maybeScript.ToLocalChecked();
            auto result = script->Run(context);
            if (result.IsEmpty())
            {
                auto ex = tryCatch.Exception();
            }
        }
    }
}
//>---------------------------------------------------------------------------------------
