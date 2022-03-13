#pragma once
#ifndef FG_INC_TIMESYS
#define FG_INC_TIMESYS

#include <chrono>

namespace timesys
{
    enum TickCategory
    {
        TICK_UPDATE = 0,
        TICK_PRERENDER = 1,
        TICK_RENDER = 2,
        TICK_EVENT = 3,
        TICK_SCRIPT = 4,
        TICK_NETWORK = 5,
        NUM_TICK_CATEGORIES = 6
    };

    inline const double MINIMUM_TICK = 0.001;
    inline const TickCategory DEFAULT_TICK_CATEGORY = TICK_PRERENDER;

    /**
     * First initial time stamp
     */
    void init(void);
    /**
     * Mark current time
     * @param category
     */
    void markTick(TickCategory category = DEFAULT_TICK_CATEGORY);

    /**
     * Return elapsed time since last tick (seconds)
     * @param category
     * @return
     */
    double elapsed(TickCategory category = DEFAULT_TICK_CATEGORY);
    /**
     * Get exact time since init (seconds)
     * @return
     */
    double exact(void);
    /**
     * Platform independent function for getting time in milliseconds
     * as a unsigned long value (for more accurate double - use ms()
     * This function gets time in miliseconds. It doesnt matter from what
     * point in time this is calculated - it is used for delta time mostly.
     * This function is very similar in usage as the SDL_GetTicks().
     * @return
     */
    int64_t ticks(void);
    /**
     * Get time since init in milliseconds
     * @return
     */
    double ms(void);
    /**
     * Get the number of seconds since 00:00 hours, Jan 1, 1970 UTC
     * (i.e., the current unix timestamp). Uses time(null)
     * @return
     */
    int64_t seconds(void);
    void sleep(unsigned int ms);
} //> namespace timesys

#endif //> FG_INC_TIMESYS