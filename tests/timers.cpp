#include <catch2/catch.hpp>

#include <chrono>
#include <thread>

#include <util/Timesys.hpp>
#include <event/EventManager.hpp>

static bool g_shouldExit = false;

void triggerExit(void)
{
    g_shouldExit = true;
}

event::EventManager *initializeEventManager(void);

TEST_CASE("Initialize event manager I", "[timers]")
{
    auto pEventMgr = initializeEventManager();
    delete pEventMgr;
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Create timers with callbacks", "[timers]")
{
    auto pEventMgr = initializeEventManager();
    delete pEventMgr;
}
//!---------------------------------------------------------------------------------------

uint32_t g_myTimerId = 0;
bool myTimer(void) {
    static int count = 0;
    count++;
    if(count > 5) {
        triggerExit();
    }
    return true;
}

TEST_CASE("Trigger timers in fake loop", "[timers]")
{
    auto pEventMgr = initializeEventManager();
    g_myTimerId = pEventMgr->addInterval(1000, &myTimer);
    int UPDATES_PER_SECOND = 60;
    int WAIT_TICKS = 1000 / UPDATES_PER_SECOND;
    int MAX_UPDATES_PER_SECOND = 120;
    int MIN_WAIT_TICKS = 1000 / MAX_UPDATES_PER_SECOND;
    int MAX_FRAMESKIP = 5;
    auto nextUpdate = timesys::ticks();
    auto lastUpdate = timesys::ticks();
    int framesSkipped = 0;
    float interpolation = 0.0f;
    while (true)
    {
        while (timesys::ticks() < lastUpdate + MIN_WAIT_TICKS)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(MIN_WAIT_TICKS / 5));
        }
        lastUpdate = timesys::ticks();
        framesSkipped = 0;
        while (timesys::ticks() > nextUpdate && framesSkipped < MAX_FRAMESKIP)
        {
            // trigger timers (could potentially queue new microtasks, to be processed on next tick)
            pEventMgr->processEventsAndTimers();
            // Schedule next update:
            nextUpdate += WAIT_TICKS;
            framesSkipped++;
        }
        // Calculate interpolation for smooth animation between states:
        interpolation = ((float)(timesys::ticks() + WAIT_TICKS - nextUpdate)) / ((float)WAIT_TICKS);
        if (!pEventMgr->hasTimers() || g_shouldExit)
        {
            break;
        }
    }
    delete pEventMgr;
}
//!---------------------------------------------------------------------------------------