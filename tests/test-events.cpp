#include <catch2/catch.hpp>

#include <event/EventManager.hpp>
#include <resource/GlobalObjectRegistry.hpp>

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
    destroyEventManager();
}
//!---------------------------------------------------------------------------------------

bool TestCallbackA(event::EventCombined *event)
{
    return true;
}

class TestClass
{
public:
    bool TestMemberCallbackA(event::EventCombined *event)
    {
        return true;
    }
};

TEST_CASE("Register callbacks for standard events", "[events]")
{
    auto testObject = new TestClass();
    auto pEventMgr = initializeEventManager();
    pEventMgr->addCallback(event::Type::ProgramInit, &TestCallbackA);
    pEventMgr->addCallback(event::Type::ProgramInit, &TestClass::TestMemberCallbackA, testObject);

    pEventMgr->isRegistered(testObject);
    pEventMgr->isRegistered(&TestCallbackA);

    pEventMgr->isRegistered(&TestClass::TestMemberCallbackA, (TestClass *)nullptr);
    pEventMgr->isRegistered(&TestClass::TestMemberCallbackA, testObject);

    pEventMgr->isRegistered(event::Type::ProgramInit, testObject);
    pEventMgr->isRegistered(event::Type::ProgramInit, &TestCallbackA);
    pEventMgr->isRegistered(event::Type::ProgramInit, &TestClass::TestMemberCallbackA, (TestClass *)nullptr);
    pEventMgr->isRegistered(event::Type::ProgramInit, &TestClass::TestMemberCallbackA, testObject);
    destroyEventManager();
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Trigger events in fake loop", "[events]")
{
    auto pEventMgr = initializeEventManager();
    destroyEventManager();
}
//!---------------------------------------------------------------------------------------