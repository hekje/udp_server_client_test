/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#pragma once

#include <queue>
#include <mutex>
#include <optional>

namespace hek {

template <typename T>
class ProtectedQueue {
public:
    ProtectedQueue() = default;
    virtual ~ProtectedQueue() = default;

    ProtectedQueue(const ProtectedQueue &other) {
        std::lock_guard<std::mutex> guard(other.m_mutex);
        m_queue = other.m_queue;
    }

    ProtectedQueue &operator=(ProtectedQueue &other) {
        if (&other == this) {
            return *this;
        }

        std::unique_lock<std::mutex> lock1(m_mutex, std::defer_lock);
        std::unique_lock<std::mutex> lock2(other.m_mutex, std::defer_lock);
        std::lock(lock1, lock2);
        m_queue = other.m_queue;

        return *this;
    }

    bool Pop(T &outItem) {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_queue.empty()) {
            outItem = m_queue.front();
            m_queue.pop();
            return true;
        }
        return false;
    }

    bool Pop() {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_queue.empty()) {
            m_queue.pop();
            return true;
        }
        return false;
    }

    bool Front(T &outItem) {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_queue.empty()) {
            outItem = m_queue.front();
            return true;
        }
        return false;
    }

    std::optional<std::reference_wrapper<T>> Front() {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_queue.empty()) {
            return m_queue.front();
        }
        return std::nullopt;
    }


    void Push(const T &inItem) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_queue.push(inItem);
    }

    void Push(T &&inItem) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_queue.push(std::move(inItem));
    }

    bool Empty() {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_queue.empty();
    }

    int Size() {
        std::lock_guard<std::mutex> guard(m_mutex);
        return static_cast<int>(m_queue.size());
    }

    void Clear() {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::queue<T> emptyQueue;
        std::swap(m_queue, emptyQueue);
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

} // namespace hek
