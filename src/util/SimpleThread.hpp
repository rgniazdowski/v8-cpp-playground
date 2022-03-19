#pragma once
#ifndef FG_INC_UTIL_SIMPLE_THREAD
#define FG_INC_UTIL_SIMPLE_THREAD

#include <util/Logger.hpp>

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <stdexcept>

namespace util
{
    void setThreadName(std::thread &thread, const char *threadName);

    class SimpleThread
    {
    public:
        using self_type = SimpleThread;
        using tag_type = util::Tag<self_type>;
        using logger = logger::Logger<tag_type>;

    public:
        using InnerFunction = std::function<bool(void)>;

        SimpleThread() : m_isWakeable(),
                         m_thread(),
                         m_function(),
                         m_running(),
                         m_abortRequested(),
                         m_wakeFlag(),
                         m_threadMutex(),
                         m_wakeCondition(),
                         m_name("thread"),
                         m_interval() {}

        SimpleThread(const InnerFunction &function, bool isWakeable = false, int interval = 0)
            : m_isWakeable(isWakeable),
              m_thread(),
              m_function(function),
              m_running(),
              m_abortRequested(),
              m_wakeFlag(),
              m_threadMutex(),
              m_wakeCondition(),
              m_name("thread"),
              m_interval(interval) {}

        ~SimpleThread()
        {
            abortAndJoin();
        }

        inline void setInterval(int interval) noexcept
        {
            m_interval = interval;
            if (interval > 0)
                m_isWakeable = true;
        }

        inline void setWakeable(bool toggle) noexcept
        {
            m_isWakeable = toggle;
            if (!toggle)
                m_interval = 0;
        }

        inline void setThreadName(std::string_view name) { m_name = name; }

        inline void stop(void) noexcept { abortAndJoin(); }

        inline bool isJoinable(void) const noexcept { return m_thread.joinable(); }

        inline bool isRunning(void) const noexcept { return m_running.load(); }

        inline bool isWakeable(void) const noexcept { return m_isWakeable; };

        void setFunction(const InnerFunction &function)
        {
            if (isRunning())
                return;
            m_function = function;
        }

        bool start(void)
        {
            if (!m_function)
                return false;
            if (m_running.load())
                return false;
            try
            {
                m_running.store(true);
                m_thread = std::thread(&SimpleThread::mainWrapper, this);
            }
            catch (...)
            {
                m_running.store(false);
                return false;
            }
            return true;
        }

        template <typename TFuncType>
        bool startWrapped(TFuncType wrapper)
        {
            if (!m_function)
                return false;
            if (m_running.load())
                return false;
            try
            {
                m_running.store(true);
                m_thread = std::thread(wrapper);
            }
            catch (...)
            {
                m_running.store(false);
                return false;
            }
            return true;
        }

        bool wakeup(void)
        {
            if (!m_isWakeable || !isRunning())
                return false;
            std::lock_guard<std::mutex> lock(m_threadMutex);
            m_wakeFlag.store(true);
            m_wakeCondition.notify_one();
            logger::trace("Notified thread '%s'", m_name.data());
            return true;
        }

        bool join(void)
        {
            if (m_thread.joinable())
            {
                m_thread.join();
                logger::trace("Joined thread '%s'", m_name.data());
                return true;
            }
            return false;
        }

    private:
        void abortAndJoin()
        {
            if (m_abortRequested.load())
                return; // skip!
            m_running.load() && logger::trace("Marked '%s' for closing", m_name.data());
            m_abortRequested.store(true);
            join();
            m_running.store(false);
        }

        void mainWrapper(void)
        {
            logger::debug("Starting thread '%s'...", m_name.data());
            util::setThreadName(m_thread, m_name.data());
            m_running.store(true);
            while (m_abortRequested.load() == false)
            {
                try
                {
                    std::unique_lock<std::mutex> lock(m_threadMutex);
                    if (m_isWakeable)
                    {
                        m_wakeCondition.wait_for(lock, std::chrono::milliseconds(m_interval),
                                                 [this]()
                                                 { return m_wakeFlag.load(); });
                        m_wakeFlag.store(false);
                    }
                    auto status = m_function();
                    if (!status)
                        break; // quit
                }
                catch (std::runtime_error &ex)
                {
                    logger::error("Runtime exception occurred in thread '%s': %s", m_name.data(), ex.what());
                    break;
                }
                catch (...)
                {
                    // Make sure that nothing leaves the thread for now...
                    break;
                }
            }
            m_running.store(false);
            logger::debug("Stopped thread '%s'", m_name.data());
        }

    protected:
        inline auto getThreadWrapper(void)
        {
            return &SimpleThread::mainWrapper;
        }

    private:
        bool m_isWakeable;
        std::thread m_thread;
        InnerFunction m_function;
        std::atomic_bool m_running;
        std::atomic_bool m_abortRequested;
        std::atomic_bool m_wakeFlag;
        std::mutex m_threadMutex;
        std::condition_variable m_wakeCondition;
        std::string m_name;
        int m_interval;
    }; //# SimpleThread

} //> namespace util

#endif //> FG_INC_UTIL_SIMPLE_THREAD
