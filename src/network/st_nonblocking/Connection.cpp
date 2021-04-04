#include "Connection.h"

#include <iostream>

namespace Afina {
namespace Network {
namespace STnonblock {

// See Connection.h
void Connection::Start()
{
    flag_run = true;
    std::cout << "Start" << std::endl;
}

// See Connection.h
void Connection::OnError()
{
    flag_run = false;
    std::cout << "OnError. Socket " << _socket << std::endl;
}

// See Connection.h
void Connection::OnClose()
{
    flag_run = false;
    std::cout << "OnClose" << std::endl;
}

// See Connection.h
void Connection::DoRead()
{
    std::cout << "DoRead" << std::endl;
}

// See Connection.h
void Connection::DoWrite()
{
    std::cout << "DoWrite" << std::endl;
}

} // namespace STnonblock
} // namespace Network
} // namespace Afina
