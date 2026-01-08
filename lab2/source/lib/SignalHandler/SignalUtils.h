#pragma once
#include "Conn.h"
#include "SignalHandler.h"

namespace SignalUtils {
void installHandler(int signo, SignalHandler* handler);

void uninstallHandler(int signo, SignalHandler* handler);

void sendUsr1(pid_t pid, ClientConnType connType);

void sendUsr2(pid_t pid, std::uint64_t clientId);
}  // namespace SignalUtils
