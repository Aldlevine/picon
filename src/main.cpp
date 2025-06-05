#include "assets/images/images.hpp"

#include "drivers/sh1122.hpp"
#include "graphics/graphics.hpp"
#include "time/time.hpp"

#include <cmath>
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

int main()
{
    stdio_init_all();

    printf("Total Heap: %d\n", getTotalHeap());
    printf("Flash Binary Start: 0x%x\n", flash_binary_start);
    printf("Flash Binary End: 0x%x\n", flash_binary_end);

    display.init(pio0);

    pg::DeltaTimer display_timer{std::micro::den / 60};

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
