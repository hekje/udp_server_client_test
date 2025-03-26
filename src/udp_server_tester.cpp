/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/
#include "udp_server_tester.hpp"
#include "sl_log.hpp"
#include <arpa/inet.h>

namespace hek {

UdpServerTester::UdpServerTester(uint16_t port, const std::string& ipAddress) {
    hek::SLLog::LogInfo( "UdpServerTester::UdpServerTester - Enter constructor" );

    if (m_Socket.Init(port, ipAddress) != 0) {
        hek::SLLog::LogError("UdpServerTester::UdpServerTester - ERROR! Failed to initialize server socket for port " + std::to_string(port));
        return;
    } else {
        hek::SLLog::LogInfo("UdpServerTester::UdpServerTester - Successfully initialized server socket for port " + std::to_string(port));

        m_Socket.RegisterObserver(this);
        m_Socket.StartReading();
    }
}

UdpServerTester::~UdpServerTester() {
    SLLog::LogInfo( "UdpServerTester::~UdpServerTester - Enter destructor");

    // First stop the Parent
    hek::AsyncHandler< struct CallbackAction >::Stop();

    m_Socket.StopReading();
}


void UdpServerTester::NewUdpDataCallback(const std::string& data, const sockaddr_in& senderAddr) {
    // Trigger this action to be handled on a different thread, so this callback can return immediately
    CallbackAction action;
    action.type = CallbackType::EUdpDataAvailable;
    action.data = data;
    action.senderAddr = senderAddr;
    TriggerHandlerThread( action );
}

void UdpServerTester::HandleTriggerAction( struct CallbackAction &action ) {
    switch ( action.type ) {
    case CallbackType::EUdpDataAvailable: {
        HandleUdpData(action.data, action.senderAddr);
        break;
    }
    default:
        break;
    }
}

void UdpServerTester::HandleUdpData(const std::string& data, const sockaddr_in& senderAddr) {
    std::cout << "================================================================================" << std::endl;
    std::cout << "Received " << data.size() << " bytes from "
              << inet_ntoa(senderAddr.sin_addr) << ":" << ntohs(senderAddr.sin_port) << ": " << data << std::endl;

    m_Socket.WriteData("Pong!", senderAddr);
}


} // namespace hek
