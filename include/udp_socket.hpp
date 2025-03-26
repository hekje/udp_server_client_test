/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>


namespace hek {

class IUdpObserver {
public:
    virtual ~IUdpObserver() = default;
    virtual void NewUdpDataCallback(const std::string& data, const sockaddr_in& senderAddr) = 0;
};

class UdpSocket {
public:
    explicit UdpSocket(size_t bufferSize = 1024);
    ~UdpSocket();

    int Init(uint16_t port, const std::string& ipAddress = ""); // Leave ipAddress empty for Server Socket
    bool IsInitialized() const;

    void StartReading();
    void StopReading();

    void RegisterObserver(IUdpObserver* observer);
    void UnregisterObserver(IUdpObserver* observer);

    int WriteData(const std::string& data, const sockaddr_in& destination);
    int WriteData(const std::string& data);

private:
    void ReceiverThreadFunc();
    void NotifyObservers(const std::string& data, const sockaddr_in& senderAddr);

    std::thread m_receiverThread;
    std::atomic<bool> m_running;
    std::mutex m_observerMutex;
    std::vector<IUdpObserver*> m_observers;

    int m_socketFd;
    sockaddr_in m_socketAddress;
    std::vector<uint8_t> m_receiveBuffer;
    size_t m_bufferSize;
};

} // namespace hek
