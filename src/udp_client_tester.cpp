/*****************************************************************************
*
* Copyright 2025 Dirk van Hek
*
*****************************************************************************/
#include "udp_client_tester.hpp"
#include "sl_log.hpp"
#include <arpa/inet.h>

namespace hek {

static constexpr uint32_t TIMEOUT_MS = 1024;

UdpClientTester::UdpClientTester(uint16_t port, const std::string& ipAddress) {
    hek::SLLog::LogInfo( "UdpClientTester::UdpClientTester - Enter constructor" );

    if (m_Socket.Init(port, ipAddress) != 0) {
        hek::SLLog::LogError("UdpClientTester::UdpClientTester - ERROR! Failed to initialize client socket for port " + std::to_string(port));
        return;
    } else {
        hek::SLLog::LogInfo("UdpClientTester::UdpClientTester - Successfully initialized client socket for port " + std::to_string(port));

        m_Socket.RegisterObserver(this);
        m_Socket.StartReading();

        m_timer.SetTimerCallback(std::bind(&UdpClientTester::TimerCallback, this));
        m_timer.ReqTimerStart(TIMEOUT_MS, true);
    }
}

UdpClientTester::~UdpClientTester() {
    SLLog::LogInfo( "UdpClientTester::~UdpClientTester - Enter destructor");

    // First stop the Parent
    hek::AsyncHandler< struct CallbackAction >::Stop();

    m_Socket.StopReading();

    // Stop the timer
    m_timer.ReqTimerStop();
    m_timer.UnsetTimerCallback();
}


void UdpClientTester::NewUdpDataCallback(const std::string& data, const sockaddr_in& senderAddr) {
    // Trigger this action to be handled on a different thread, so this callback can return immediately
    CallbackAction action;
    action.type = CallbackType::EUdpDataAvailable;
    action.data = data;
    action.senderAddr = senderAddr;
    TriggerHandlerThread( action );
}

void UdpClientTester::TimerCallback()
{
    // Trigger this action to be handled on a different thread, so this callback can return immediately
    CallbackAction action;
    action.type = CallbackType::ETimeoutCallback;
    TriggerHandlerThread(action);
}

void UdpClientTester::HandleTriggerAction( struct CallbackAction &action ) {
    switch ( action.type ) {
    case CallbackType::EUdpDataAvailable: {
        HandleUdpData(action.data, action.senderAddr);
        break;
    }
    case CallbackType::ETimeoutCallback: {
        //! Enable for debugging purposes
        //!SLLog::LogInfo( "UdpClientTester::HandleTriggerAction - Send messsage");
        m_Socket.WriteData("Ping!");
        break;
    }
    default:
        break;
    }
}

void UdpClientTester::HandleUdpData(const std::string& data, const sockaddr_in& senderAddr) {
    std::cout << "================================================================================" << std::endl;
    std::cout << "Received " << data.size() << " bytes from "
              << inet_ntoa(senderAddr.sin_addr) << ":" << ntohs(senderAddr.sin_port) << ": " << data << std::endl;
}


} // namespace hek
