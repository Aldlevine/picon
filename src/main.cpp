#include "assets/images/images.hpp"

#include "drivers/sh1122.hpp"
#include "graphics/graphics.hpp"
#include "time/time.hpp"

#include <hardware/dma.h>
#include <hardware/pio.h>
#include <hardware/pio_instructions.h>
#include <hardware/timer.h>
#include <pico/time.h>

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <malloc.h>
#include <ratio>

extern char __flash_binary_start;
extern char __flash_binary_end;
uintptr_t flash_binary_start = (uintptr_t) &__flash_binary_start;
uintptr_t flash_binary_end = (uintptr_t) &__flash_binary_end;

uint32_t getTotalHeap(void)
{
   extern char __StackLimit, __bss_end__;
   return &__StackLimit  - &__bss_end__;
}
uint32_t getFreeHeap(void)
{
   struct mallinfo m = mallinfo();
   return getTotalHeap() - m.uordblks;
}


pg::SH1122Driver<
    256,  // width
    64,   // height
    18,   // t_sck
    19,   // t_mosi,
    16,   // t_dc
    17,   // t_cs
    20    // t_rst
    > display{};


constexpr auto& bg_image = pg::assets::bg;
constexpr auto& blit_image1 = pg::assets::heart;
const auto blit_image2 = blit_image1.flipped(true, true);

std::float_t bg_offset_x{ 0 };
std::float_t bg_offset_y{ 0 };
constexpr std::float_t bg_speed_x{(256.0) / std::micro::den / 8};
constexpr std::float_t bg_speed_y{(64.0) / std::micro::den / 8};

constexpr auto num_heart_cols = 8;
constexpr auto num_heart_rows = 8;

std::float_t heart_offset_x{ 1.0 * blit_image1.width * -num_heart_cols };
std::float_t heart_offset_y{ 1.0 * blit_image1.height * -num_heart_rows };
constexpr std::float_t heart_speed_x{(256.0 + blit_image1.width * num_heart_cols) / std::micro::den / 2};
constexpr std::float_t heart_speed_y{(64.0 + blit_image1.height * num_heart_rows) / std::micro::den / 8};


void displayTick(std::uint64_t p_delta)
{
    auto &fb = display.getBackBuffer();

    fb.fill(0x0F);

    bg_offset_x += p_delta * bg_speed_x;
    bg_offset_y += p_delta * bg_speed_y;
    if (bg_offset_x > fb.width)
    {
        bg_offset_x = 0;
    }

    if (bg_offset_y > fb.height)
    {
        bg_offset_y = 0;
    }
    
    std::int64_t bg_x = bg_offset_x;
    std::int64_t bg_y = bg_offset_y;

    fb.blitSafe(bg_x - bg_image.width * 1, bg_y - bg_image.height * 1, bg_image, -1, -1, -1, -1);
    fb.blitSafe(bg_x + bg_image.width * 0, bg_y - bg_image.height * 1, bg_image, -1, -1, -1, -1);
    fb.blitSafe(bg_x + bg_image.width * 1, bg_y - bg_image.height * 1, bg_image, -1, -1, -1, -1);

    fb.blitSafe(bg_x - bg_image.width * 1, bg_y + bg_image.height * 0, bg_image, -1, -1, -1, -1);
    fb.blitSafe(bg_x + bg_image.width * 0, bg_y + bg_image.height * 0, bg_image, -1, -1, -1, -1);
    fb.blitSafe(bg_x + bg_image.width * 1, bg_y + bg_image.height * 0, bg_image, -1, -1, -1, -1);

    fb.blitSafe(bg_x - bg_image.width * 1, bg_y + bg_image.height * 1, bg_image, -1, -1, -1, -1);
    fb.blitSafe(bg_x + bg_image.width * 0, bg_y + bg_image.height * 1, bg_image, -1, -1, -1, -1);
    fb.blitSafe(bg_x + bg_image.width * 1, bg_y + bg_image.height * 1, bg_image, -1, -1, -1, -1);


    std::int16_t heart_x = heart_offset_x;
    std::int16_t heart_y = heart_offset_y;

    heart_offset_x += p_delta * heart_speed_x;
    heart_offset_y += p_delta * heart_speed_y;
    if (heart_offset_x > fb.width)
    {
        heart_offset_x = 1.0 * blit_image1.width * -num_heart_cols;
    }

    if (heart_offset_y > fb.height)
    {
        heart_offset_y = 1.0 * blit_image1.height * -num_heart_rows;
    }
    
    for (auto i = 0; i < num_heart_cols; ++i)
    {
        const auto &image = i % 2 == 0 ? blit_image1 : blit_image2;

        for (auto j = 0; j < num_heart_rows; ++j)
        {
            fb.blitSafe(
                heart_x + static_cast<std::int16_t>(i * image.width),
                heart_y + static_cast<std::int16_t>(j * image.height),
                image,
                -1, -1,
                -1, -1);
        }
    }

    display.swapBuffers();
}

