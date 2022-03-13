#pragma once
#ifndef FG_INC_ENGINE_MAIN
#define FG_INC_ENGINE_MAIN

#include <BuildConfig.hpp>
#include <util/Callbacks.hpp>
#include <util/Logger.hpp>
#include <event/EventManager.hpp>

namespace resource
{
    class ResourceManager;
}

class EngineMain : public event::EventManager
{
public:
    using self_type = EngineMain;
    using base_type = event::EventManager;

private:
    using tag_type = util::Tag<self_type>;
    using logger = logger::Logger<tag_type>;

public:
    EngineMain(int argc, char **argv);
    virtual ~EngineMain();

public:
    virtual bool initialize(void);

    bool loadConfiguration(void);
    bool loadResources(void);

    bool releaseResources(void);
    bool closeSybsystems(void);

    virtual bool destroy(void);

    bool quit(void) { return this->destroy(); }

    bool update(bool force = false);

public:
    inline resource::ResourceManager *getResourceManager(void) const { return m_resourceMgr; }
    inline event::EventManager *getEventManager(void) { return static_cast<event::EventManager *>(this); }

protected:
    void setEventManager(void);

private:
    /// Number of the arguments passed to program
    int m_argc;
    /// Array of arguments passed to program
    char **m_argv;
    /// Main Resource Manager
    resource::ResourceManager *m_resourceMgr;
    /// Builtin script subsystem - it needs access to all main managers
    // script::CScriptSubsystem* m_scriptSubsystem;
}; //> class EngineMain

#endif //> FG_INC_ENGINE_MAIN
