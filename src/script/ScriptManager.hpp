#pragma once
#ifndef FG_INC_SCRIPT_MANAGER
#define FG_INC_SCRIPT_MANAGER

#include <Manager.hpp>
#include <Singleton.hpp>
#include <util/Tag.hpp>
#include <util/Logger.hpp>
#include <util/Bindings.hpp>
#include <script/V8.hpp>
#include <script/Module.hpp>

#include <unordered_map>
#include <stack>

namespace script
{
    class ScriptCallback;
    class ScriptTimerCallback;
    /**
     * For now it should manage just one top Isolate and one main context with possibility
     * to create more named contexts and switch between them.
     *
     * Script Manager should also add support for registering timers and event callbacks
     * from scripts back to the Event Manager. Now the execution of a given script callback
     * should happen in a correct context and isolate, and in later stages also thread.
     * This is the reason why it should be possible (from Event Manager's level) to split
     * list of thrown events and active timers based on criteria and execute separate loop.
     *
     * Script/V8 based timer uses the same internal structure but differs in callback type.
     *
     * Script Manager will also initialize a socket and register debug handler with V8.
     */
    class ScriptManager : public base::Manager<ScriptManager>, public fg::Singleton<ScriptManager>
    {
    public:
        using self_type = ScriptManager;
        using manager_type = base::Manager<self_type>;
        friend class fg::Singleton<self_type>;
        using tag_type = util::Tag<self_type>;
        using logger = ::logger::Logger<tag_type>;

    protected:
        ScriptManager(char **argv);
        ScriptManager(const self_type &other) = delete;
        ScriptManager(self_type &&other) = delete;
        virtual ~ScriptManager();

        using ModuleRegistry = std::unordered_map<std::string, Module *>;

    public:
        virtual bool initialize(void) override;
        virtual bool destroy(void) override;

        inline v8::Isolate *getIsolate(void) { return m_isolate; }
        LocalContext getContext(const std::string &name = "main");
        LocalContext createContext(const std::string &name);

        bool registerModule(Module *module);
        Module *getModule(const std::string &name);
        bool hasModule(const std::string &name) const;
        void releaseModules(void);
        void clearContexts(void);

        //#-------------------------------------------------------------------------------

        ScriptCallback *createScriptCallback(void);
        ScriptCallback *createScriptCallback(const LocalFunction &function);
        ScriptCallback *createScriptCallback(const LocalString &script);
        ScriptTimerCallback *createScriptTimerCallback(void);
        ScriptTimerCallback *createScriptTimerCallback(const LocalFunction &function);
        ScriptTimerCallback *createScriptTimerCallback(const LocalFunction &function, std::vector<LocalValue> const &args);

        bool scriptCallbackHandler(const util::WrappedArgs &args);
        //#-------------------------------------------------------------------------------

        static void onModuleResolutionSuccess(const FunctionCallbackInfo &info);

        static void onModuleResolutionFailure(const FunctionCallbackInfo &info);

        static MaybeLocalPromise onHostImportModuleDynamically(LocalContext context,
                                                               LocalScriptOrModule referrer,
                                                               LocalString specifier);
        static void doHostImportModuleDynamically(void *import_data);

        static void onHostInitializeImportMetaObject(LocalContext context,
                                                     LocalModule module, LocalObject meta);

        static MaybeLocalModule onResolveModule(LocalContext context,
                                                LocalString specifier,
                                                LocalModule referrer);

    protected:
        MaybeLocalModule fetchModuleTree(LocalContext context, std::string_view filePath);

        bool executeModule(std::string_view filePath);

        bool internalProcessMessages(v8::Isolate *isolate,
                                     const std::function<v8::platform::MessageLoopBehavior()> &behavior);

        bool internalEmptyMessageQueues(v8::Isolate *isolate);

        void processPendingCallbacks(void);

        struct DynamicImportData
        {
            DynamicImportData(v8::Isolate *isolate_, LocalString referrer_,
                              LocalString specifier_, LocalResolver resolver_)
                : isolate(isolate_)
            {
                referrer.Reset(isolate, referrer_);
                specifier.Reset(isolate, specifier_);
                resolver.Reset(isolate, resolver_);
            }
            v8::Isolate *isolate;
            GlobalString referrer;
            GlobalString specifier;
            GlobalResolver resolver;
        };
        //#-------------------------------------------------------------------------------
        struct ModuleResolutionData
        {
            ModuleResolutionData(v8::Isolate *isolate_, LocalValue module_namespace_,
                                 LocalResolver resolver_, LocalContext context_)
                : isolate(isolate_)
            {
                module_namespace.Reset(isolate, module_namespace_);
                resolver.Reset(isolate, resolver_);
                context.Reset(isolate, context_);
            }
            v8::Isolate *isolate;
            GlobalValue module_namespace;
            GlobalResolver resolver;
            GlobalContext context;
        };
        //#-------------------------------------------------------------------------------
        class ModuleEmbedderData
        {
        private:
            class ModuleGlobalHash
            {
            public:
                explicit ModuleGlobalHash(v8::Isolate *isolate) : isolate_(isolate) {}
                size_t operator()(const GlobalModule &module) const
                {
                    return module.Get(isolate_)->GetIdentityHash();
                }

            private:
                v8::Isolate *isolate_;
            };

