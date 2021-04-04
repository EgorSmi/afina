#include "Connection.h"

#include <afina/execute/Command.h>
#include <afina/concurrency/Executor.h>
#include "protocol/Parser.h"

#include <iostream>
#include <sys/uio.h>

namespace Afina {
namespace Network {
namespace STnonblock {

// See Connection.h
void Connection::Start()
{
    flag_run = true;
    _event.events = EPOLLERR | EPOLLHUP | EPOLLIN;
    _logger->debug("Start socket {}", _socket);
}

// See Connection.h
void Connection::OnError()
{
    flag_run = false;
    _logger->debug("Error on socket {}", _socket);
}

// See Connection.h
void Connection::OnClose()
{
    flag_run = false;
    _logger->debug("Close on socket {}", _socket);
}

// See Connection.h
void Connection::DoRead()
{
    _logger->debug("Read on socket {}", _socket);
    std::size_t arg_remains = 0;
    Protocol::Parser parser;
    std::string argument_for_command;
    std::unique_ptr<Execute::Command> command_to_execute;
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
                    if (parser.Parse(client_buffer, pos, parsed)) {
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
                    output_buffer.emplace_back(result);
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
        if (readed_bytes == 0) {
            flag_run = false;
            _logger->debug("Connection closed");
        } else {
            throw std::runtime_error(std::string(strerror(errno)));
        }
    } catch (std::runtime_error &ex) {
        _logger->error("Failed to process connection on descriptor {}: {}", _socket, ex.what());
        flag_run = false;
    }
}

// See Connection.h
void Connection::DoWrite()
{
    _logger->debug("Write on socket {}", _socket);
    if (!output_buffer.empty())
    {
        const std::size_t vec_size = 16;
        iovec vec[vec_size];
        try
        {
            std::size_t num = 0;
            vec[num].iov_base = &(output_buffer[num][0]) + output_offset;
            vec[num].iov_len = output_buffer[num].size() - output_offset;
            num++;
            while (num < output_buffer.size() && num < vec_size)
            {
                vec[num].iov_base = &(output_buffer[num][0]);
                vec[num].iov_len = output_buffer[num].size();
                num++;
            }
            int written_bytes = -1;
            if ((written_bytes = writev(_socket, vec, num)) >= 0)
            {
                std::size_t i = 0;
                while (written_bytes >= vec[i].iov_len)
                {
                    written_bytes -= vec[i].iov_len;
                    output_buffer.pop_front();
                    i++;
                }
                output_offset = written_bytes;
            }
            else
            {
                throw std::runtime_error(std::string(strerror(errno)));
            }
            if (output_buffer.size() < limit - 20)
            {
                _event.events |= EPOLLIN;
            }
            if (output_buffer.empty())
            {
                _event.events &= ~EPOLLOUT;
            }
        }
        catch (std::runtime_error &ex) {
            _logger->error("Failed to process connection on descriptor {}: {}", _socket, ex.what());
            flag_run = false;
        }
    }
}

} // namespace STnonblock
} // namespace Network
} // namespace Afina
