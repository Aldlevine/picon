#pragma once

#if defined(PICON_PLATFORM_PICO)

#include <bit>
#include <cstdint>
#include <malloc.h>

// memory info

extern char __flash_binary_start;
extern char __flash_binary_end;
const auto flash_binary_start = std::bit_cast<std::uintptr_t>(&__flash_binary_start);
const auto flash_binary_end = std::bit_cast<std::uintptr_t>(&__flash_binary_end);

inline std::uint32_t getTotalHeap(void)
{
   extern char __StackLimit, __bss_end__;
   return &__StackLimit  - &__bss_end__;
}

inline std::uint32_t getFreeHeap(void)
{
   struct mallinfo m = mallinfo();
   return getTotalHeap() - m.uordblks;
}

#endif