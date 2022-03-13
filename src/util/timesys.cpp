#include <BuildConfig.hpp>
#include <util/Timesys.hpp>
#include <limits>

namespace timesys
{
    static double g_start;
    static double s_start = -1.0;
    static double s_current[NUM_TICK_CATEGORIES] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    static double s_lastTick[NUM_TICK_CATEGORIES] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

    const char *const g_TickCategoriesText[] = {
        "UPDATE",
        "PRERENDER",
        "RENDER",
        "EVENT",
        "SCRIPT",
        "NETWORK",
        "UNKNOWN"};
} // namespace timesys

#if defined(FG_USING_SDL2)
#include <SDL_timer.h>
#include <Unistd.hpp>
#endif

void timesys::init(void)
{
    g_start = ms() / 1000.0f; // seconds
    for (int id = 0; id < NUM_TICK_CATEGORIES; id++)
    {
        timesys::s_current[id] = 0.0;
        timesys::s_lastTick[id] = MINIMUM_TICK;
    }
}
//>---------------------------------------------------------------------------------------

void timesys::markTick(TickCategory category)
{
    double newTime = ms() / 1000.0f;
    auto catidx = (unsigned int)category;
    timesys::s_lastTick[catidx] = newTime - timesys::s_current[catidx];
    timesys::s_current[catidx] = newTime;
    if (timesys::s_lastTick[catidx] <= 0.0f)
        timesys::s_lastTick[catidx] = MINIMUM_TICK;
}
//>---------------------------------------------------------------------------------------

double timesys::elapsed(TickCategory category)
{
    return timesys::s_lastTick[(unsigned int)category];
}
//>---------------------------------------------------------------------------------------

double timesys::exact(void)
{
    auto ts = ms() / 1000.0f; // seconds
    return (ts - g_start);
}
//>---------------------------------------------------------------------------------------

double timesys::ms(void)
{
    return ((double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000.0f;
}
//>---------------------------------------------------------------------------------------

int64_t timesys::seconds(void)
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
//>---------------------------------------------------------------------------------------

int64_t timesys::ticks(void)
{
#if defined(FG_USING_SDL) || defined(FG_USING_SDL2)
    return (int64_t)SDL_GetTicks();
#else
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif
}
//>---------------------------------------------------------------------------------------
#if defined(FG_USING_WINDOWS)
#include <WindowsStandard.hpp>
#elif defined(FG_USING_LINUX)
#include <ctime>
#include <cerrno>
#endif
void timesys::sleep(unsigned int ms)
{
    if (!ms)
        ms = 5;
#if defined(FG_USING_WINDOWS)
    ::LARGE_INTEGER ft;
    ft.QuadPart = -static_cast<int64>(ms * 1000 * 1000 * 10); // '-' using relative time

    ::HANDLE timer = ::CreateWaitableTimer(NULL, TRUE, NULL);
    ::SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    ::WaitForSingleObject(timer, INFINITE);
    ::CloseHandle(timer);
#elif defined(FG_USING_LINUX)
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = ms % 1000 * 1000000;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
    {
    };
#endif
}
//>---------------------------------------------------------------------------------------
