/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/

#include "udp_socket.hpp"
#include "sl_log.hpp"
#include <arpa/inet.h>
#include <functional>
#include <fcntl.h>
#include <ifaddrs.h>


namespace hek {

UdpSocket::UdpSocket(size_t bufferSize)
    : m_running(false), m_socketFd(-1), m_receiveBuffer(bufferSize, 0), m_bufferSize(bufferSize) {
    SLLog::LogInfo("UdpSocket::UdpSocket - Constructed");
}

UdpSocket::~UdpSocket() {
    StopReading();
    if (m_socketFd != -1) {
        close(m_socketFd);
    }
    SLLog::LogInfo("UdpSocket::~UdpSocket - Destructed#include <ifaddrs.h>");
}

int UdpSocket::Init(uint16_t port, const std::string& ipAddress) {

    m_socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socketFd == -1) {
        SLLog::LogError("Failed to create UDP socket: " + std::string(strerror(errno)));
        return -1;
    }

    m_socketAddress = {};
    m_socketAddress.sin_family = AF_INET;

    if (ipAddress.empty()) {
        // Configure SERVER Socket
        m_socketAddress.sin_addr.s_addr = INADDR_ANY;
    } else {
        // Configure CLIENT Socket: Set server IP address as destionation
        m_socketAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    }

    m_socketAddress.sin_port = htons(port);

    // "bind" is only required for SERVER Sockets
    if (ipAddress.empty()) {
        // SERVER Socket
        if (bind(m_socketFd, reinterpret_cast<struct sockaddr*>(&m_socketAddress), sizeof(m_socketAddress)) == -1) {
            SLLog::LogError("Failed to bind UDP socket: " + std::string(strerror(errno)));
            close(m_socketFd);
            m_socketFd = -1;
            return -1;
        }
        SLLog::LogInfo("UdpSocket::Init - Successfully initialized SERVER Socket - bound to IP: " +
                       (ipAddress.empty() ? "INADDR_ANY" : ipAddress) + " and port: " + std::to_string(port));
    } else {
        // CLIENT Socket
        SLLog::LogInfo("UdpSocket::Init - Successfully initialized CLIENT Socket - port: " + std::to_string(port));
    }

    return 0;
}

bool UdpSocket::IsInitialized() const {
    return m_socketFd != -1;
}

void UdpSocket::StartReading() {
    if (m_running.load()) {
        return;
    }

    m_running.store(true);
    m_receiverThread = std::thread(&UdpSocket::ReceiverThreadFunc, this);
}

void UdpSocket::StopReading() {
    if (!m_running.load()) {
        return;
    }

    m_running.store(false);
    if (m_receiverThread.joinable()) {
        m_receiverThread.join();
    }
}

void UdpSocket::RegisterObserver(IUdpObserver* observer) {
    std::lock_guard<std::mutex> lock(m_observerMutex);
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end()) {
        m_observers.push_back(observer);
    }
}

void UdpSocket::UnregisterObserver(IUdpObserver* observer) {
    std::lock_guard<std::mutex> lock(m_observerMutex);
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

int UdpSocket::WriteData(const std::string& data, const sockaddr_in& destination) {
    if (m_socketFd == -1) {
        return -1;
    }

    ssize_t bytesSent = sendto(m_socketFd, data.data(), data.size(), 0,
                               reinterpret_cast<const struct sockaddr*>(&destination),
                               sizeof(destination));
    if (bytesSent < 0) {
        SLLog::LogError("UdpSocket::WriteData - sendto() failed: " + std::string(strerror(errno)));
        return -1;
    }

    return static_cast<int>(bytesSent);
}

int UdpSocket::WriteData(const std::string& data) {
    return WriteData(data, m_socketAddress);
}

void UdpSocket::ReceiverThreadFunc() {
    fd_set readfds;
    sockaddr_in senderAddr = {};
    socklen_t senderAddrLen = sizeof(senderAddr);

    while (m_running.load()) {
        FD_ZERO(&readfds);
        FD_SET(m_socketFd, &readfds);

        struct timeval timeout = {1, 0}; // 1-second timeout

        int retval = select(m_socketFd + 1, &readfds, nullptr, nullptr, &timeout);
        if (retval > 0) {
            ssize_t bytesReceived = recvfrom(m_socketFd, m_receiveBuffer.data(), m_bufferSize, 0,
                                             reinterpret_cast<struct sockaddr*>(&senderAddr),
                                             &senderAddrLen);
            if (bytesReceived > 0) {
                std::string data(m_receiveBuffer.begin(), m_receiveBuffer.begin() + bytesReceived);
                //! Enable for debugging purposes
                //! SLLog::LogWarn("Received Data: " + data);
                NotifyObservers(data, senderAddr);
            } else if (bytesReceived < 0) {
                SLLog::LogError("recvfrom() failed: " + std::string(strerror(errno)));
            }
        } else if (retval == 0) {
            //! Enable for debugging pusposes
            //! SLLog::LogInfo("Timeout: No data received.");
        } else {
            SLLog::LogError("select() error: " + std::string(strerror(errno)));
        }
    }
}

void UdpSocket::NotifyObservers(const std::string& data, const sockaddr_in& senderAddr) {
    std::lock_guard<std::mutex> lock(m_observerMutex);
    for (IUdpObserver* observer : m_observers) {
        if (observer) {
            observer->NewUdpDataCallback(data, senderAddr);
        }
    }
}

} // namespace hek
