#pragma once
#ifndef FG_INC_APPLICATION
#define FG_INC_APPLICATION

#include <util/Timesys.hpp>
#include <util/Tag.hpp>
#include <util/Logger.hpp>
#include <EngineMain.hpp>

#include <functional>
#include <exception>
#include <csignal>

template <typename TEngineType>
class Application
{
    static_assert(std::is_base_of_v<EngineMain, TEngineType>,
                  "TEngineType template parameter type needs to be derived from EngineMain");

public:
    using self_type = Application<TEngineType>;
    using tag_type = util::Tag<self_type>;
    using logger = logger::Logger<tag_type>;

public:
    Application(int argc, char *argv[]) : m_argc(argc), m_argv(argv),
                                          m_appInit(),
                                          m_isExit(),
                                          m_isSuspend(),
                                          m_isFrameFreeze(),
                                          m_engineMain(nullptr)
    {
    }

    virtual ~Application()
    {
        if (m_appInit.load())
            cleanup(); // risky to call in destructor because calls out virtual functions
    }

private:
    Application(const self_type &other) = delete;
    Application(self_type &&other) = delete;

public:
    inline bool isInitialized(void) const { return m_appInit.load(); }
    inline bool isExit(void) const { return m_isExit.load(); }
    inline bool isFrameFreeze(void) const { return m_isFrameFreeze.load(); }
    inline void setExit(bool exit = true, int exitCode = 0)
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_isExit.store(exit);
        m_exitCode = exitCode;
    }
    inline void setFrameFreeze(bool freeze = true) { m_isFrameFreeze.store(freeze); }

    void cleanup(void);

private:
    bool initialize(void);
    bool mainLoopStep(void);

private:
    static void signalHandlerWrapper(int signum)
    {
        auto &handler = self_type::getSignalHandler();
        if (handler)
            handler(signum);
    }
    using SignalHandlerFunction = std::function<void(int)>;
    static SignalHandlerFunction &getSignalHandler(void)
    {
        static SignalHandlerFunction handler;
        return handler;
    }

public:
    int run(void);

protected:
    virtual bool preInitStep(void) = 0;
    virtual bool postInitStep(void) = 0;

    virtual void preLoopStep(void) = 0;
    virtual void postLoopStep(void) = 0;

    virtual void preQuitStep(void) = 0;
    virtual void postQuitStep(void) = 0;

protected:
    /// Number of arguments passed to program
    int m_argc;
    /// Array of arguments passed to program
    char **m_argv;
    /// Is app fully initialized?
    std::atomic_bool m_appInit;
    int m_exitCode;
    /// Is exit activated?
    std::atomic_bool m_isExit;
    std::atomic_bool m_isSuspend;
    std::atomic_bool m_isFrameFreeze;
    std::mutex m_mutex;
    EngineMain *m_engineMain;
}; //# class Application<TEngineType>
//#---------------------------------------------------------------------------------------

template <typename TEngineType>
int Application<TEngineType>::run(void)
{
    if (!m_appInit.load())
    {
        if (!initialize())
            throw std::exception("Failed to initialize and run the program");
    }
    // Main thread with busy loop is not wakeable so pre-loop step or post-loop step should
    // have locking capabilities or do something else to have proper frequency / FPS.
    util::SimpleThread self_thread([this]()
                                   {
        bool status = this->mainLoopStep();
        if (!status || this->isExit())
        {
            logger::trace("Main loop break...");
            return false;
        }
        return true; });
    self_thread.setThreadName(tag_type::name());
    self_thread.start();
    self_thread.join();
    if (m_appInit.load())
        cleanup(); // call cleanup just to be sure that it won't get called in destructor
    return m_exitCode;
}
//>---------------------------------------------------------------------------------------

