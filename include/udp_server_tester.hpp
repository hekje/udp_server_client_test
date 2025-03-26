/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/
#pragma once

#include "udp_socket.hpp"
#include "async_handler.hpp"
#include "timer.hpp"


namespace hek {

enum class CallbackType {
    EUndefined = 0,
    EUdpDataAvailable = 1
};

struct CallbackAction {
    CallbackType type = CallbackType::EUndefined;
    std::string data;
    sockaddr_in senderAddr;
};

class UdpServerTester : public hek::IUdpObserver, public hek::AsyncHandler< struct CallbackAction > {
public:
    UdpServerTester(uint16_t port, const std::string& ipAddress = "");
    ~UdpServerTester();

    void NewUdpDataCallback(const std::string& data, const sockaddr_in& senderAddr) override;
    void HandleTriggerAction( struct CallbackAction &action ) override;

private:
    void HandleUdpData(const std::string& data, const sockaddr_in& senderAddr);

private:
    UdpSocket m_Socket;
};

} // namespace hek

