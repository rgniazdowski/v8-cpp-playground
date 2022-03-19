#pragma once
#ifndef FG_INC_SCRIPT_MANAGER
#define FG_INC_SCRIPT_MANAGER

#include <Manager.hpp>
#include <Singleton.hpp>
#include <util/Tag.hpp>
#include <util/Logger.hpp>
#include <util/Bindings.hpp>
#include <script/V8.hpp>

#include <unordered_map>
#include <stack>

namespace script
{
    class ScriptCallback;
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

    public:
        virtual bool initialize(void) override;
        virtual bool destroy(void) override;

        bool scriptCallbackHandler(const util::WrappedArgs &args);

        LocalContext getContext(const std::string &name = "main");

        inline v8::Isolate *getIsolate(void) { return m_isolate; }

    protected:
        void processPendingCallbacks(void);

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
            WrappedContext() : isolate(nullptr), self() {}
            WrappedContext(v8::Isolate *_isolate, LocalContext &_context) : isolate(_isolate), self()
            {
                self.Reset(isolate, _context);
            }
            WrappedContext(const WrappedContext &other) = delete;
            WrappedContext(WrappedContext &&other) : isolate(other.isolate)
            {
                self.Reset(isolate, other.self);
                other.self.Reset();
            }
            ~WrappedContext()
            {
                self.Reset(); // can only do a simple reset - dispose should be called before
                isolate = nullptr;
            }
            inline LocalContext local() const { return self.Get(isolate); }
            inline LocalContext local(v8::Isolate *_isolate) const { return self.Get(_isolate); }
        }; //# struct WrappedContext
        std::unordered_map<std::string, WrappedContext> m_contexts;
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
        std::stack<PendingCallback> m_pendingCallbacks;
        std::mutex m_mutex;
    }; //# class ScriptManager

} //> namespace script

#endif //> FG_INC_SCRIPT_MANAGER