template <typename TEngineType>
bool Application<TEngineType>::initialize(void)
{
    if (m_appInit.load())
        throw std::exception("It's invalid to call initialize() twice for Application<TEngineType>!");
    if (!preInitStep()) //* Custom pre-init step
        return false;
    std::vector<int> signals = {SIGINT, SIGILL, SIGFPE, SIGSEGV, SIGTERM, SIGBREAK, SIGABRT, SIGABRT_COMPAT};
    for (auto signum : signals)
        signal(signum, &self_type::signalHandlerWrapper);
    auto &handler = self_type::getSignalHandler();
    handler = [this](int signum)
    {
        std::vector<std::string_view> descriptions = {
            "interrupt",                                    // 0
            "illegal instruction - invalid function image", // 1
            "floating point exception",                     // 2
            "segment violation",                            // 3
            "Software termination signal from kill",        // 4
            "Ctrl-Break sequence",                          // 5
            "abnormal termination triggered by abort call", // 6
            "unknown signal / unhandled"                    // 7
        };
        int idx = -1;
        base::ManagerRegistry::instance()->signalAll();
        switch (signum)
        {
        case SIGINT:
            // interrupt
            signal(SIGINT, [](int signum)
                   { logger::debug("Interrupt signal received!"); });
            idx = 0;
            break;
        case SIGILL:
            // illegal instruction - invalid function image
            idx = 1;
            break;
        case SIGFPE:
            // floating point exception
            idx = 2;
            break;
        case SIGSEGV:
            // segment violation
            idx = 3;
            break;
        case SIGTERM:
            // Software termination signal from kill
            idx = 4;
            break;
        case SIGBREAK:
            // Ctrl-Break sequence
            idx = 5;
            break;
        case SIGABRT:
        case SIGABRT_COMPAT:
            // abnormal termination triggered by abort call
            idx = 6;
            break;
        default:
            idx = 7;
            break;
        };
        logger::debug("Interrupt signal received: '%d' - %s!", signum, descriptions[idx].data());
        this->setExit(true, signum);
        if (signum == SIGSEGV)
        {
            this->cleanup();
            exit(SIGSEGV);
        }
    };
    double t1 = timesys::ms();
    logger::debug("Init program main...");
    if (!m_engineMain)
    {
        logger::debug("Creating engine main object...");
        m_engineMain = new TEngineType(m_argc, m_argv);
    }
    // Well the whole configuration process should update the screen (swap buffers)
    // this is needed to display splash screens and show the game initialization
    // process by displaying the progress bar.
    if (!m_engineMain->loadConfiguration())
    {
        // TODO log fail load config
        return false;
    }
    // Initialize the main subsystems (gui, gfx and others)
    if (!m_engineMain->initialize())
    {
        // TODO log fail engine initialization
        return false;
    }
    // Preload any required resources
    if (!m_engineMain->loadResources())
    {
        // TODO log fail resource
        return false;
    }
    m_appInit.store(true);
    m_engineMain->update();
    double t2 = timesys::ms();
    logger::debug("Main: Program initialized in %.2f seconds", (t2 - t1) / 1000.0f);
    // trigger update frame in place, before thread starts
    m_engineMain->update();
    postInitStep(); //* Custom post-init step
    return true;
}
//>---------------------------------------------------------------------------------------

template <typename TEngineType>
bool Application<TEngineType>::mainLoopStep(void)
{
    if (!m_appInit.load())
    {
        logger::error("MainModule: Loop step - application not initialized...");
        return false;
    }
    preLoopStep(); //* Custom pre-loop step
    if (m_isExit.load())
    {
        logger::debug("Exit activated - breaking main loop!");
        return false;
    }
    postLoopStep(); //* Custom post-loop step
    return true;
}
//>---------------------------------------------------------------------------------------

template <typename TEngineType>
void Application<TEngineType>::cleanup(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_appInit.load())
        return;
    base::ManagerRegistry::instance()->signalAll();
    auto &registry = base::ManagerRegistry::instance()->getRegistryDirect();
    for (auto &it : registry)
    {
        // Stop every manager possible (just the loops/threads, not releasing any memory).
        // This also includes internal event manager (in this case EngineMain object).
        it.second.manager->stopThread();
    }
    logger::debug("Closing program...");
    preQuitStep(); //* Custom pre-quit step
    if (m_engineMain)
    {
        m_engineMain->destroy();
        delete m_engineMain;
        m_engineMain = nullptr;
    }
    postQuitStep(); //* Custom post-quit step
    m_appInit.store(false);
    m_isFrameFreeze.store(false);
    m_isSuspend.store(false);
    logger::trace("Cleanup fully finished");
}
//>---------------------------------------------------------------------------------------

#endif //> FG_INC_APPLICATION
