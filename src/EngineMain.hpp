#pragma once
#ifndef FG_INC_ENGINE_MAIN
#define FG_INC_ENGINE_MAIN

#include <BuildConfig.hpp>
#include <util/Callbacks.hpp>
#include <util/Logger.hpp>
#include <util/FpsControl.hpp>
#include <event/EventManager.hpp>

namespace resource
{
    class ResourceManager;
}

namespace script 
{
    class ScriptManager;
}

class EngineMain : public event::EventManager
{
public:
    using self_type = EngineMain;
    using base_type = event::EventManager;
    using tag_type = util::Tag<self_type>;
    using logger = ::logger::Logger<tag_type>;

public:
    EngineMain(int argc, char **argv);
    virtual ~EngineMain();

public:
    virtual bool initialize(void);

    bool loadConfiguration(void);
    bool loadResources(void);

    virtual bool destroy(void);

    bool update();

protected:
    bool releaseResources(void);
    bool closeSybsystems(void);

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
    ///
    util::FpsControl m_frameControl;
    /// Main Resource Manager, initialized manually
    resource::ResourceManager *m_resourceMgr;
    /// Builtin script subsystem - it needs access to all main managers
    script::ScriptManager* m_scriptMgr;
}; //> class EngineMain

#endif //> FG_INC_ENGINE_MAIN
