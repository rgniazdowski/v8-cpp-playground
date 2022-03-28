#include <catch2/catch.hpp>

#include <iostream>
#include <chrono>
#include <thread>

#include <util/Timesys.hpp>
#include <util/FpsControl.hpp>
#include <event/EventManager.hpp>

static bool g_shouldExit = false;

void triggerExit(void)
{
    g_shouldExit = true;
}

event::EventManager *initializeEventManager(void);
void destroyEventManager(void);

TEST_CASE("Initialize event manager I", "[timers]")
{
    auto pEventMgr = initializeEventManager();
    destroyEventManager();
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Create timers with callbacks", "[timers]")
{
    auto pEventMgr = initializeEventManager();
    destroyEventManager();
}
//!---------------------------------------------------------------------------------------

uint32_t g_myTimerId = 0;
bool myTimer(void)
{
    static int count = 0;
    count++;
    if (count > 5)
    {
        triggerExit();
    }
    return true;
}

TEST_CASE("Trigger timers in fake loop", "[timers]")
{
    auto pEventMgr = initializeEventManager();
    g_myTimerId = pEventMgr->addInterval(250, &myTimer);

    util::FpsControl frameControl(30, 60);
    while (true)
    {
        frameControl.process(true, pEventMgr, &event::EventManager::processEventsAndTimers);
        if (!pEventMgr->hasTimers() || g_shouldExit)
            break;
    }
    destroyEventManager();
}
//!---------------------------------------------------------------------------------------