/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/


#include "timer.hpp"
#include "sl_log.hpp"

namespace hek {

Timer::Timer()
{
    m_HandlerRunning.store(true);
    m_HandlerThread = std::thread(&Timer::Handler, this);
}

Timer::~Timer()
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_HandlerRunning.store(false);
    }

    m_TimerCondition.notify_all();

    if (m_HandlerThread.joinable()) m_HandlerThread.join();

    SLLog::LogInfo("Timer::~Timer - Timer terminated gracefully...");
}

void Timer::ReqTimerStart(uint32_t inTimeoutMs, bool inRepeat)
{
    TimerParams_t tp;
    tp.timeoutMs = inTimeoutMs;
    tp.repeat = inRepeat;
    tp.enabled = true;

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_TimerQueue.Clear();
        m_TimerQueue.Push(tp);
    }

    m_TimerCondition.notify_all();
}

void Timer::ReqTimerStop()
{
    TimerParams_t tp;
    tp.enabled = false;

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_TimerQueue.Clear();
        m_TimerQueue.Push(tp);
    }

    m_TimerCondition.notify_all();
}

void Timer::SetTimerCallback(TimeoutCallback_t inCallback)
{
    std::lock_guard<std::mutex> lock(m_TimeoutCallbackMutex);
    m_pTimeoutCallback = inCallback;
}

void Timer::UnsetTimerCallback()
{
    std::lock_guard<std::mutex> lock(m_TimeoutCallbackMutex);
    m_pTimeoutCallback = nullptr;
}

void Timer::Handler()
{
    std::chrono::steady_clock::time_point tpBegin = std::chrono::steady_clock::now();
    TimerParams_t tp;
    tp.enabled = false;

    while (m_HandlerRunning.load()) {
        std::unique_lock<std::mutex> lock(m_Mutex);

        if (!tp.enabled) {
            m_TimerCondition.wait(lock, [this] { return (!m_HandlerRunning.load() || !m_TimerQueue.Empty()); });
        } else {
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tpBegin).count();
            uint32_t overtimeMs = (tp.timeoutMs > 0) ? (elapsedMs % tp.timeoutMs) : 0;
            uint32_t adjustedTimeoutMs = (overtimeMs > 0) ? (tp.timeoutMs - overtimeMs) : tp.timeoutMs;

            if (!m_TimerCondition.wait_for(lock, std::chrono::milliseconds(adjustedTimeoutMs),
                                           [this] { return (!m_HandlerRunning.load() || !m_TimerQueue.Empty()); })) {
                lock.unlock();
                {
                    std::lock_guard<std::mutex> lock(m_TimeoutCallbackMutex);
                    if (m_pTimeoutCallback) {
                        m_pTimeoutCallback();
                    }
                }
                lock.lock();

                if (!tp.repeat) {
                    tp.enabled = false;
                }
            }
        }

        if (!m_HandlerRunning.load()) {
            break;
        }

        if (m_TimerQueue.Empty()) {
            continue;
        }

        TimerParams_t tpNew;
        if (m_TimerQueue.Pop(tpNew)) {
            tp = tpNew;
            tpBegin = std::chrono::steady_clock::now();
        }
    }

    SLLog::LogInfo("Timer::Handler - Thread terminated gracefully...");
}

}  // namespace hek
