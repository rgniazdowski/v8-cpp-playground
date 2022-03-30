#include <script/ScriptManager.hpp>
#include <script/ScriptCallback.hpp>
#include <script/ScriptResource.hpp>
#include <script/modules/Console.hpp>
#include <script/modules/Timers.hpp>
#include <script/modules/Events.hpp>
#include <script/modules/Resources.hpp>

#include <util/Util.hpp>
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
    m_isolate->SetHostImportModuleDynamicallyCallback(ScriptManager::onHostImportModuleDynamically);
    m_isolate->SetHostInitializeImportMetaObjectCallback(ScriptManager::onHostInitializeImportMetaObject);
    {
        v8::Isolate::Scope isolate_scope(m_isolate);
        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(m_isolate);
        std::initializer_list<script::InternalModule *> modules = {
            new modules::Console(m_isolate),  /* Need to create and initialize list of globals and built-in modules. */
            new modules::Timers(m_isolate),   /* Timers wrapped - create callbacks and timers on EventManager */
            new modules::Events(m_isolate),   /* Events - this is EventManager global along with additional info */
            new modules::Resources(m_isolate) /* Resources - this is ResourceManager global along with additional info */
        };
        for (auto module : modules)
        {
            module->initialize();
            registerModule(module);
        }
        // Create a new context.
        // Main context is registered - now can create and manage all global objects before
        // entering (in another thread) a special event loop for microtasks and callbacks.
        // Globals are also initialized within.
        LocalContext context = createContext("main");
        v8::Context::Scope context_scope(context);     // FIXME
        this->executeModule("./data/main-module.mjs"); // FIXME
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
    clearContexts();
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
    v8::EscapableHandleScope handle_scope(m_isolate);
    LocalContext context = v8::Context::New(m_isolate);
    v8::Context::Scope context_scope(context);
    auto it = m_contexts.emplace(name, WrappedContext(m_isolate, context)).first;
    instantiateGlobals(context, it->second);
    return handle_scope.Escape(context);
} //> createContext(...)
//>#--------------------------------------------------------------------------------------

script::ScriptManager::ModuleEmbedderData *script::ScriptManager::getModuleDataForContext(const std::string &name)
{
    if (name.empty())
        return nullptr;
    auto it = m_contexts.find(name);
    if (it == m_contexts.end())
        return nullptr;
    return it->second.embedderData.get();
}
//>#--------------------------------------------------------------------------------------

script::ScriptManager::ModuleEmbedderData *script::ScriptManager::getModuleDataForContext(LocalContext context)
{
    return static_cast<ModuleEmbedderData *>(context->GetAlignedPointerFromEmbedderData(0)); //#FIXME -- need enum for index
}
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

void script::ScriptManager::clearContexts(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    v8::Locker locker(m_isolate);
    v8::Isolate::Scope isolate_scope(m_isolate);
    v8::HandleScope handle_scope(m_isolate);
    m_contexts.clear(); // reset the context map completely
} //> clearContexts()
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

void script::ScriptManager::onModuleResolutionSuccess(const FunctionCallbackInfo &info)
{
    std::unique_ptr<ModuleResolutionData> module_resolution_data(
        static_cast<ModuleResolutionData *>(
            info.Data().As<v8::External>()->Value()));
    v8::Isolate *isolate(module_resolution_data->isolate);
    v8::HandleScope handle_scope(isolate);
    LocalResolver resolver(module_resolution_data->resolver.Get(isolate));
    LocalValue module_namespace(module_resolution_data->module_namespace.Get(isolate));
    LocalContext context = module_resolution_data->context.Get(isolate);
    v8::Context::Scope context_scope(context);
    resolver->Resolve(context, module_namespace).ToChecked();
} //> onModuleResolutionSuccess(...)
//>#--------------------------------------------------------------------------------------

void script::ScriptManager::onModuleResolutionFailure(const FunctionCallbackInfo &info)
{
    std::unique_ptr<ModuleResolutionData> module_resolution_data(
        static_cast<ModuleResolutionData *>(
            info.Data().As<v8::External>()->Value()));
    v8::Isolate *isolate(module_resolution_data->isolate);
    v8::HandleScope handle_scope(isolate);
    LocalResolver resolver(module_resolution_data->resolver.Get(isolate));
    LocalContext context = module_resolution_data->context.Get(isolate);
    v8::Context::Scope context_scope(context);
    // DCHECK_EQ(info.Length(), 1);
    resolver->Reject(context, info[0]).ToChecked();
} //> onModuleResolutionFailure(...)
//>#--------------------------------------------------------------------------------------

