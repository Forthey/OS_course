#include <csignal>
#include <thread>

#include "ChatClient/ChatClient.h"
#include "Conns/Conn_Mq.h"
#include "Conns/Conn_Sock.h"
#ifdef CLIENT_CONN_MQ
#include "Conns/Conn_Mq.h"
#elifdef CLIENT_CONN_FIFO
#include "Conns/Conn_Fifo.h"
#elifdef CLIENT_CONN_SOCKET
#include "Conns/Conn_Sock.h"
#endif
#include "SignalHandler/SingleSignalHandler.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <server pid>" << std::endl;
        return 1;
    }
    int serverPid = std::stoi(argv[1]);

#ifdef CLIENT_CONN_MQ
    constexpr auto connType = ClientConnType::MessageQueue;
#elifdef CLIENT_CONN_FIFO
    constexpr auto connType = ClientConnType::Fifo;
#elifdef CLIENT_CONN_SOCKET
    constexpr auto connType = ClientConnType::Socket;
#endif

    ChatClient client{
        std::make_unique<SingleSignalHandler>(SIGUSR2,
                                              [&client](pid_t pid, int id) {
                                                  std::unique_ptr<Conn> conn;
#ifdef CLIENT_CONN_MQ
                                                  conn = std::make_unique<Conn_Mq>(
                                                      false, id, std::format("{}_{}", MQ_TO_CLIENT_CHANNEL_BASE, id),
                                                      std::format("{}_{}", MQ_TO_HOST_CHANNEL_BASE, id));
#elifdef CLIENT_CONN_FIFO
                conn = std::make_unique<Conn_Fifo>(false, id, std::format("{}_{}", FIFO_TO_CLIENT_CHANNEL_BASE, id),
                                                   std::format("{}_{}", FIFO_TO_HOST_CHANNEL_BASE, id));
#elifdef CLIENT_CONN_SOCKET
                conn = std::make_unique<Conn_Sock>(false, id, std::format("{}_{}", SOCKET_CHANNEL_BASE, id));
#endif

                                                  client.setConnAndId(std::move(conn), id);
                                              }),
        serverPid, connType};

    const auto begin = std::chrono::steady_clock::now();
    auto now = begin;
    while (!client.isRunning() && now - begin < std::chrono::seconds{5}) {
        client.doPollStep();
        now = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    if (!client.isRunning()) {
        std::cout << "Cannot connect to host" << std::endl;
        return 1;
    }

    std::jthread interactiveWritingThread{[&client] { client.runInteractiveWriting(); }};
    std::jthread mainReadLoopThread{[&client] { client.runReadLoop(); }};

    return 0;
}
