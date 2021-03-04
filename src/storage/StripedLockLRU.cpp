#include "StripedLockLRU.h"
#include <stdexcept>

namespace Afina{
namespace Backend{

StripedLockLRU* StripedLockLRU::create_cash(std::size_t count = 8, std::size_t max_size = 16 * 1024 * 1024)
{
    size_t limit = max_size / count;
    if (limit < 1 * 1024 * 1024UL) {
        throw std::runtime_error("Limit size");
    }
    return new StripedLockLRU(count, limit);
}

}
}