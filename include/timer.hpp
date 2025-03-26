/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#pragma once

#include "protected_queue.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>


namespace hek {

using TimeoutCallback_t = std::function<void(void)>;

struct TimerParams_t {
    uint32_t timeoutMs = 0;
    bool repeat = false;
    bool enabled = false;
};

class Timer {
public:
    Timer();
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void SetTimerCallback(TimeoutCallback_t inCallback);
    void UnsetTimerCallback();

    void ReqTimerStart(uint32_t inTimeoutMs, bool inRepeat = false);
    void ReqTimerStop();

private:
    void Handler();

private:
    std::thread m_HandlerThread;
    std::atomic_bool m_HandlerRunning;

    std::mutex m_Mutex;
    std::condition_variable m_TimerCondition;
    ProtectedQueue<TimerParams_t> m_TimerQueue;

    std::mutex m_TimeoutCallbackMutex;
    TimeoutCallback_t m_pTimeoutCallback;
};

}  // namespace hek
