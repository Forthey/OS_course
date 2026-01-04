#include "ChatServer.h"

#include <messages.pb.h>

#include <csignal>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

#include "ChatClient/ChatClient.h"
#include "ClientConn.h"
#include "Conns/Conn.h"
#include "SignalHandler/MultiSignalHandler.h"
#include "SignalHandler/SignalUtils.h"

class ChatServer::Impl : public ChatClient {
public:
    Impl();

    void serve();

private:
    void runReadLoop() override;
    void runHealthCheckLoop();

    void onHandshakeBegin(pid_t clientPid, int code);

    void onMessage(const chat::Message& msg) override;
    void broadcastAnyMessage(const chat::Message& msg, std::uint64_t ignoreId);
    void proxyPrivate(const chat::Message& msg);

    std::shared_mutex m_clientsMtx;
    std::unordered_map<std::uint64_t, std::unique_ptr<ClientConn>> m_clients;
    std::vector<std::uint64_t> m_clientsToDelete;

    std::atomic<std::uint64_t> m_lastId{0};
};

ChatServer::ChatServer() : m_impl{std::make_unique<Impl>()} {}

ChatServer::~ChatServer() = default;

void ChatServer::serve() { m_impl->serve(); }

ChatServer::Impl::Impl()
    : ChatClient{std::make_unique<MultiSignalHandler>(
          SIGUSR1, [this](pid_t clientPid, int code) { onHandshakeBegin(clientPid, code); })} {}

void ChatServer::Impl::serve() {
    m_isRunning = true;

    std::jthread clientThread{[this] { runInteractiveWriting(); }};
    std::jthread readThread{[this] { runReadLoop(); }};
    std::jthread healthCheckThread{[this] { runHealthCheckLoop(); }};
}

void ChatServer::Impl::runReadLoop() {
    while (m_isRunning) {
        {
            std::shared_lock lock{m_clientsMtx};
            for (auto& clientData : m_clients | std::views::values) {
                if (const auto readResult = clientData->conn->read()) {
                    chat::Message msg;
                    if (!msg.ParseFromString(readResult.value())) {
                        std::cout << "Incorrect message received" << std::endl;
                    }
                    onMessage(msg);
                } else {
                    switch (readResult.error()) {
                        case ConnReadError::NoData:
                            break;
                        case ConnReadError::TryReadError:
                        case ConnReadError::Unknown:
                        case ConnReadError::Closed:
                            m_clientsToDelete.emplace_back(clientData->id);
                            break;
                    }
                }
            }
        }
        {
            std::unique_lock lock{m_clientsMtx};
            for (auto& id : m_clientsToDelete) {
                m_clients.erase(id);
            }
        }
        m_clientsToDelete.clear();

        doPollStep();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ChatServer::Impl::runHealthCheckLoop() {
    while (m_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ChatServer::Impl::onHandshakeBegin(pid_t clientPid, int code) {
    std::cout << "Initial handshake with " << clientPid << ", connection type " << code << std::endl;
    m_lastId = m_lastId + 1;

    if (code < 1 || code > 3) {
        // Клиент умрет по таймауту
        return;
    }

    {
        std::unique_lock lock{m_clientsMtx};
        m_clients.emplace(m_lastId, std::make_unique<ClientConn>(m_lastId, clientPid, static_cast<ClientConnType>(code)));
    }

    SignalUtils::sendUsr2(clientPid, m_lastId);
    std::cout << "Initial handshake completed" << std::endl;
}

void ChatServer::Impl::onMessage(const chat::Message& msg) {
    bool shouldBeClientProcessed = true;

    switch (msg.payload_case()) {
        case chat::Message::kChatBroadcast:
            broadcastAnyMessage(msg, msg.system_leave().client_id());
            break;
        case chat::Message::kChatPrivate:
            if (msg.chat_private().to_id() == 0ull) {
                break;
            }
            proxyPrivate(msg);
            shouldBeClientProcessed = false;
            break;
        case chat::Message::kSystemLeave:
            broadcastAnyMessage(msg, msg.system_leave().client_id());
            m_clientsToDelete.emplace_back(msg.system_leave().client_id());
            break;
        default:
            // Клиент не может присылать остальные как серверу
            break;
    }

    if (shouldBeClientProcessed) {
        ChatClient::onMessage(msg);
    }
}

void ChatServer::Impl::broadcastAnyMessage(const chat::Message& msg, std::uint64_t ignoreId) {
    const auto buffer = msg.SerializeAsString();

    std::shared_lock lock{m_clientsMtx};
    for (auto& clientData : m_clients | std::views::values) {
        if (clientData->id == ignoreId) {
            continue;
        }
        clientData->conn->write(buffer);
    }
}

void ChatServer::Impl::proxyPrivate(const chat::Message& msg) {
    std::shared_lock lock{m_clientsMtx};
    if (!m_clients.contains(msg.chat_private().to_id())) {
        // Нет клиента с нужным id, просто игнор
        return;
    }
    m_clients.at(msg.chat_private().to_id())->conn->write(msg.SerializeAsString());
}
