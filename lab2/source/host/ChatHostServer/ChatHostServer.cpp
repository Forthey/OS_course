#include "ChatHostServer.h"

#include <messages.pb.h>

#include <csignal>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

#include "ChatClient/ChatClient.h"
#include "ClientConn.h"
#include "MessageMaker/MessageMaker.h"
#include "SignalHandler/MultiSignalHandler.h"
#include "SignalHandler/SignalUtils.h"
#include "Types/Conn.h"
#include "Types/Console.h"

class ChatHostServer::Impl {
    using ClientConnPtr = std::shared_ptr<ClientConn>;

public:
    Impl();

    void serve();

private:
    void runReadLoop();
    void runHealthCheckLoop();

    void onHandshakeBegin(pid_t clientPid, int code);
    void onMessage(const chat::Message& msg);
    void onSystemLeave();
    void onClientInactive(std::uint64_t clientId);

    void sendPrivateMessage(std::uint64_t toId, std::string_view text);
    void sendBroadcastMessage(std::string_view text);
    void sendSystemJoin(std::uint64_t clientId);
    void sendSystemLeave(std::uint64_t clientId);
    void sendSystemKillNotice(std::uint64_t clientId);

    void broadcastAnyMessage(const chat::Message& msg, std::uint64_t ignoreId);
    void proxyPrivate(const chat::Message& msg);

    void deleteClient(std::uint64_t clientId);
    void pruneDeletedClients();

    std::atomic<bool> m_isRunning{false};

    std::atomic<std::uint64_t> m_lastId{0};

    std::shared_mutex m_clientsMtx;
    std::unordered_map<std::uint64_t, ClientConnPtr> m_clients;

    std::shared_mutex m_clientsToDeleteMtx;
    std::vector<std::uint64_t> m_clientsToDelete;

    MultiSignalHandler m_multiSignalHandler;
    ChatClient m_chatClient;
};

ChatHostServer::ChatHostServer() : m_impl{std::make_unique<Impl>()} {}

ChatHostServer::~ChatHostServer() = default;

void ChatHostServer::serve() { m_impl->serve(); }

ChatHostServer::Impl::Impl()
    : m_multiSignalHandler{SIGUSR1, [this](pid_t clientPid, int code) { onHandshakeBegin(clientPid, code); }}
    , m_chatClient{
          [this](std::uint64_t toId, std::string_view text) { sendPrivateMessage(toId, text); },
          [this](std::string_view text) { sendBroadcastMessage(text); },
          [this] { onSystemLeave(); },
      } {}

void ChatHostServer::Impl::serve() {
    m_isRunning = true;

    std::jthread inputThread{[this] { m_chatClient.runInteractiveWriting(); }};
    std::jthread readThread{[this] { runReadLoop(); }};
    std::jthread healthCheckThread{[this] { runHealthCheckLoop(); }};
}

