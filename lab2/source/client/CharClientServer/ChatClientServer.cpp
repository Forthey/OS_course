#include "ChatClientServer.h"

#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>

#include "SignalHandler/SingleSignalHandler.h"
#include "Timer/Timer.h"
#include "Types/Console.h"

#ifdef CLIENT_CONN_MQ
#include "Conns/Conn_Mq.h"
#elifdef CLIENT_CONN_FIFO
#include "Conns/Conn_Fifo.h"
#elifdef CLIENT_CONN_SOCKET
#include "Conns/Conn_Sock.h"
#endif

#include "ChatClient/ChatClient.h"
#include "MessageMaker/MessageMaker.h"
#include "SignalHandler/SignalUtils.h"

class ChatClientServer::Impl {
    enum class ConnectionStatus { Handshake, Active };

public:
    explicit Impl(pid_t serverPid);

    void serve();

private:
    void tryRead();

    void onMessage(const chat::Message& msg);
    void onHandshakeConfirm(int id);
    void onHandshakeTimeout();
    void onSystemLeave();

    void sendPrivateMessage(std::uint64_t toId, std::string_view text) const;
    void sendBroadcastMessage(std::string_view text) const;
    void sendSystemLeaveMessage() const;

    std::uint64_t m_id{};
    std::atomic<bool> m_isRunning{false};
    std::atomic<ConnectionStatus> m_connectionStatus = ConnectionStatus::Handshake;
    pid_t m_serverPid;

    SingleSignalHandler m_singleSignalHandler;
    ChatClient m_chatClient;
    Timer m_timeoutTimer;
    std::unique_ptr<Conn> m_conn;

    std::jthread m_inputThread;
};

ChatClientServer::ChatClientServer(pid_t serverPid) : m_impl{std::make_unique<Impl>(serverPid)} {}

ChatClientServer::~ChatClientServer() = default;

void ChatClientServer::serve() { m_impl->serve(); }

ChatClientServer::Impl::Impl(pid_t serverPid)
    : m_serverPid{serverPid}
    , m_singleSignalHandler{SIGUSR2, [this](pid_t _, int id) { onHandshakeConfirm(id); }}
    , m_chatClient{
          [this](std::uint64_t toId, std::string_view text) { sendPrivateMessage(toId, text); },
          [this](std::string_view text) { sendBroadcastMessage(text); },
          [this] { onSystemLeave(); },
      }
    , m_timeoutTimer{std::chrono::seconds{5}, [this] { onHandshakeTimeout(); }} {
    m_timeoutTimer.start();
}

void ChatClientServer::Impl::serve() {
#ifdef CLIENT_CONN_MQ
    ClientConnType type{ClientConnType::MessageQueue};
#elifdef CLIENT_CONN_FIFO
    ClientConnType type{ClientConnType::Fifo};
#elifdef CLIENT_CONN_SOCKET
    ClientConnType type{ClientConnType::Socket};
#endif

    switch (type) {
        case ClientConnType::Fifo:
            consoleSrv().info("Connecting to server via FIFO...");
            break;
        case ClientConnType::MessageQueue:
            consoleSrv().info("Connecting to server via Message Queue...");
            break;
        case ClientConnType::Socket:
            consoleSrv().info("Connecting to server via Socket...");
            break;
    }
    SignalUtils::sendUsr1(m_serverPid, type);

    m_isRunning = true;

    while (m_isRunning) {
        switch (m_connectionStatus.load()) {
            case ConnectionStatus::Handshake:
                m_singleSignalHandler.poll();
                break;
            case ConnectionStatus::Active:
                tryRead();
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_chatClient.stopInteractiveWriting();
}

void ChatClientServer::Impl::tryRead() {
    if (const auto readResult = m_conn->read()) {
        chat::Message msg;
        if (!msg.ParseFromString(readResult.value())) {
            consoleSrv().system("Incorrect message received");
            return;
        }
        consoleSrv().system("Received new message");
        onMessage(msg);
    } else {
        switch (readResult.error()) {
            case ConnReadError::NoData:
                break;
            case ConnReadError::TryReadError:
            case ConnReadError::Unknown:
            case ConnReadError::Closed:
                m_isRunning = false;
                break;
        }
    }
}

void ChatClientServer::Impl::onMessage(const chat::Message& msg) {
    m_chatClient.onMessage(msg);

    if (msg.payload_case() == chat::Message::kSystemLeave && msg.system_leave().client_id() == 0) {
        consoleSrv().info("Server left, shutting down...");
        m_isRunning = false;
    }
}

void ChatClientServer::Impl::onHandshakeConfirm(int id) {
    m_timeoutTimer.stop();
    consoleSrv().info(std::format("Connection established, id = {}", id));
    m_id = id;
#ifdef CLIENT_CONN_MQ
    m_conn = std::make_unique<Conn_Mq>(false, id, std::format("{}_{}", MQ_TO_CLIENT_CHANNEL_BASE, id),
                                       std::format("{}_{}", MQ_TO_HOST_CHANNEL_BASE, id));
#elifdef CLIENT_CONN_FIFO
    m_conn = std::make_unique<Conn_Fifo>(false, id,
                                         std::format("{}/{}_{}", std::getenv("HOME"), FIFO_TO_CLIENT_CHANNEL_BASE, id),
                                         std::format("{}/{}_{}", std::getenv("HOME"), FIFO_TO_HOST_CHANNEL_BASE, id));
#elifdef CLIENT_CONN_SOCKET
    m_conn =
        std::make_unique<Conn_Sock>(false, id, std::format("{}/{}_{}", std::getenv("HOME"), SOCKET_CHANNEL_BASE, id));
#endif

    m_connectionStatus = ConnectionStatus::Active;
    m_inputThread = std::jthread([this]() { m_chatClient.runInteractiveWriting(); });
}

void ChatClientServer::Impl::onHandshakeTimeout() {
    consoleSrv().info("Handshake timeout, exiting");
    m_isRunning = false;
}

void ChatClientServer::Impl::onSystemLeave() {
    sendSystemLeaveMessage();
    m_isRunning = false;
}

void ChatClientServer::Impl::sendPrivateMessage(std::uint64_t toId, std::string_view text) const {
    const auto msg = MessageMaker::makePrivate(m_id, toId, text);
    m_chatClient.printMyPrivateMessage(msg);
    m_conn->write(msg.SerializeAsString());
}

void ChatClientServer::Impl::sendBroadcastMessage(std::string_view text) const {
    const auto msg = MessageMaker::makeBroadcast(m_id, text);
    m_chatClient.printMyBroadcastMessage(msg);
    m_conn->write(msg.SerializeAsString());
}

void ChatClientServer::Impl::sendSystemLeaveMessage() const {
    const auto msg = MessageMaker::makeSystemLeave(m_id);
    m_conn->write(msg.SerializeAsString());
}
