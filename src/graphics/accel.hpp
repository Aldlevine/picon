#pragma once

#include <hardware/pio.h>

#include <cstdint>

namespace pg
{

    struct Accelerator
    {
        PIO pio;
        std::uint32_t pio_sm;
        std::uint32_t dma_src_channel;
        std::uint32_t dma_dst_channel;
        std::uint32_t dma_res_channel;
    };
} // namespace