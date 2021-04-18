#ifndef AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <vector>
#include <string>
#include <deque>


#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>

#include <sys/epoll.h>

#include <spdlog/logger.h>

#include <afina/Storage.h>
#include <afina/execute/Command.h>
#include <afina/logging/Service.h>
#include <afina/concurrency/Executor.h>

#include "protocol/Parser.h"

namespace Afina {
namespace Network {
namespace MTnonblock {
class Connection {
public:
    Connection(int s, std::shared_ptr<Afina::Storage> &pStorage, std::shared_ptr<spdlog::logger> &logger) :
        _socket(s), _pStorage(pStorage), _logger(logger)
    {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    inline bool isAlive() const { return flag_run; }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class ServerImpl;
    friend class Worker;

    int _socket;

    std::shared_ptr<Afina::Storage> _pStorage;
    std::shared_ptr<spdlog::logger> _logger;

    struct epoll_event _event;
    std::mutex _m;
    bool connection_close = false;
    std::atomic_bool flag_run;
    char client_buffer[4096] = "";
    std::size_t pos = 0;
    std::deque<std::string> output_buffer;
    std::size_t limit = 100;
    std::size_t eps = 20;
    std::size_t output_offset = 0;
};

} // namespace MTnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
