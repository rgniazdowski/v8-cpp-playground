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
/// Event management
#include <event/EventManager.hpp>
#include <Unistd.hpp>

EngineMain::EngineMain(int argc, char **argv) : m_argc(argc),
                                                m_argv(argv),
                                                m_resourceMgr(nullptr)
{
    if (!base_type::initialize())
    {
    }
    srand((unsigned int)time(nullptr));
    this->setEventManager();
    m_init = false;
}
//>---------------------------------------------------------------------------------------

EngineMain::~EngineMain()
{
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
    logger::debug("Game main quit requested");
    executeEvent(event::Type::ProgramQuit);
    this->update(true);
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
    if (!m_resourceMgr)
        m_resourceMgr = new resource::ResourceManager(this);
    m_resourceMgr->setMaximumMemory(128 * 1024 * 1024 - 1024 * 1024 * 10); // #FIXME #TODO
    m_resourceMgr->initialize();
    setEventManager();
    m_init = true;
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
    this->update(true);
    return true;
}
//>---------------------------------------------------------------------------------------

bool EngineMain::update(bool force)
{
    if (!m_init)
        return false;
#if 0
    static float t1 = -1.0f;
    float t2 = timesys::ms();
    float dt = 0.0f;
    const float msPerFrame = 1000.0f / (float)m_updateFixedFPS;
    if (t1 < 0.0f)
    {
        t1 = timesys::ms();
    }
    dt = t2 - t1;
    if (dt > msPerFrame)
    {
        t1 = t2;
    }
    else
#endif
    if (!force)
    {
        return false;
    }
    timesys::markTick(timesys::TICK_UPDATE);
#if 0
	int screenW = 1024, screenH = 600;
	if (m_gfxMain && m_gfxMain->getMainWindow()) {
		screenW = m_gfxMain->getMainWindow()->getWidth();
		screenH = m_gfxMain->getMainWindow()->getHeight();
	}
	if (m_guiMain)
		m_guiMain->setScreenSize(screenW, screenH);

	// Update logic manager
	if (m_gameMain)
		m_gameMain->update();
#endif
    // Well this is really useful system, in the end GUI and others will be hooked
    // to EventManager so everything what needs to be done is done in this function
    event::EventManager::processEventsAndTimers();

    executeEvent(event::Type::UpdateShot);
    return true;
}
//>---------------------------------------------------------------------------------------
