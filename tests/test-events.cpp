#include <catch2/catch.hpp>

#include <event/EventManager.hpp>

event::EventManager *g_eventMgr = nullptr;

event::EventManager *initializeEventManager(void)
{
    if (!g_eventMgr)
    {
        g_eventMgr = new event::EventManager();
        REQUIRE(g_eventMgr->initialize());
    }
    return g_eventMgr;
}

void destroyEventManager(void)
{
    if (g_eventMgr != nullptr)
        delete g_eventMgr;
    g_eventMgr = nullptr;
}

TEST_CASE("Initialize event manager II", "[events]")
{
    auto pEventMgr = initializeEventManager();
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Register callbacks for standard events", "[events]")
{
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Trigger events in fake loop", "[events]")
{
}
//!---------------------------------------------------------------------------------------