void ChatHostServer::Impl::runReadLoop() {
    while (m_isRunning) {
        m_multiSignalHandler.poll();
        {
            std::shared_lock lock{m_clientsMtx};
            for (const auto& clientData : m_clients | std::views::values) {
                if (const auto readResult = clientData->conn->read()) {
                    chat::Message msg;
                    if (!msg.ParseFromString(readResult.value())) {
                        consoleSrv().system("Incorrect message received");
                    }
                    clientData->inactiveTimer.reset();
                    onMessage(msg);
                } else {
                    switch (readResult.error()) {
                        case ConnReadError::NoData:
                            break;
                        case ConnReadError::TryReadError:
                        case ConnReadError::Unknown:
                        case ConnReadError::Closed:
                            deleteClient(clientData->id);
                            break;
                    }
                }
            }
        }
        pruneDeletedClients();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ChatHostServer::Impl::runHealthCheckLoop() {
    while (m_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ChatHostServer::Impl::onHandshakeBegin(pid_t clientPid, int code) {
    consoleSrv().system(std::format("Initial handshake with {}, connection type {}", clientPid, code));
    m_lastId = m_lastId + 1;

    if (code < 1 || code > 3) {
        // Клиент умрет по таймауту
        return;
    }

    {
        std::unique_lock lock{m_clientsMtx};

        std::uint64_t id = m_lastId;
        ClientConnPtr conn{new ClientConn{
            id,
            clientPid,
            static_cast<ClientConnType>(code),
            [this](const std::uint64_t id) { onClientInactive(id); },
        }};

        m_clients.emplace(id, std::move(conn));
    }

    SignalUtils::sendUsr2(clientPid, m_lastId);
    consoleSrv().system("Initial handshake completed");
    sendSystemJoin(m_lastId);
}

void ChatHostServer::Impl::onMessage(const chat::Message& msg) {
    bool shouldBeClientProcessed = true;

    switch (msg.payload_case()) {
        case chat::Message::kChatBroadcast:
            broadcastAnyMessage(msg, msg.chat_broadcast().from_id());
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
            deleteClient(msg.system_leave().client_id());
            break;
        default:
            // Клиент не может присылать остальные как серверу
            break;
    }

    if (shouldBeClientProcessed) {
        m_chatClient.onMessage(msg);
    }
}

void ChatHostServer::Impl::onSystemLeave() {
    consoleSrv().info(std::format("Exit requested, shutting down..."));
    m_chatClient.stopInteractiveWriting();
    m_isRunning = false;
}

void ChatHostServer::Impl::onClientInactive(std::uint64_t clientId) {
    consoleSrv().system(std::format("Inactive client {}", clientId));
    sendSystemKillNotice(clientId);
    deleteClient(clientId);
}

void ChatHostServer::Impl::sendPrivateMessage(std::uint64_t toId, std::string_view text) {
    const auto msg = MessageMaker::makePrivate(0, toId, text);
    m_chatClient.printMyPrivateMessage(msg);
    std::shared_lock lock{m_clientsMtx};
    if (!m_clients.contains(toId)) {
        // Нет клиента с нужным id, просто игнор
        consoleSrv().system(std::format("Received private message to wrong id {}, ignoring", toId));
        return;
    }
    m_clients.at(toId)->conn->write(msg.SerializeAsString());
}

void ChatHostServer::Impl::sendBroadcastMessage(std::string_view text) {
    const auto msg = MessageMaker::makeBroadcast(0, text);
    m_chatClient.printMyBroadcastMessage(msg);
    broadcastAnyMessage(msg, 0);
}

void ChatHostServer::Impl::sendSystemJoin(std::uint64_t clientId) {
    const auto msg = MessageMaker::makeSystemJoin(clientId);
    m_chatClient.onMessage(msg);
    broadcastAnyMessage(msg, clientId);
}

void ChatHostServer::Impl::sendSystemLeave(std::uint64_t clientId) {
    const auto msg = MessageMaker::makeSystemLeave(clientId);
    consoleSrv().info("hii");
    m_chatClient.onMessage(msg);
    broadcastAnyMessage(msg, clientId);
}

void ChatHostServer::Impl::sendSystemKillNotice(std::uint64_t clientId) {
    const auto msg = MessageMaker::makeSystemKillNotice(clientId);
    m_chatClient.onMessage(msg);
    broadcastAnyMessage(msg, clientId);
}

void ChatHostServer::Impl::broadcastAnyMessage(const chat::Message& msg, std::uint64_t ignoreId) {
    consoleSrv().system(std::format("Broadcasting message with ignore id {}...", ignoreId));
    const auto buffer = msg.SerializeAsString();

    std::shared_lock lock{m_clientsMtx};
    for (auto& clientData : m_clients | std::views::values) {
        if (clientData->id == ignoreId) {
            continue;
        }
        consoleSrv().system(std::format("Broadcasting message to {}...", clientData->id));
        clientData->conn->write(buffer);
    }
}

void ChatHostServer::Impl::proxyPrivate(const chat::Message& msg) {
    consoleSrv().system(
        std::format("Sending private message {} --> {}", msg.chat_private().from_id(), msg.chat_private().to_id()));
    std::shared_lock lock{m_clientsMtx};
    if (!m_clients.contains(msg.chat_private().to_id())) {
        // Нет клиента с нужным id, просто игнор
        consoleSrv().system(
            std::format("Received private message to wrong id {}, ignoring", msg.chat_private().to_id()));
        return;
    }
    m_clients.at(msg.chat_private().to_id())->conn->write(msg.SerializeAsString());
}

void ChatHostServer::Impl::deleteClient(std::uint64_t clientId) {
    {
        std::shared_lock lock{m_clientsMtx};
        if (!m_clients.contains(clientId)) {
            consoleSrv().system(std::format("Trying to delete client {}, which does not exist", clientId));
            return;
        }
        if (kill(m_clients.at(clientId)->pid, SIGKILL) == -1) {
            consoleSrv().system(std::format("Failed to send SIGKILL to client {}: {}", clientId,
                                            std::system_category().message(errno)));
        }
    }
    std::unique_lock lock{m_clientsToDeleteMtx};
    m_clientsToDelete.emplace_back(clientId);
}

void ChatHostServer::Impl::pruneDeletedClients() {
    std::unique_lock lock{m_clientsToDeleteMtx};
    for (const auto& id : m_clientsToDelete) {
        sendSystemLeave(id);
    }

    std::unique_lock lock2{m_clientsMtx};
    for (auto& id : m_clientsToDelete) {
        m_clients.erase(id);
    }
    m_clientsToDelete.clear();
}
