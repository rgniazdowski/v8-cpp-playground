/// Standard includes
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>

#include <EngineMain.hpp>
/// Various utilities
#include <util/Timesys.hpp>
#include <util/Profiling.hpp>
/// Resource management
#include <resource/ResourceManager.hpp>
/// Script management
#include <script/ScriptManager.hpp>
#include <script/ScriptCallback.hpp>
/// Event management
#include <event/EventManager.hpp>
#include <Unistd.hpp>

EngineMain::EngineMain(int argc, char **argv) : base_type(),
                                                m_argc(argc),
                                                m_argv(argv),
                                                m_frameControl(60, 1000),
                                                m_resourceMgr(nullptr)
{
    base_type::initialize();
    srand((unsigned int)time(nullptr));
    this->setEventManager();
    m_init.store(false);
    // Setting up the main thread for processing events
    m_thread.setFunction([this]()
                         {
        this->m_frameControl.process(true, this, &EngineMain::update);
        return true; });
    m_thread.setWakeable(false);
    // base::ManagerRegistry::instance()->add(this);
    //  Add the engine main as event manager to this registry (it is anyway) - Manager<EventManager> matters only
    base::ManagerRegistry::instance()->add(static_cast<event::EventManager *>(this));
    m_thread.setThreadName("EngineMain");
}
//>---------------------------------------------------------------------------------------

EngineMain::~EngineMain()
{
    logger::debug("Engine final memory cleanup...");
    script::ScriptManager::deleteInstance();
    m_scriptMgr = nullptr;
    if (m_resourceMgr)
    {
        logger::debug("Destroying the Resource Manager...");
        delete m_resourceMgr;
        m_resourceMgr = nullptr;
    }
}
//>---------------------------------------------------------------------------------------

bool EngineMain::releaseResources(void)
{
    if (m_resourceMgr)
    {
        logger::debug("Releasing resources...");
        return m_resourceMgr->destroy();
    }
    return false;
}
//>---------------------------------------------------------------------------------------

bool EngineMain::closeSybsystems(void)
{
    logger::debug("Closing subsystems...");
    self_type::releaseResources();
    return true;
}
//>---------------------------------------------------------------------------------------

bool EngineMain::destroy(void)
{
    if (!isInit())
        return false;
    logger::debug("Destroying the main Engine object...");
    executeEvent(event::Type::ProgramQuit);
    this->update();
    this->stopThread();
    bool status = true;
    if (!releaseResources())
        status = false;
    if (!closeSybsystems())
        status = false;
    return status;
}
//>---------------------------------------------------------------------------------------

void EngineMain::setEventManager(void)
{
    if (m_resourceMgr)
        m_resourceMgr->setEventManager(this);
}
//>---------------------------------------------------------------------------------------

bool EngineMain::initialize(void)
{
    if (isInit())
        return true;
    if (!m_resourceMgr)
        m_resourceMgr = new resource::ResourceManager(this);
    base::ManagerRegistry::instance()->add(m_resourceMgr);
    m_resourceMgr->setMaximumMemory(128 * 1024 * 1024 - 1024 * 1024 * 10); // #FIXME #TODO
    m_resourceMgr->initialize();
    setEventManager(); // FIXME
    m_scriptMgr = script::ScriptManager::instance(m_argv);
    base::ManagerRegistry::instance()->add(m_scriptMgr); // Add Script Manager to the registry
    m_scriptMgr->initialize();
    m_init.store(true);
    this->startThread();
    m_resourceMgr->startThread();
    m_scriptMgr->startThread();
    executeEvent(event::Type::ProgramInit);
    return true;
}
//>---------------------------------------------------------------------------------------

bool EngineMain::loadConfiguration(void)
{
    logger::debug("Loading configuration...");
    bool status = true;
    return status;
}
//>---------------------------------------------------------------------------------------

bool EngineMain::loadResources(void)
{
    auto t1 = timesys::ms();
    this->update();
    return true;
}
//>---------------------------------------------------------------------------------------

bool EngineMain::update()
{
    if (!m_init)
        return false;
    timesys::markTick(timesys::TICK_UPDATE);
    // Well this is really useful system, in the end GUI and others will be hooked
    // to EventManager so everything what needs to be done is done in this function
    event::EventManager::processEventsAndTimers();
    executeEvent(event::Type::UpdateShot);
    return true;
}
//>---------------------------------------------------------------------------------------
