#include "assets/images.hpp"

#include "drivers/sh1122.hpp"
#include "graphics/color.hpp"
#include "graphics/functions.hpp"
#include "time/time.hpp"

#include <hardware/dma.h>
#include <hardware/pio.h>
#include <hardware/pio_instructions.h>
#include <hardware/timer.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <pico/rand.h>

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <malloc.h>
#include <ratio>


using namespace picon;


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


drivers::SH1122Driver<
    256,  // width
    64,   // height
    18,   // t_sck
    19,   // t_mosi,
    16,   // t_dc
    17,   // t_cs
    20    // t_rst
    > display{};

constexpr auto bg = assets::images::bg;
constexpr auto heart = assets::images::heart;

std::float_t bg_offset_x{ 0 };
std::float_t bg_offset_y{ 0 };
constexpr std::float_t bg_speed_x{(256.0) / std::micro::den / 8};
constexpr std::float_t bg_speed_y{(64.0) / std::micro::den / 8};


constexpr auto num_heart_cols = 16;
constexpr auto num_heart_rows = 16;

std::float_t heart_offset_x{ 1.0 * heart.width * -num_heart_cols };
std::float_t heart_offset_y{ 1.0 * heart.height * -num_heart_rows };
constexpr std::float_t heart_speed_x{(256.0 + heart.width * num_heart_cols) / std::micro::den / 2};
constexpr std::float_t heart_speed_y{(64.0 + heart.height * num_heart_rows) / std::micro::den / 8};

// constexpr auto num_heart_cols = 1;
// constexpr auto num_heart_rows = 480;
// constexpr auto num_heart_rows = 240;

// std::float_t heart_offset_x{ -1.0 * blit_image1.width };
// std::float_t heart_offset_y{ -1.0 * blit_image1.height };
// constexpr std::float_t heart_speed_x{(256.0 + blit_image1.width) / std::micro::den / 2};
// constexpr std::float_t heart_speed_y{(64.0 + blit_image1.height) / std::micro::den / 8};


void displayTick(std::uint64_t p_delta)
{
    auto fb = display.getBackBuffer();

    graphics::fn::fill(fb, graphics::color::GS4(0x00));

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

    graphics::fn::blitSafe(fb, bg_x - bg.width * 1, bg_y - bg.height * 1, bg);
    graphics::fn::blitSafe(fb, bg_x + bg.width * 0, bg_y - bg.height * 1, bg);
    graphics::fn::blitSafe(fb, bg_x + bg.width * 1, bg_y - bg.height * 1, bg);

    graphics::fn::blitSafe(fb, bg_x - bg.width * 1, bg_y + bg.height * 0, bg);
    graphics::fn::blitSafe(fb, bg_x + bg.width * 0, bg_y + bg.height * 0, bg);
    graphics::fn::blitSafe(fb, bg_x + bg.width * 1, bg_y + bg.height * 0, bg);

    graphics::fn::blitSafe(fb, bg_x - bg.width * 1, bg_y + bg.height * 1, bg);
    graphics::fn::blitSafe(fb, bg_x + bg.width * 0, bg_y + bg.height * 1, bg);
    graphics::fn::blitSafe(fb, bg_x + bg.width * 1, bg_y + bg.height * 1, bg);


    std::int16_t heart_x = heart_offset_x;
    std::int16_t heart_y = heart_offset_y;

    heart_offset_x += p_delta * heart_speed_x;
    heart_offset_y += p_delta * heart_speed_y;
    if (heart_offset_x > fb.width)
    {
        heart_offset_x = 1.0 * heart.width * -num_heart_cols;
        // heart_offset_x = -1.0 * blit_image1.width;
    }

    if (heart_offset_y > fb.height)
    {
        heart_offset_y = 1.0 * heart.height * -num_heart_rows;
        // heart_offset_y = -1.0 * blit_image1.height;
    }

    for (auto n = 0; n < 2; ++n)
    {
        for (auto i = 0; i < num_heart_cols; ++i)
        {
            for (auto j = 0; j < num_heart_rows; ++j)
            {
                graphics::fn::blitSafe(
                    fb,
                    heart_x + static_cast<std::int16_t>(i * heart.width),
                    heart_y + static_cast<std::int16_t>(j * heart.height),
                    // heart_x,
                    // heart_y,
                    heart,
                    graphics::blend::alpha);
            }
        }
    }

    display.swapBuffers();
}

int main()
{
    stdio_init_all();

    printf("Total Heap: %d\n", getTotalHeap());
    printf("Flash Binary Start: 0x%x\n", flash_binary_start);
    printf("Flash Binary End: 0x%x\n", flash_binary_end);

    display.init(pio0);

    // time::DeltaTimer display_timer{std::micro::den / 120};
    time::DeltaTimer display_timer{std::micro::den / 170};

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