script::MaybeLocalPromise script::ScriptManager::onHostImportModuleDynamically(
    LocalContext context, LocalScriptOrModule referrer, LocalString specifier)
{
    v8::Isolate *isolate = context->GetIsolate();
    MaybeLocalResolver maybe_resolver = v8::Promise::Resolver::New(context);
    LocalResolver resolver;
    if (maybe_resolver.ToLocal(&resolver))
    {
        DynamicImportData *data = new DynamicImportData(
            isolate, LocalString::Cast(referrer->GetResourceName()), specifier, resolver);
        isolate->EnqueueMicrotask(ScriptManager::doHostImportModuleDynamically, data);
        return resolver->GetPromise();
    }
    return MaybeLocalPromise();
} //> onHostImportModuleDynamically(...)
//>#--------------------------------------------------------------------------------------

void script::ScriptManager::doHostImportModuleDynamically(void *import_data)
{
    std::unique_ptr<DynamicImportData> import_data_(
        static_cast<DynamicImportData *>(import_data));
    v8::Isolate *isolate(import_data_->isolate);
    v8::HandleScope handle_scope(isolate);
    auto self = ScriptManager::instance(nullptr);

    LocalString referrer(import_data_->referrer.Get(isolate));
    LocalString specifier(import_data_->specifier.Get(isolate));
    LocalResolver resolver(import_data_->resolver.Get(isolate));

    // PerIsolateData *data = PerIsolateData::Get(isolate);
    // LocalContext realm = data->realms_[data->realm_current_].Get(isolate);
    LocalContext context = isolate->GetEnteredContext(); // FIXME
    v8::Context::Scope context_scope(context);

    std::string source_url = v8pp::from_v8<std::string>(isolate, referrer);
    std::string dir_name =
        path::dirName(path::normalize(source_url, path::getCurrentWorkingPath()));
    std::string file_name = v8pp::from_v8<std::string>(isolate, specifier);
    std::string absolute_path = path::normalize(file_name, dir_name);

    v8::TryCatch try_catch(isolate);
    try_catch.SetVerbose(true);

    ModuleEmbedderData *embedderData = self->getModuleDataForContext(context);
    LocalModule root_module;
    auto module_it = embedderData->specifier_to_module_map.find(absolute_path);
    if (module_it != embedderData->specifier_to_module_map.end())
    {
        root_module = module_it->second.Get(isolate);
    }
    else if (!self->fetchModuleTree(context, absolute_path).ToLocal(&root_module))
    {
        // CHECK(try_catch.HasCaught());
        resolver->Reject(context, try_catch.Exception()).ToChecked();
        return;
    }
    MaybeLocalValue maybe_result;
    if (root_module->InstantiateModule(context, ScriptManager::onResolveModule).FromMaybe(false))
    {
        maybe_result = root_module->Evaluate(context);
        // CHECK_IMPLIES(i::FLAG_harmony_top_level_await, !maybe_result.IsEmpty());
        self->internalEmptyMessageQueues(isolate); //#FIXME
    }

    LocalValue result;
    if (!maybe_result.ToLocal(&result))
    {
        // DCHECK(try_catch.HasCaught());
        resolver->Reject(context, try_catch.Exception()).ToChecked();
        return;
    }

    LocalValue module_namespace = root_module->GetModuleNamespace();
    // if (i::FLAG_harmony_top_level_await)
    //{
    LocalPromise result_promise(LocalPromise::Cast(result));
    if (result_promise->State() == v8::Promise::kRejected)
    {
        resolver->Reject(context, result_promise->Result()).ToChecked();
        return;
    }

    // Setup callbacks, and then chain them to the result promise.
    // ModuleResolutionData will be deleted by the callbacks.
    auto module_resolution_data =
        new ModuleResolutionData(isolate, module_namespace, resolver, context);
    LocalExternal edata = v8::External::New(isolate, module_resolution_data);
    LocalFunction callback_success;
    /*CHECK*/ (v8::Function::New(context, ScriptManager::onModuleResolutionSuccess, edata)
                   .ToLocal(&callback_success));
    LocalFunction callback_failure;
    /*CHECK*/ (v8::Function::New(context, ScriptManager::onModuleResolutionFailure, edata)
                   .ToLocal(&callback_failure));
    result_promise->Then(context, callback_success, callback_failure).ToLocalChecked();
    //}
    // else
    //{
    //    // TODO(cbruni): Clean up exception handling after introducing new
    //    // API for evaluating async modules.
    //    // DCHECK(!try_catch.HasCaught());
    //    resolver->Resolve(realm, module_namespace).ToChecked();
    //}
} //> doHostImportModuleDynamically(...)
//>#--------------------------------------------------------------------------------------