void maskedDmaTest()
{
    using Data = std::uint8_t;
    constexpr auto num_bits = sizeof(Data) * CHAR_BIT;
    assert(num_bits < 16);

    constexpr auto write_src_label = 6;
    constexpr auto write_dst_label = 2;

    std::array pio_commands = {
        static_cast<std::uint16_t>( pio_encode_out(pio_src_dest::pio_x, num_bits) ), // get x(src) from fifo
        static_cast<std::uint16_t>( pio_encode_jmp_x_ne_y(write_src_label) ), // if x(src) != y(mask) write x(src)
        // write_dst_label
        static_cast<std::uint16_t>( pio_encode_set(pio_src_dest::pio_x, 0) ), // clear x(src)
        static_cast<std::uint16_t>( pio_encode_out(pio_src_dest::pio_x, num_bits) ), // get x(dst) from fifo
        static_cast<std::uint16_t>( pio_encode_in(pio_src_dest::pio_x, num_bits) ), // shift x(dst) to fifo
        static_cast<std::uint16_t>( pio_encode_jmp(0) ), // jump to start
        // write_src_label
        static_cast<std::uint16_t>( pio_encode_out(pio_src_dest::pio_null, num_bits) ), // toss dst bits from fifo
        static_cast<std::uint16_t>( pio_encode_in(pio_src_dest::pio_x, num_bits) ), // shift x(src) to fifo
    };

    const pio_program_t program{
        .instructions = pio_commands.data(),
        .length = pio_commands.size(),
        .origin = -1,
        .pio_version = 0,
    };

    const auto pio{pio1};
    const auto sm{pio_claim_unused_sm(pio, true)};
    const auto offset{pio_add_program(pio, &program)};

    auto pio_config{pio_get_default_sm_config()};
    sm_config_set_wrap(&pio_config, offset, offset + pio_commands.size() - 1);
    sm_config_set_in_shift(&pio_config, false, true, num_bits);
    sm_config_set_out_shift(&pio_config, true, true, num_bits);

    pio_sm_init(pio, sm, offset, &pio_config);
    pio_sm_set_enabled(pio, sm, true);

    const auto src_chan{dma_claim_unused_channel(true)};
    const auto dst_chan{dma_claim_unused_channel(true)};
    const auto res_chan{dma_claim_unused_channel(true)};
    auto src_config{dma_channel_get_default_config(src_chan)};
    auto dst_config{dma_channel_get_default_config(dst_chan)};
    auto res_config{dma_channel_get_default_config(res_chan)};

    channel_config_set_transfer_data_size(&src_config, dma_channel_transfer_size::DMA_SIZE_8);
    channel_config_set_transfer_data_size(&dst_config, dma_channel_transfer_size::DMA_SIZE_8);
    channel_config_set_transfer_data_size(&res_config, dma_channel_transfer_size::DMA_SIZE_8);

    channel_config_set_read_increment(&src_config, true);
    channel_config_set_read_increment(&dst_config, true);
    channel_config_set_read_increment(&res_config, false);

    channel_config_set_write_increment(&src_config, false);
    channel_config_set_write_increment(&dst_config, false);
    channel_config_set_write_increment(&res_config, true);

    channel_config_set_dreq(&src_config, pio_get_dreq(pio, sm, true));
    channel_config_set_dreq(&dst_config, pio_get_dreq(pio, sm, true));
    channel_config_set_dreq(&res_config, pio_get_dreq(pio, sm, false));

    channel_config_set_chain_to(&src_config, dst_chan);
    channel_config_set_chain_to(&dst_config, src_chan);

    constexpr std::uint8_t mask_value{0};
    std::array<std::uint8_t, 8> dst_buffer  {1, 2, 3, 4, 5, 6, 7, 8};
    std::array<std::uint8_t, 8> src_buffer  {0, 1, 0, 2, 0, 3, 0, 4};
    std::array<std::uint8_t, 8> src2_buffer {1, 0, 2, 0, 3, 0, 4, 0};
                                             
    // set mask value
    pio_sm_exec(pio, sm, pio_encode_set(pio_src_dest::pio_y, 0)); // clear y(mask)
    pio_sm_exec(pio, sm, pio_encode_out(pio_src_dest::pio_y, num_bits)); // get y(mask) from fifo
    pio_sm_put_blocking(pio, sm,mask_value); // put mask on fifo

    printf("\nsrc 1:\n");
    {
        // reset pio
        pio_sm_clear_fifos(pio, sm);
        pio->sm[sm].instr = offset;

        // setup dma channels
        dma_channel_configure(
            src_chan, &src_config, &pio->txf[sm], src_buffer.data(), 1, false);
        dma_channel_configure(
            dst_chan, &dst_config, &pio->txf[sm], dst_buffer.data(), 1, false);
        dma_channel_configure(
            res_chan, &res_config, dst_buffer.data(), &pio->rxf[sm], dst_buffer.size(), false);

        // run dma and cleanup when done
        dma_start_channel_mask((1 << src_chan) | (1 << res_chan));
        dma_channel_wait_for_finish_blocking(res_chan);
        dma_channel_cleanup(src_chan);
        dma_channel_cleanup(dst_chan);

        for (std::size_t i = 0; i < dst_buffer.size(); ++i)
        {
            printf("    %d = %d\n", i, dst_buffer.at(i));
        }
    }

    printf("\nsrc 2:\n");
    {
        // reset pio
        pio_sm_clear_fifos(pio, sm);
        pio->sm[sm].instr = offset;

        // setup dma channels
        dma_channel_configure(
            src_chan, &src_config, &pio->txf[sm], src2_buffer.data(), 1, false);
        dma_channel_configure(
            dst_chan, &dst_config, &pio->txf[sm], dst_buffer.data(), 1, false);
        dma_channel_configure(
            res_chan, &res_config, dst_buffer.data(), &pio->rxf[sm], dst_buffer.size(), false);

        // run dma and cleanup when done
        dma_start_channel_mask((1 << src_chan) | (1 << res_chan));
        dma_channel_wait_for_finish_blocking(res_chan);
        dma_channel_cleanup(src_chan);
        dma_channel_cleanup(dst_chan);

        for (std::size_t i = 0; i < dst_buffer.size(); ++i)
        {
            printf("    %d = %d\n", i, dst_buffer.at(i));
        }
    }
}

int main()
{
    stdio_init_all();

    printf("Total Heap: %d\n", getTotalHeap());
    printf("Flash Binary Start: 0x%x\n", flash_binary_start);
    printf("Flash Binary End: 0x%x\n", flash_binary_end);

    maskedDmaTest();

    display.init(pio0);

    pg::DeltaTimer display_timer{std::micro::den / 240};

    while (true)
    {
        display_timer.tick();
        while (display_timer.step())
        {
            displayTick(display_timer.getDelta());
        }
        tight_loop_contents();
    }
    
}
