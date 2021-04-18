#include "Connection.h"

#include <afina/execute/Command.h>
#include "protocol/Parser.h"

#include <iostream>
#include <sys/uio.h>

namespace Afina {
namespace Network {
namespace MTnonblock {
// See Connection.h
// See Connection.h
void Connection::Start()
{
    flag_run.store(true, std::memory_order_relaxed);
    _event.events = EPOLLERR | EPOLLHUP | EPOLLIN | EPOLLRDHUP;
    output_offset = 0;
    _logger->debug("Start socket {}", _socket);
}

// See Connection.h
void Connection::OnError()
{
    flag_run.store(false,std::memory_order_relaxed);
    _logger->debug("Error on socket {}", _socket);
}

// See Connection.h
void Connection::OnClose()
{
    flag_run.store(false, std::memory_order_relaxed);
    _logger->debug("Close on socket {}", _socket);
}

// See Connection.h
void Connection::DoRead()
{
    std::atomic_thread_fence(std::memory_order_acquire);
    _logger->debug("Read on socket {}", _socket);
    try {
        int readed_bytes = -1;
        while ((readed_bytes = read(_socket, client_buffer + pos, sizeof(client_buffer) - pos)) > 0) {
            _logger->debug("Got {} bytes from socket", readed_bytes);
            pos += readed_bytes;
            while (pos > 0) {
                _logger->debug("Process {} bytes", pos);
                // There is no command yet
                if (!command_to_execute) {
                    std::size_t parsed = 0;
                    //std::cout<<client_buffer<<std::endl;
                    if (parser.Parse(client_buffer, pos, parsed)) {
                        //std::cout<<"parsed"<<std::endl;
                        _logger->debug("Found new command: {} in {} bytes", parser.Name(), parsed);
                        command_to_execute = parser.Build(arg_remains);
                        if (arg_remains > 0) {
                            arg_remains += 2;
                        }
                    }
                    if (parsed == 0) {
                        break;
                    } else {
                        std::memmove(client_buffer, client_buffer + parsed, pos - parsed);
                        pos -= parsed;
                    }
                }

                if (command_to_execute && arg_remains > 0) {
                    _logger->debug("Fill argument: {} bytes of {}", pos, arg_remains);
                    // There is some parsed command, and now we are reading argument
                    std::size_t to_read = std::min(arg_remains, std::size_t(pos));
                    argument_for_command.append(client_buffer, to_read);

                    std::memmove(client_buffer, client_buffer + to_read, pos - to_read);
                    arg_remains -= to_read;
                    pos -= to_read;
                }

                if (command_to_execute && arg_remains == 0) {
                    _logger->debug("Start command execution");

                    std::string result;
                    if (argument_for_command.size()) {
                        argument_for_command.resize(argument_for_command.size() - 2);
                    }
                    command_to_execute->Execute(*_pStorage, argument_for_command, result);

                    // Send response
                    result += "\r\n";
                    output_buffer.push_back(result);
                    if (!output_buffer.empty())
                    {
                        _event.events |= EPOLLOUT;
                    }
                    if (output_buffer.size() > limit)
                    {
                        _event.events &= ~EPOLLIN;
                    }
                    // Prepare for the next command
                    command_to_execute.reset();
                    argument_for_command.resize(0);
                    parser.Reset();
                }
            }
        }
        if (readed_bytes != -1) {
            if (readed_bytes == 0)
            {
                connection_close = true;
            }
            _logger->debug("Connection closed");
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                throw std::runtime_error(std::string(strerror(errno)));
            }
        }
    } catch (std::runtime_error &ex) {
        _logger->error("Failed to process connection on descriptor {}: {}", _socket, ex.what());
        flag_run.store(false, std::memory_order_relaxed);
    }
    std::atomic_thread_fence(std::memory_order_release);
}

// See Connection.h
void Connection::DoWrite()
{
    _logger->debug("Write on socket {}", _socket);
    const std::size_t vec_size = 16;
    iovec vec[vec_size] = {};
    if (output_buffer.empty())
    {
        return;
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    try
    {
        vec[0].iov_base = &(output_buffer[0][0]) + output_offset;
        vec[0].iov_len = output_buffer[0].size() - output_offset;
        std::size_t num = 1;
        while (num < output_buffer.size() && num < vec_size)
        {
            vec[num].iov_base = &(output_buffer[num][0]);
            vec[num].iov_len = output_buffer[num].size();
            num++;
        }
        int written_bytes = writev(_socket, vec, num);
        if (written_bytes >= 0)
        {
            std::size_t i = 0;
            while (written_bytes >= vec[i].iov_len && !output_buffer.empty())
            {
                written_bytes -= vec[i].iov_len;
                output_buffer.pop_front();
                i++;
            }
            output_offset = written_bytes;
        }
        else
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                throw std::runtime_error(std::string(strerror(errno)));
            }
        }
        if (output_buffer.size() <= limit - eps)
        {
            _event.events |= EPOLLIN;
        }
        if (output_buffer.empty())
        {
            _event.events &= ~EPOLLOUT;
            if (connection_close)
            {
                flag_run.store(false, std::memory_order_relaxed);
            }
        }
    }
    catch (std::runtime_error &ex) {
        _logger->error("Failed to process connection on descriptor {}: {}", _socket, ex.what());
        flag_run.store(false, std::memory_order_relaxed);
    }
    std::atomic_thread_fence(std::memory_order_release);
}
} // namespace MTnonblock
} // namespace Network
} // namespace Afina
