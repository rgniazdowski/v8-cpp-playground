#pragma once
#ifndef FG_INC_APPLICATION
#define FG_INC_APPLICATION

class EngineMain;

template <typename TEngineType>
class Application
{
    static_assert(std::is_base_of<EngineMain, TEngineType>::value,
                  "TEngineType template parameter type needs to be derived from EngineMain");

public:
    typedef Application self_type;

public:
    Application(int argc, char *argv[]) : m_argc(argc), m_argv(argv),
                                          m_appInit(false),
                                          m_isExit(false),
                                          m_isSuspend(false),
                                          m_isFrameFreeze(false),
                                          m_isMultithread(false),
                                          m_engineMain(nullptr) {}

    virtual ~Application() { closeProgram(); }

private:
    Application(const self_type &other) = delete;
    Application(self_type &&other) = delete;

public:
    inline bool isInitialized(void) const { return m_appInit; }
    inline bool isExit(void) const { return m_isExit; }
    inline bool isFrameFreeze(void) const { return m_isFrameFreeze; }
    inline bool isMultithread(void) const { return m_isMultithread; }

protected:
    inline void setExit(bool exit = true) { m_isExit = exit; }
    inline void setFrameFreeze(bool freeze = true) { m_isFrameFreeze = freeze; }

public:
    bool initProgram(void);
    bool mainLoopStep(void);
    void closeProgram(void);

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
    bool m_appInit;
    /// Is exit activated?
    bool m_isExit;
    bool m_isSuspend;
    bool m_isFrameFreeze;
    bool m_isMultithread;
    EngineMain *m_engineMain;
}; //# class Application

#endif //> FG_INC_APPLICATION
