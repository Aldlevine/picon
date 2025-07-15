#if defined(PICON_PLATFORM_PICO)

#include <pico/stdio.h>
#include <pico/rand.h>

#include <bit>
#include <cstdint>
#include <cerrno>

/// a workaround for missing _getentropy in pico-sdk / newlib.
/// silences linker errors, but probably overkill and/or entirely wrong.
extern "C" int _getentropy(void* buffer, std::size_t length)
{
    if (length > 256)
    {
        errno = EINVAL;
        return -1;
    }

    std::size_t i = 0;

    // set whole words at a time
    for (; i < length; i += 4)
    {
        std::bit_cast<std::uint32_t*>(buffer)[i] = get_rand_32();
    }
    
    // last word didn't get completely set, set it
    if (i > length)
    {
        std::bit_cast<std::uint32_t*>(buffer)[length - 4] = get_rand_32();
    }

    return 0;
}

#endif