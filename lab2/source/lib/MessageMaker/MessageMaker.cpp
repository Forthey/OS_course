#include "MessageMaker.h"

#include <google/protobuf/timestamp.pb.h>

namespace {
google::protobuf::Timestamp makeNowTimestamp() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto secs = time_point_cast<seconds>(now);
    const auto nanos = duration_cast<nanoseconds>(now - secs);

    google::protobuf::Timestamp ts;
    ts.set_seconds(secs.time_since_epoch().count());
    ts.set_nanos(static_cast<int32_t>(nanos.count()));
    return ts;
}

chat::Message makeBaseMessage(chat::MessageType type) {
    chat::Message msg;

    msg.set_type(type);
    *msg.mutable_timestamp() = makeNowTimestamp();
    return msg;
}
}

chat::Message MessageMaker::makeBroadcast(std::uint64_t fromId, const std::string& text) {
    auto msg = makeBaseMessage(chat::MESSAGE_TYPE_CHAT_BROADCAST);

    auto* payload = msg.mutable_chat_broadcast();
    payload->set_from_id(fromId);
    payload->set_message(text);

    return msg;
}

chat::Message MessageMaker::makePrivate(std::uint64_t fromId, uint64_t toId, const std::string& text) {
    auto msg = makeBaseMessage(chat::MESSAGE_TYPE_CHAT_PRIVATE);

    auto* payload = msg.mutable_chat_private();
    payload->set_from_id(fromId);
    payload->set_to_id(toId);
    payload->set_message(text);

    return msg;
}

chat::Message MessageMaker::makeSystemJoin(std::uint64_t clientId) {
    auto msg = makeBaseMessage(chat::MESSAGE_TYPE_SYSTEM_JOIN);

    auto* payload = msg.mutable_system_join();
    payload->set_client_id(clientId);

    return msg;
}

chat::Message MessageMaker::makeSystemLeave(std::uint64_t clientId) {
    auto msg = makeBaseMessage(chat::MESSAGE_TYPE_SYSTEM_LEAVE);

    auto* payload = msg.mutable_system_leave();
    payload->set_client_id(clientId);

    return msg;
}

chat::Message MessageMaker::makeSystemKillNotice(std::uint64_t clientId) {
    auto msg = makeBaseMessage(chat::MESSAGE_TYPE_SYSTEM_KILL_NOTICE);

    auto* payload = msg.mutable_system_kill_notice();
    payload->set_client_id(clientId);

    return msg;
}
