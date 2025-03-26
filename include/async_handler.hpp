/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#pragma once

#include "protected_queue.hpp"
#include "sl_log.hpp"
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>


namespace hek {

template <typename T>
class AsyncHandler {
public:
    AsyncHandler() :
        m_handleThreadRunning{false} {
        StartHandleThread();
        SLLog::LogInfo("AsyncHandler::AsyncHandler - Parent constructed");
    }

    ~AsyncHandler() {
        StopHandleThread();
        SLLog::LogInfo("AsyncHandler::~AsyncHandler - AsyncHandler terminated gracefully");
    }

    /**
     * This is not a pure virtual function, as this function is called in the running thread:
     * In case the child is already destructed, but the AsyncHandler parent is still alive,
     * the application will crash with a "pure virtual method called" exception.
     */
    virtual void HandleTriggerAction(T& t) { (void)t; }

protected:
    void TriggerHandlerThread(const T& triggerActionStruct) {
        if (!m_handleThreadRunning) {
            return;
        }

        {
            std::unique_lock<std::mutex> lock(m_handleThreadMutex);
            m_triggerActionQueue.Push(triggerActionStruct);
        }

        m_handleThreadCondVar.notify_all();
    }

    void Stop() {
        SLLog::LogInfo("AsyncHandler::Stop - StopHandleThread");
        StopHandleThread();
    }

private:
    void StartHandleThread() {
        StopHandleThread();

        {
            std::unique_lock<std::mutex> lock(m_handleThreadMutex);
            m_handleThreadRunning = true;
        }

        m_handleThread = std::thread(&AsyncHandler::RunHandleThread, this);
    }

    void StopHandleThread() {
        {
            std::unique_lock<std::mutex> triggerHandlerThreadlock(m_handleThreadMutex);

            m_triggerActionQueue.Clear();

            if (!m_handleThreadRunning) {
                return;
            }

            m_handleThreadRunning = false;
        }

        m_handleThreadCondVar.notify_all();

        if (m_handleThread.joinable()) {
            m_handleThread.join();
        }
    }

    void RunHandleThread() {
        SLLog::LogInfo("AsyncHandler::RunHandleThread - Handle thread started");

        while (m_handleThreadRunning) {
            {
                std::unique_lock<std::mutex> lock(m_handleThreadMutex);
                m_handleThreadCondVar.wait(lock, [this] {
                    return !m_triggerActionQueue.Empty() || !m_handleThreadRunning;
                });
            }

            if (!m_handleThreadRunning) {
                SLLog::LogInfo("AsyncHandler::RunHandleThread - Handle thread terminated gracefully");
                return;
            }

            while (!m_triggerActionQueue.Empty()) {
                T triggerActionStruct = {};
                if (m_triggerActionQueue.Pop(triggerActionStruct)) {
                    HandleTriggerAction(triggerActionStruct);
                } else {
                    SLLog::LogError("AsyncHandler::RunHandleThread - ERROR! Failed to pop an action request from the queue");
                }
            }
        }

        SLLog::LogInfo("AsyncHandler::RunHandleThread - Handle thread terminated gracefully");
    }

private:
    ProtectedQueue<T> m_triggerActionQueue;
    std::thread m_handleThread;
    std::atomic<bool> m_handleThreadRunning;
    std::condition_variable m_handleThreadCondVar;
    std::mutex m_handleThreadMutex;
};

} // namespace hek
