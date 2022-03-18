#pragma once
#ifndef FG_INC_UTIL_SIMPLE_THREAD
#define FG_INC_UTIL_SIMPLE_THREAD

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <stdexcept>

namespace util
{
    class SimpleThread
    {
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

        inline void setWakeable(bool toggle) noexcept { m_isWakeable = toggle; }

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
            return true;
        }

        bool join(void)
        {
            if (m_thread.joinable())
            {
                m_thread.join();
                return true;
            }
            return false;
        }

    private:
        void abortAndJoin()
        {
            if (m_abortRequested.load())
                return; // skip!
            m_abortRequested.store(true);
            if (m_thread.joinable())
                m_thread.join();
        }

        void mainWrapper(void)
        {
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
                    // Some more specific
                    break;
                }
                catch (...)
                {
                    // Make sure that nothing leaves the thread for now...
                    break;
                }
            }
            m_running.store(false);
        }

    protected:
        auto getThreadWrapper(void)
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
        int m_interval;
    }; //# SimpleThread

} //> namespace util

#endif //> FG_INC_UTIL_SIMPLE_THREAD
