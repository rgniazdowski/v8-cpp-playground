#include <catch2/catch.hpp>

#include <event/EventManager.hpp>

event::EventManager *initializeEventManager(void)
{
    static event::EventManager *pEventMgr = nullptr;
    if (!pEventMgr)
    {
        pEventMgr = new event::EventManager();
        REQUIRE(pEventMgr->initialize());
    }
    return pEventMgr;
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