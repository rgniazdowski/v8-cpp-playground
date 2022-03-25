#include <script/ScriptManager.hpp>
#include <script/ScriptCallback.hpp>
#include <script/ScriptResource.hpp>
#include <script/modules/Console.hpp>
#include <script/modules/Timers.hpp>
#include <script/modules/Events.hpp>
#include <v8pp/convert.hpp>

#include <event/EventManager.hpp>
#include <resource/ResourceManager.hpp>

script::ScriptManager::ScriptManager(char **argv) : manager_type(), m_argv(argv),
                                                    m_createParams(), m_isolate(nullptr),
                                                    m_platform(), m_contexts(), m_mutex()
{
    m_thread.setThreadName("ScriptManager");
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
    // thread will wakeup by itself every 1ms (1000fps) to run V8 microtasks and have (for now)
    // callbacks triggered (from event thread)
    // TODO actually should be different in order to not execute too many actions mid-frame
    // Ideally should be twice as screen refresh rate, so it should be configurable
    // Later on it also could be woken up on every refresh frame shot
    m_thread.setInterval(1); // 1000Hz - FIXME
    m_thread.setWakeable(true);
    // #FIXME
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::CHAR>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::SIGNED_CHAR>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::UNSIGNED_CHAR>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::SHORT>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::SIGNED_SHORT>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::UNSIGNED_SHORT>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::INT>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::UNSIGNED_INT>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::LONG>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::UNSIGNED_LONG>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::LONG_LONG>();
    script::registerWrappedValueConverter<v8::Integer, util::WrappedValue::UNSIGNED_LONG_LONG>();
    script::registerWrappedValueConverter<v8::Number, util::WrappedValue::FLOAT>();
    script::registerWrappedValueConverter<v8::Number, util::WrappedValue::DOUBLE>();
    script::registerWrappedValueConverter<v8::Boolean, util::WrappedValue::BOOL>();
    script::registerWrappedValueConverter<v8::String, util::WrappedValue::STRING>();
    script::registerWrappedValueConverter<v8::String, util::WrappedValue::STRING>();
} //> ScriptManager()
//>#--------------------------------------------------------------------------------------

script::ScriptManager::~ScriptManager()
{
    self_type::destroy();
    script::unregisterWrappedValueConverters(); // no longer needed - might as well unregister all of them
}
//>#--------------------------------------------------------------------------------------

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
        {
            // Need to create and initialize list of globals and built-in modules.
            auto console = new modules::Console(m_isolate);
            console->initialize();
            registerModule(console);
        }
        {
            modules::Timers::s_pScriptMgr = this;
            // Timers wrapped - create callbacks and timers on EventManager
            auto timers = new modules::Timers(m_isolate);
            timers->initialize();
            registerModule(timers);
        }
        {
            // Events - this is EventManager global along with additional info
            auto events = new modules::Events(m_isolate);
            events->initialize();
            registerModule(events);
        }
        // Create a new context.
        // Main context is registered - now can create and manage all global objects before
        // entering (in another thread) a special event loop for microtasks and callbacks.
        // Globals are also initialized within.
        LocalContext context = createContext("main");
    }
    // Try to hook up with Resource Manager and request a resource from it, in this case
    // it's important to try and find the 'main' module / script - an entry point for the
    // whole application - this module would trigger any other required code (via import).
    auto managerRegistry = base::ManagerRegistry::instance();
    auto resourceMgr = managerRegistry->get<resource::ResourceManager>();
    auto eventMgr = managerRegistry->get<event::EventManager>();
    if (resourceMgr != nullptr)
    {
        auto factory = resourceMgr->getResourceFactory();
        factory->registerObjectType<ScriptResource>("js;mjs");
        //! FIXME - this should happen on the thread and possibly the script resource
        //! should contain a valid 'Module' object (depending on type) that would get
        //! correctly compiled and executed on correct thread.
        //* The create() action on ScriptResource should just load the file content,
        //* compilation and execution has to be done explictly (and only once).
        //* Additionally, each ScriptResource should get auto-locked, so dispose() is not
        //* called at all on it.
        auto mainScript = resourceMgr->request("main.js", ScriptResource::SelfResourceId);
        if (!mainScript)
            mainScript = resourceMgr->request("main.mjs", ScriptResource::SelfResourceId);
        resourceMgr->rename(mainScript, "main-module");
        util::NamedHandle xd("main-module");
        auto x1 = resourceMgr->get("main-module");
        auto x2 = resourceMgr->get(xd);
        //? Could possibly add a script callback to process the code/modules on ProgramInit
        //? event - then the ScriptCallback custom handler (executed on event thread) would
        //? push actions to be triggered later on the desired target thread.
        // eventMgr->addCallback(event::Type::ProgramInit);
        //! FIXME
        auto callback = ScriptCallback::create(&ScriptManager::scriptCallbackHandler, this);
        callback->m_script = static_cast<ScriptResource *>(mainScript)->getContent();
        (*callback)();
        m_thread.runJustOnce();
    }
    m_init.store(true);
    return true;
} //> initialize()
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::destroy(void)
{
    if (!isInit())
        return false;
    signalThread();
    stopThread();
    releaseModules();
    m_contexts.clear(); // reset the context map completely
    // Dispose the isolate and tear down V8.
    m_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete m_createParams.array_buffer_allocator;
    m_isolate = nullptr;
    m_init.store(false);
    return true;
} //> destroy()
//>#--------------------------------------------------------------------------------------