void script::ScriptManager::onHostInitializeImportMetaObject(LocalContext context,
                                                             LocalModule module,
                                                             LocalObject meta)
{
    v8::Isolate *isolate = context->GetIsolate();
    v8::HandleScope handle_scope(isolate);
    auto self = ScriptManager::instance(nullptr);
    ModuleEmbedderData *embedderData = self->getModuleDataForContext(context);
    auto specifier_it = embedderData->module_to_specifier_map.find(GlobalModule(isolate, module));
    //  CHECK(specifier_it != d->module_to_specifier_map.end()); // ASSERT?

    LocalString url_key = v8::String::NewFromUtf8Literal(isolate, "url", v8::NewStringType::kInternalized);
    LocalString url = v8pp::to_v8(isolate, specifier_it->second);
    meta->CreateDataProperty(context, url_key, url).ToChecked();
} //> onHostInitializeImportMetaObject(...)
//>#--------------------------------------------------------------------------------------

script::MaybeLocalModule script::ScriptManager::onResolveModule(LocalContext context,
                                                                LocalString specifier,
                                                                LocalModule referrer)
{
    auto isolate = context->GetIsolate();
    auto self = ScriptManager::instance(nullptr);
    ModuleEmbedderData *embedderData = self->getModuleDataForContext(context);
    auto specifier_it =
        embedderData->module_to_specifier_map.find(GlobalModule(isolate, referrer));
    // CHECK(specifier_it != d->module_to_specifier_map.end());
    std::string absolute_path = path::normalize(
        v8pp::from_v8<std::string>(isolate, specifier),
        path::dirName(specifier_it->second));
    auto module_it = embedderData->specifier_to_module_map.find(absolute_path);
    // CHECK(module_it != d->specifier_to_module_map.end());
    return module_it->second.Get(isolate);
} //> onResolveModule(...)
//>#--------------------------------------------------------------------------------------

