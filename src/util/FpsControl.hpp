#pragma once
#ifndef FG_UTIL_FPS_CONTROL
#define FG_UTIL_FPS_CONTROL

#include <cstdint>

#include <util/Timesys.hpp>
#include <thread>

namespace util
{
    struct FpsControl
    {
        inline static const int DEFAULT_MAX_FRAMESKIP = 5;
        inline static const int DEFAULT_FRAMERATE = 60;

        unsigned int targetUpdatesPerSecond;
        unsigned int targetWaitTicks;

        unsigned int maxUpdatesPerSecond;
        unsigned int minWaitTicks;

        unsigned int maxFrameSkip;

        int64_t nextUpdate;
        int64_t lastUpdate;

        unsigned int framesSkipped;
        double delta;

        FpsControl(unsigned int targetFps,
                   unsigned int maxFps) : targetUpdatesPerSecond(targetFps),
                                          targetWaitTicks(1000 / targetFps),
                                          maxUpdatesPerSecond(maxFps),
                                          minWaitTicks(1000 / maxFps),
                                          maxFrameSkip(DEFAULT_MAX_FRAMESKIP),
                                          nextUpdate(timesys::ticks()),
                                          lastUpdate(timesys::ticks()),
                                          framesSkipped(0),
                                          delta(0.0f) {}

        inline void prewait(void)
        {
            // This is not always reliable and only makes sense if not using signals in threads
            // or screen synchronization lock features.
            while (timesys::ticks() < lastUpdate + minWaitTicks)
                timesys::sleep(minWaitTicks / 2);
            lastUpdate = timesys::ticks();
        }
        //>-------------------------------------------------------------------------------

        template <typename UserClass, typename ReturnType, typename... Args>
        inline bool process(bool doPrewait, UserClass *pObject, ReturnType (UserClass::*method)(Args...), Args &&...args)
        {
            if (!pObject || !method)
                return false; //! skip
            const auto ts = timesys::ms();
            if (doPrewait)
                prewait(); // shouldn't be called at all if VSYNC is enabled
            framesSkipped = 0;
            while (timesys::ticks() > nextUpdate && framesSkipped < maxFrameSkip)
            {
                // execute the code in place
                (pObject->*method)(std::forward<Args>(args)...);
                // schedule next update:
                nextUpdate += targetWaitTicks;
                framesSkipped++;
            }
            delta = timesys::ms() - ts;
            return true;
        }
        //>-------------------------------------------------------------------------------

        template <typename ReturnType, typename... Args>
        inline bool process(bool doPrewait, ReturnType (*function)(Args...), Args &&...args)
        {
            if (!function)
                return false; //! skip
            const auto ts = timesys::ms();
            if (doPrewait)
                prewait();
            framesSkipped = 0;
            while (timesys::ticks() > nextUpdate && framesSkipped < maxFrameSkip)
            {
                // execute the code in place
                method(std::forward<Args>(args)...);
                // schedule next update:
                nextUpdate += targetWaitTicks;
                framesSkipped++;
            }
            delta = timesys::ms() - ts;
            return true;
        }
        //>-------------------------------------------------------------------------------

        inline float interpolation(void) const noexcept { return ((float)(timesys::ticks() + targetWaitTicks - nextUpdate)) / ((float)targetWaitTicks); }
    }; //# struct FpsControl
} //> namespace util

#endif //> FG_UTIL_FPS_CONTROL