        public:
            explicit ModuleEmbedderData(v8::Isolate *isolate) : module_to_specifier_map(10, ModuleGlobalHash(isolate)) {}

            // Map from normalized module specifier to Module.
            std::unordered_map<std::string, GlobalModule> specifier_to_module_map;
            // Map from Module to its URL as defined in the ScriptOrigin
            std::unordered_map<GlobalModule, std::string, ModuleGlobalHash> module_to_specifier_map;
        };
        ModuleEmbedderData *getModuleDataForContext(const std::string &name = "main");
        ModuleEmbedderData *getModuleDataForContext(LocalContext context);
        //#-------------------------------------------------------------------------------
    protected:
        char **m_argv;
        v8::Isolate::CreateParams m_createParams;
        /// For now it's only feasable to have one main isolate for all and only use multiple
        /// named contexts when needed. It's required to use locks if isolate is to be
        /// accessed via multiple threads. Another issue is with Event Manager as timers
        /// and event callbacks that are V8 based need to be executed in context's scope,
        /// with isolate and context 'entered' and 'locked'.
        v8::Isolate *m_isolate;
        std::unique_ptr<v8::Platform> m_platform;
        //#-------------------------------------------------------------------------------
        /**
         * Wrap around a persistent context so it can have move constructor and placed in
         * STL container like map. Ideally contexts will be mapped with a string key.
         * This is a helper to be able to create multiple named contexts and initialize
         * them easily on demand.
         */
        struct WrappedContext
        {
            v8::Isolate *isolate;
            PersistentContext self;
            std::vector<std::string> modules;
            std::unique_ptr<ModuleEmbedderData> embedderData; // FIXME - module embedder data / per context config
            WrappedContext() : isolate(nullptr), self(), modules(), embedderData() {}
            WrappedContext(v8::Isolate *_isolate, LocalContext &_context)
                : isolate(_isolate), self(), modules(), embedderData()
            {
                embedderData.reset(new ModuleEmbedderData(_isolate));
                _context->SetAlignedPointerInEmbedderData(0, embedderData.get());
                self.Reset(isolate, _context);
            }
            WrappedContext(const WrappedContext &other) = delete;
            WrappedContext(WrappedContext &&other) : isolate(other.isolate), embedderData(std::move(other.embedderData))
            {
                self.Reset(isolate, other.self);
                other.self.Reset();
            }
            ~WrappedContext()
            {
                // Best to call this before disposing of the isolate and dependent contexts
                if (isolate)
                {
                    v8::HandleScope handle_scope(isolate);
                    if (!self.IsEmpty())
                    {
                        auto context = local();
                        if (!context.IsEmpty())
                            context->SetAlignedPointerInEmbedderData(0, nullptr);
                    }
                }
                isolate = nullptr;
                self.Reset();
            }
            inline LocalContext local() const { return self.Get(isolate); }
            inline LocalContext local(v8::Isolate *_isolate) const { return self.Get(_isolate); }
            inline void addModule(const std::string &name)
            {
                if (hasModule(name))
                    return;
                modules.push_back(name);
            }
            inline bool hasModule(const std::string &name)
            {
                auto idx = util::find(modules, name);
                return idx >= 0;
            }
            inline void removeModule(const std::string &name)
            {
                auto idx = util::find(modules, name);
                if (idx < 0)
                    return;
                util::remove(modules, (size_t)idx);
            }
        }; //# struct WrappedContext
        //#-------------------------------------------------------------------------------
        bool instantiateGlobals(LocalContext &context, WrappedContext &wrappedContext);
        using ContextCache = std::unordered_map<std::string, WrappedContext>;
        ContextCache m_contexts;
        ModuleRegistry m_modules;
        struct PendingCallback
        {
            util::WrappedArgs args;
            int numArgs;
            ScriptCallback *callback;
            PendingCallback() : args(), numArgs(0), callback(nullptr) {}
            PendingCallback(const util::WrappedArgs &_args, ScriptCallback *_callback, int _numArgs) : args(), callback(_callback), numArgs(_numArgs) { util::copy_arguments(_args, args); }
            PendingCallback(const PendingCallback &other) : callback(other.callback), numArgs(other.numArgs) { util::copy_arguments(other.args, args); }
            PendingCallback(PendingCallback &&other) : args(std::move(other.args)), callback(other.callback), numArgs(other.numArgs)
            {
                other.callback = nullptr;
                other.numArgs = 0;
            }
            PendingCallback &operator=(const PendingCallback &other)
            {
                util::copy_arguments(other.args, args);
                callback = other.callback;
                numArgs = other.numArgs;
                return *this;
            }
            PendingCallback &operator=(PendingCallback &&other)
            {
                if (this != &other)
                {
                    args = std::move(other.args);
                    callback = other.callback;
                    numArgs = other.numArgs;
                    other.callback = nullptr;
                    other.numArgs = 0;
                }
                return *this;
            }
            ~PendingCallback()
            {
                // need to call destructors on input arguments - it's a copy
                util::reset_arguments(args);
                callback = nullptr;
                numArgs = 0;
            }
        }; //# struct PendingCallback
        //#-------------------------------------------------------------------------------
        std::stack<PendingCallback> m_pendingCallbacks;
        std::mutex m_mutex;
    }; //# class ScriptManager

} //> namespace script

#endif //> FG_INC_SCRIPT_MANAGER