script::MaybeLocalModule script::ScriptManager::fetchModuleTree(LocalContext context, std::string_view filePath)
{
    auto isolate = context->GetIsolate();
    auto self = ScriptManager::instance(nullptr);
    auto file_name = std::string(filePath);
    ModuleEmbedderData *embedderData = self->getModuleDataForContext(context);
    auto module_it = embedderData->specifier_to_module_map.find(file_name);
    if (module_it != embedderData->specifier_to_module_map.end())
        return module_it->second.Get(isolate);
    LocalString source_text;
    auto fileContent = util::File::loadInPlaceAsString(filePath);
    if (!fileContent.empty())
        source_text = v8pp::to_v8(isolate, fileContent);
    if (source_text.IsEmpty())
    {
        std::string fallback_file_name;
        fallback_file_name.append(filePath);
        fallback_file_name.append(".js");
        {
            fileContent = util::File::loadInPlaceAsString(fallback_file_name);
            if (!fileContent.empty())
                source_text = v8pp::to_v8(isolate, fileContent);
        }
        if (source_text.IsEmpty())
        {
            fallback_file_name.clear();
            fallback_file_name.append(filePath);
            fallback_file_name.append(".mjs");
            fileContent = util::File::loadInPlaceAsString(fallback_file_name);
            if (!fileContent.empty())
                source_text = v8pp::to_v8(isolate, fileContent);
        }
    }
    if (source_text.IsEmpty())
    {
        isolate->ThrowException(v8pp::to_v8(isolate, "Error reading: " + std::string(filePath)));
        return MaybeLocalModule();
    }
    LocalModule module;
    v8::ScriptOrigin origin(v8pp::to_v8(isolate, filePath.data()),
                            LocalInteger(), LocalInteger(), LocalBoolean(), LocalInteger(),
                            LocalValue(), LocalBoolean(), LocalBoolean(), True(isolate));
    v8::ScriptCompiler::Source source(source_text, origin);
    if (!v8::ScriptCompiler::CompileModule(isolate, &source).ToLocal(&module))
    {
        return MaybeLocalModule();
    }
    /*CHECK*/ embedderData->specifier_to_module_map
        .insert(std::make_pair(file_name, GlobalModule(isolate, module)));
    /*CHECK*/ embedderData->module_to_specifier_map
        .insert(std::make_pair(GlobalModule(isolate, module), file_name));
    std::string dir_name = path::dirName(file_name);
    for (int i = 0, length = module->GetModuleRequestsLength(); i < length; ++i)
    {
        LocalString name = module->GetModuleRequest(i);
        std::string absolute_path =
            path::normalize(v8pp::from_v8<std::string>(isolate, name), dir_name);
        if (embedderData->specifier_to_module_map.count(absolute_path))
            continue;
        if (self->fetchModuleTree(context, absolute_path).IsEmpty())
            return MaybeLocalModule();
    }
    return module;
}
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::executeModule(std::string_view filePath)
{
    v8::HandleScope handle_scope(m_isolate);
    auto context = m_isolate->GetCurrentContext(); // #FIXME
    // PerIsolateData *data = PerIsolateData::Get(isolate);
    // Local<Context> realm = data->realms_[data->realm_current_].Get(isolate);
    v8::Context::Scope context_scope(context);
    std::string absolute_path = path::normalize(filePath, path::getCurrentWorkingPath());
    // Use a non-verbose TryCatch and report exceptions manually using
    // Shell::ReportException, because some errors (such as file errors) are
    // thrown without entering JS and thus do not trigger
    // isolate->ReportPendingMessages().
    v8::TryCatch try_catch(m_isolate);
    LocalModule root_module;
    if (!this->fetchModuleTree(context, absolute_path).ToLocal(&root_module))
    {
        // CHECK(try_catch.HasCaught());
        script::processException(m_isolate, context, try_catch);
        return false;
    }
    MaybeLocalValue maybe_result;
    if (root_module->InstantiateModule(context, ScriptManager::onResolveModule).FromMaybe(false))
    {
        maybe_result = root_module->Evaluate(context);
        // CHECK_IMPLIES(i::FLAG_harmony_top_level_await, !maybe_result.IsEmpty());
        this->internalEmptyMessageQueues(m_isolate);
    }
    LocalValue result;
    if (!maybe_result.ToLocal(&result))
    {
        // DCHECK(try_catch.HasCaught());
        script::processException(m_isolate, context, try_catch);
        return false;
    }
    // if (i::FLAG_harmony_top_level_await)
    //{
    //  Loop until module execution finishes
    //  TODO(cbruni): This is a bit wonky. "Real" engines would not be
    //  able to just busy loop waiting for execution to finish.
    if (result->IsPromise())
    {
        LocalPromise result_promise(LocalPromise::Cast(result));
        while (result_promise->State() == v8::Promise::kPending)
            m_isolate->PerformMicrotaskCheckpoint();
        if (result_promise->State() == v8::Promise::kRejected)
        {
            // If the exception has been caught by the promise pipeline, we rethrow
            // here in order to ReportException.
            // TODO(cbruni): Clean this up after we create a new API for the case
            // where TLA is enabled.
            if (!try_catch.HasCaught())
                m_isolate->ThrowException(result_promise->Result());
            // else
            //{
            // DCHECK_EQ(try_catch.Exception(), result_promise->Result());
            //}
            script::processException(m_isolate, context, try_catch);
            return false;
        }
    }
    // DCHECK(!try_catch.HasCaught());
    return true;
}
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::internalProcessMessages(v8::Isolate *isolate,
                                                    const std::function<v8::platform::MessageLoopBehavior()> &behavior)
{
    // v8::internal::Isolate *i_isolate = reinterpret_cast<v8::internal::Isolate *>(isolate);
    // v8::internal::SaveAndSwitchContext saved_context(i_isolate, v8::internal::Context());
    auto platform = m_platform.get();
    v8::SealHandleScope shs(isolate);
    while (v8::platform::PumpMessageLoop(platform, isolate, behavior()))
        v8::MicrotasksScope::PerformCheckpoint(isolate);
    if (platform->IdleTasksEnabled(isolate))
        v8::platform::RunIdleTasks(platform, isolate, 50.0 / 100.0);
    return true;
}
//>#--------------------------------------------------------------------------------------

bool script::ScriptManager::internalEmptyMessageQueues(v8::Isolate *isolate)
{
    return internalProcessMessages(isolate, []()
                                   { return v8::platform::MessageLoopBehavior::kDoNotWait; });
}
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
