#include <BuildConfig.hpp>
#include <util/Timesys.hpp>

#if defined(FG_USING_PLATFORM_WINDOWS)
#include <WindowsStandard.hpp>
#include <timeapi.h>
#else
#include <sys/time.hpp>
#endif

namespace timesys
{
#if defined(FG_USING_PLATFORM_WINDOWS)
    static DWORD g_start;
#else
    static struct timeval g_start;
#endif
    static float s_start = -1.0;
    static float s_current[NUM_TICK_CATEGORIES] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    static float s_lastTick[NUM_TICK_CATEGORIES] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

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
#ifdef FG_USING_PLATFORM_WINDOWS
    g_start = timeGetTime();
#else
    gettimeofday(&timesys::g_start, NULL);
#endif
    for (int id = 0; id < NUM_TICK_CATEGORIES; id++)
    {
        timesys::s_current[id] = 0.0;
        timesys::s_lastTick[id] = MINIMUM_TICK;
    }
}
//>---------------------------------------------------------------------------------------

void timesys::markTick(TickCategory category)
{
#ifdef FG_USING_PLATFORM_WINDOWS
    float newTime = (float)timeGetTime() / 1000.0f;
#else
    struct timeval dtime;
    gettimeofday(&dtime, NULL);
    float newTime = float(dtime.tv_sec - timesys::g_start.tv_sec +
                          dtime.tv_usec / 1000000.0f - timesys::g_start.tv_usec / 1000000.0f);
#endif
    auto catidx = (unsigned int)category;
    timesys::s_lastTick[catidx] = newTime - timesys::s_current[catidx];
    timesys::s_current[catidx] = newTime;
    if (timesys::s_lastTick[catidx] <= 0.0f)
        timesys::s_lastTick[catidx] = MINIMUM_TICK;
}
//>---------------------------------------------------------------------------------------

float timesys::elapsed(TickCategory category)
{
    return timesys::s_lastTick[(unsigned int)category];
}
//>---------------------------------------------------------------------------------------

float timesys::exact(void)
{
#if defined(FG_USING_PLATFORM_WINDOWS)
    return (float)timeGetTime() / 1000.0f;
#else
    struct timeval dtime;
    gettimeofday(&dtime, NULL);
    return float(dtime.tv_sec - timesys::g_start.tv_sec) +
		float(dtime.tv_usec - timesys::g_start.tv_usec) / 1000000.0f);
#endif
}
//>---------------------------------------------------------------------------------------

float timesys::ms(void)
{
    return ((float)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000.0f;
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
    // return (int64_t)(timesys::ms());
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif
}
//>---------------------------------------------------------------------------------------