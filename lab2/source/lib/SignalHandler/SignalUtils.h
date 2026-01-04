#pragma once
#include "Types.h"

class ISignalHandler;

namespace SignalUtils {

void installHandler(int signo, ISignalHandler* handler);

void uninstallHandler(int signo, ISignalHandler* handler);

void sendUsr1(pid_t pid, ClientConnType connType);

void sendUsr2(pid_t pid, std::uint64_t clientId);
}  // namespace SignalUtils