script::LocalContext script::ScriptManager::getContext(const std::string &name)
{
    auto it = m_contexts.find(name);
    if (it == m_contexts.end())
        return LocalContext();
    return it->second.local(m_isolate);
} //> getContext(...)
//>#--------------------------------------------------------------------------------------

script::LocalContext script::ScriptManager::createContext(const std::string &name)
{
    if (name.empty() || !getContext(name).IsEmpty())
        return LocalContext(); // empty
    LocalContext context = v8::Context::New(m_isolate);
    v8::Context::Scope context_scope(context);
    auto it = m_contexts.emplace(name, WrappedContext(m_isolate, context)).first;
    instantiateGlobals(context, it->second);
    return context;
} //> createContext(...)
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::instantiateGlobals(LocalContext &context, WrappedContext &wrappedContext)
{
    if (context.IsEmpty())
        return false;
    // Process the current list of present modules and update contexts
    for (auto &it : m_modules)
    {
        auto module = it.second;
        // process current module only if it's not loadable, is initialized and is not present on context
        if (!module->isLoadable() && module->isInitialized() && !wrappedContext.hasModule(module->getName()))
        {
            // module is not loadable - therefore it's only for polluting the global scope
            if (module->instantiateGlobals(context))
                wrappedContext.addModule(module->getName()); // this just adds the name for tracing
        }
    }
    return true;
} //> initializeGlobals(...)
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::registerModule(Module *module)
{
    if (!module)
        return false;
    if (module->getName().empty())
        return false;
    if (hasModule(module->getName()))
        return false;
    // registering new module should also update the existing contexts?
    m_modules.emplace(module->getName(), module);
    return true;
} //> registerModule(...)
//>#--------------------------------------------------------------------------------------

script::Module *script::ScriptManager::getModule(const std::string &name)
{
    if (name.empty())
        return nullptr;
    auto found = m_modules.find(name);
    if (found == m_modules.end())
        return nullptr;
    return found->second;
} //> getModule(...)
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::hasModule(const std::string &name) const
{
    if (name.empty())
        return false;
    return m_modules.find(name) != m_modules.end();
} //> hasModule(...)
//>#--------------------------------------------------------------------------------------

void script::ScriptManager::releaseModules(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    for (auto &it : m_modules)
    {
        delete it.second;
        it.second = nullptr;
    }
    m_modules.clear();
} //> releaseModules()
//>#--------------------------------------------------------------------------------------

script::ScriptCallback *script::ScriptManager::createScriptCallback(void)
{
    return ScriptCallback::create(&ScriptManager::scriptCallbackHandler, this);
}
//>#--------------------------------------------------------------------------------------

script::ScriptCallback *script::ScriptManager::createScriptCallback(const LocalFunction &function)
{
    return ScriptCallback::create(&ScriptManager::scriptCallbackHandler, m_isolate, function, this);
}
//>#--------------------------------------------------------------------------------------

script::ScriptCallback *script::ScriptManager::createScriptCallback(const LocalString &script)
{
    return ScriptCallback::create(&ScriptManager::scriptCallbackHandler, m_isolate, script, this);
}
//>#--------------------------------------------------------------------------------------

script::ScriptTimerCallback *script::ScriptManager::createScriptTimerCallback(void)
{
    return ScriptTimerCallback::create(&ScriptManager::scriptCallbackHandler, this);
}
//>#--------------------------------------------------------------------------------------

script::ScriptTimerCallback *script::ScriptManager::createScriptTimerCallback(const LocalFunction &function)
{
    return ScriptTimerCallback::create(&ScriptManager::scriptCallbackHandler, m_isolate, function, this);
}
//>#--------------------------------------------------------------------------------------

script::ScriptTimerCallback *script::ScriptManager::createScriptTimerCallback(const LocalFunction &function, std::vector<LocalValue> const &args)
{
    auto callback = ScriptTimerCallback::create(&ScriptManager::scriptCallbackHandler, m_isolate, function, this);
    callback->setArguments(m_isolate, args);
    return callback;
}
//>#--------------------------------------------------------------------------------------

// void script::ScriptManager::evaluateModule(const std::string &script, const std::string &name)
//{
//  This is a special callback action, that's supposed to evaluate script source
//  in full (the whole file) and treat it as a module (ModuleCallback). In this
//  case it has to create a temporary context (different global scope).
//  Every module has it's own context assigned to it.
//  Modules are loaded via require() global.
//} //> evaluateModule(...)
//>#--------------------------------------------------------------------------------------

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
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingCallbacks.emplace(args, callback, (int)(args.size() - 1));
    }
    m_thread.wakeup(); // signal the thread because there are new callbacks pending
    return true;
} //> scriptCallbackHandler(...)
//>#--------------------------------------------------------------------------------------

void script::ScriptManager::processPendingCallbacks(void)
{
    v8::HandleScope handle_scope(m_isolate);
    auto context = m_isolate->GetEnteredContext();
    while (true)
    {
        PendingCallback top;
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (m_pendingCallbacks.empty())
                break;
            // move to new structure, will call destructor on next loop, releasing copied args vector
            top = std::move(m_pendingCallbacks.top());
            m_pendingCallbacks.pop();
        }
        v8::TryCatch tryCatch(m_isolate);
        top.callback->evaluate(m_isolate, top.args, top.numArgs);
        script::processException(m_isolate, context, tryCatch);
    }
} //> processPendingCallbacks(...)
//>#--------------------------------------------------------------------------------------
