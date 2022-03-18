#include <script/ScriptManager.hpp>
#include <script/ScriptCallback.hpp>

script::ScriptManager::ScriptManager(char **argv) : manager_type(), m_argv(argv),
                                                    m_createParams(), m_isolate(nullptr),
                                                    m_platform(), m_contexts()
{
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
    return true;
}
//>---------------------------------------------------------------------------------------

bool script::ScriptManager::destroy(void)
{
    if (!isInit())
        return false;
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
    util::WrappedArgs _args(args); // shallow copy
    // auto backcopy = _args.back();
    _args.pop_back(); // remove the last item (external to callback)
    // delete backcopy;  // this is a complete duplicate - release it
    //  arguments are copied over! (full duplication) in PendingCallback constructor
    m_pendingCallbacks.emplace(_args, callback);
    auto &__back = m_pendingCallbacks.top();
    auto __c = __back.callback;
    auto &__a = __back.args;
    return true;
}
//>---------------------------------------------------------------------------------------

void script::ScriptManager::processPendingCallbacks(void)
{
    v8::HandleScope handle_scope(m_isolate);
    auto context = m_isolate->GetEnteredContext();
    while (!m_pendingCallbacks.empty())
    {
        auto &top = m_pendingCallbacks.top();
        auto callback = top.callback;
        auto &args = top.args;
        if (callback->isFunction())
        {
            auto function = callback->m_nativeFunction.Get(m_isolate);
            v8::TryCatch tryCatch(m_isolate);
            // convert wrapped args into an array of local values to be used as inputs to JS function
            // it requires a map/array of converters because type information is lost when values is wrapped
            util::WrappedArgs registeredArgs; // any external pointers that have been registered with V8 are placed here - these are temporary
            std::unique_ptr<LocalValue> argv(argsToPointer(m_isolate, args, registeredArgs));
            // need to figure out if using 'this' as global is ok, or can it be undefined
            auto result = function->Call(context, context->Global(), static_cast<int>(args.size()), argv.get());
            unregisterArgs(m_isolate, registeredArgs); // release any temporary objects from V8 - any handles will be invalidated
            if (result.IsEmpty())
            {
                auto ex = tryCatch.Exception();
            }
        }
        else if (callback->isScriptEval())
        {
            auto source =
                v8::String::NewFromUtf8(m_isolate, callback->m_script.c_str(), v8::NewStringType::kNormal)
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
        m_pendingCallbacks.pop();
    }
}
//>---------------------------------------------------------------------------------------
