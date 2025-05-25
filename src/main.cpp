#include "assets/images/images.hpp"

#include "drivers/sh1122.hpp"
#include "graphics/graphics.hpp"
#include "time/time.hpp"

#include <hardware/flash.h>
#include <hardware/spi.h>
#include <pico.h>
#include <pico/stdlib.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <malloc.h>

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


template <typename T_Data, T_Data t_value>
T_Data valueScissor(T_Data p_dst, T_Data p_src)
{
    if (p_src == t_value)
    {
        return p_dst;
    }
    return p_src;
}
constexpr auto gs4WhiteScissor{valueScissor<std::uint8_t, 0x0F>};


const auto& bg_image = pg::assets::bg;
const auto& blit_image1 = pg::assets::heart;
const auto blit_image2 = blit_image1.flipped(true, true);

std::float_t bg_offset_x{ 0 };
std::float_t bg_offset_y{ 0 };
constexpr std::float_t bg_speed_x{(256.0) / 1'000'000 * 0.125};
constexpr std::float_t bg_speed_y{(64.0) / 1'000'000 * 0.125};

std::float_t heart_offset_x{ blit_image1.width * -2.0 };
std::float_t heart_offset_y{ blit_image1.height * -4.0 };
constexpr std::float_t heart_speed_x{(256.0 + 32.0) / 1'000'000 * 0.5};
constexpr std::float_t heart_speed_y{(64.0 + 64.0) / 1'000'000 * 0.25};


void displayTick(std::uint64_t p_delta)
{
    auto &fb = display.getBackBuffer();

    fb.fill(0x00);

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
        heart_offset_x = blit_image1.width * -2.0;
    }

    if (heart_offset_y > fb.height)
    {
        heart_offset_y = blit_image1.height * -4.0;
    }

    fb.blitSafe(heart_x, heart_y + 00, blit_image1, -1, -1, -1, -1, gs4WhiteScissor);
    fb.blitSafe(heart_x, heart_y + 16, blit_image1, -1, -1, -1, -1, gs4WhiteScissor);
    fb.blitSafe(heart_x, heart_y + 32, blit_image1, -1, -1, -1, -1, gs4WhiteScissor);
    fb.blitSafe(heart_x, heart_y + 48, blit_image1, -1, -1, -1, -1, gs4WhiteScissor);

    heart_x += blit_image1.width;
    fb.blitSafe(heart_x, heart_y + 00, blit_image2, -1, -1, -1, -1, gs4WhiteScissor);
    fb.blitSafe(heart_x, heart_y + 16, blit_image2, -1, -1, -1, -1, gs4WhiteScissor);
    fb.blitSafe(heart_x, heart_y + 32, blit_image2, -1, -1, -1, -1, gs4WhiteScissor);
    fb.blitSafe(heart_x, heart_y + 48, blit_image2, -1, -1, -1, -1, gs4WhiteScissor);

    display.swapBuffers();
}


int main()
{
    stdio_init_all();

    printf("Total Heap: %d\n", getTotalHeap());
    printf("Flash Binary Start: 0x%x\n", flash_binary_start);
    printf("Flash Binary End: 0x%x\n", flash_binary_end);

    display.init(spi0);

    pg::DeltaTimer display_timer{1'000'000 / 120};

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
