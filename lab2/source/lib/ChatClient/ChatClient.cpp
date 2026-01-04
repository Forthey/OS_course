#include "ChatClient.h"

#include <thread>

#include "MessageMaker/MessageMaker.h"
#include "SignalHandler/SignalUtils.h"

ChatClient::ChatClient(std::unique_ptr<ISignalHandler> sigHandler) : m_sigHandler{std::move(sigHandler)} {}

ChatClient::ChatClient(std::unique_ptr<ISignalHandler> sigHandler, pid_t serverPid, ClientConnType type)
    : m_serverPid{serverPid}
    , m_sigHandler{std::move(sigHandler)} {
    switch (type) {
        case ClientConnType::Fifo:
            std::cout << "Connecting to server via FIFO..." << std::endl;
            break;
        case ClientConnType::MessageQueue:
            std::cout << "Connecting to server via Message Queue..." << std::endl;
            break;
        case ClientConnType::Socket:
            std::cout << "Connecting to server via Socket..." << std::endl;
            break;
    }

    SignalUtils::sendUsr1(m_serverPid, type);
}

void ChatClient::setConnAndId(std::unique_ptr<Conn> conn, std::uint64_t id) {
    std::cout << "Connection established, id = " << id << std::endl;
    m_conn = std::move(conn);
    m_id = id;
    m_isRunning = true;
}

void ChatClient::doPollStep() {
    m_sigHandler->poll();
}

void ChatClient::runInteractiveWriting() {
    while (m_isRunning) {
        std::string text;
        std::cin >> text;
    }
}

void ChatClient::runReadLoop() {
    while (m_isRunning) {
        if (const auto readResult = m_conn->read()) {
            chat::Message msg;
            if (!msg.ParseFromString(readResult.value())) {
                continue;
            }
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

        sendBroadcastMessage("xdd");

        m_sigHandler->poll();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ChatClient::sendPrivateMessage(const std::string& text, std::uint64_t toId) {
    const auto msg = MessageMaker::makePrivate(m_id, toId, text);
    m_conn->write(msg.SerializeAsString());
}

void ChatClient::sendBroadcastMessage(const std::string& text) {
    const auto msg = MessageMaker::makeBroadcast(m_id, text);
    m_conn->write(msg.SerializeAsString());
}

void ChatClient::printPrivateMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                                     const std::string& msg) {
    std::cout << std::format("{:%d.%m.%Y %H:%M:%S} PRIVATE from {}: {}\n", timePoint, from, msg);
}

void ChatClient::printBroadcastMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                                       const std::string& msg) {
    std::cout << std::format("{:%d.%m.%Y %H:%M:%S} BROADCAST from {}: {}\n", timePoint, from, msg);
}

void ChatClient::onMessage(const chat::Message& msg) {
    const auto timePoint = std::chrono::system_clock::time_point{std::chrono::seconds{msg.timestamp().seconds()}};
    switch (msg.payload_case()) {
        case chat::Message::kChatBroadcast:
            printBroadcastMessage(timePoint, msg.chat_broadcast().from_id(), msg.chat_broadcast().message());
            break;
        case chat::Message::kChatPrivate:
            printPrivateMessage(timePoint, msg.chat_private().from_id(), msg.chat_private().message());
            break;
        case chat::Message::kSystemJoin:
            break;
        case chat::Message::kSystemLeave:
            break;
        case chat::Message::kSystemKillNotice:
            break;
        default:
            break;
    }
}
