#include "config/config.hpp"

#include "assets/images.hpp"

#include "graphics/color.hpp"
#include "graphics/functions.hpp"
#include "graphics/image.hpp"
#include "time/time.hpp"

#include <cmath>
#include <cstddef>
#include <ratio>


#if defined(PICON_PLATFORM_PICO)
#include "drivers/sh1122.hpp"
#include "pal/pico/pico.hpp"
#include <pico/stdio.h>
#endif // defined(PICON_PLATFORM_PICO)

#if defined(PICON_PLATFORM_LINUX)
#include "drivers/sdl.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#endif // defined(PICON_PLATFORM_LINUX)

using namespace picon;

#if defined(PICON_PLATFORM_PICO)
drivers::SH1122Driver<
    256,  // t_width
    64,   // t_height
    18,   // t_sck
    19,   // t_mosi
    16,   // t_dc
    17,   // t_cs
    20    // t_rst
> display{.pio = pio0};
#endif // defined(PICON_PLATFORM_PICO)

#if defined(PICON_PLATFORM_LINUX)
drivers::SdlDriver<
    // graphics::color::GS4,       // T_Color
    // graphics::color::R8G8B8,    // T_Color
    graphics::color::R4G4B4A4,  // T_Color
    256,                        // t_width
    64                          // t_height
> display{.integer_scaling = true};
#endif // defined(PICON_PLATFORM_LINUX)

constexpr auto bg = assets::images::bg;
constexpr auto heart = assets::images::heart;

constexpr std::size_t fb_width = decltype(display)::width;
constexpr std::size_t fb_height = decltype(display)::height;

std::float_t bg_offset_x{ 0 };
std::float_t bg_offset_y{ 0 };
constexpr std::float_t bg_speed_x{1.0 * fb_width / std::micro::den / 8};
constexpr std::float_t bg_speed_y{1.0 * fb_height / std::micro::den / 8};


constexpr auto num_heart_cols = 16;
constexpr auto num_heart_rows = 16;

std::float_t heart_offset_x{ 1.0 * heart.width * -num_heart_cols };
std::float_t heart_offset_y{ 1.0 * heart.height * -num_heart_rows };
constexpr std::float_t heart_speed_x{(1.0 * fb_width + heart.width * num_heart_cols) / std::micro::den / 2};
constexpr std::float_t heart_speed_y{(1.0 * fb_height + heart.height * num_heart_rows) / std::micro::den / 8};

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

    // graphics::fn::fill(fb, graphics::color::R5G6B5{0, 0, 0});
    graphics::fn::fill(fb, graphics::color::R4G4B4A4{0, 0, 0, 0});

    bg_offset_x += p_delta * bg_speed_x;
    bg_offset_y += p_delta * bg_speed_y;
    if (bg_offset_x > fb_width)
    {
        bg_offset_x = 0;
    }

    if (bg_offset_y > fb_height)
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
    if (heart_offset_x > fb_width)
    {
        heart_offset_x = 1.0 * heart.width * -num_heart_cols;
        // heart_offset_x = -1.0 * blit_image1.width;
    }

    if (heart_offset_y > fb_height)
    {
        heart_offset_y = 1.0 * heart.height * -num_heart_rows;
        // heart_offset_y = -1.0 * blit_image1.height;
    }

    for (auto n = 0; n < 1; ++n)
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
                    graphics::color::blend::alpha);
            }
        }
    }
    
    graphics::fn::fillRect(fb, 120 + 16, 24, 16, 16, graphics::color::GS4{0b0010});
    // graphics::fn::fillRect(fb, 120 + 16, 24, 16, 16, graphics::color::R5G5B5A1{15, 15, 15, 1});
    // graphics::fn::fillRect(fb, 120 + 16, 24, 16, 16, graphics::color::R5G5B5A1{31, 0, 0, 1});
    display.swapBuffers();
}

int main()
{
    #if defined(PICON_PLATFORM_PICO)
    stdio_init_all();

    printf("Total Heap: %d\n", getTotalHeap());
    printf("Flash Binary Start: 0x%x\n", flash_binary_start);
    printf("Flash Binary End: 0x%x\n", flash_binary_end);

    #elif defined(PICON_PLATFORM_LINUX)
    #endif

    display.init();

    // time::DeltaTimer display_timer{std::micro::den / 60};
    time::DeltaTimer display_timer{std::micro::den / 120};
    // time::DeltaTimer display_timer{std::micro::den / 15};

    while (true)
    {

        #if defined(PICON_PLATFORM_LINUX)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                return 0;
            }
        }
        #endif

        display_timer.tick();
        while (display_timer.step())
        {
            displayTick(display_timer.getDelta());
        }
        
        #if defined (PICON_PLATFORM_PICO)
        tight_loop_contents();

        #elif defined (PICON_PLATFORM_LINUX)
        SDL_Delay(std::max(display_timer.getDeltaLeft() / 1'000, 20uz) - 20);

        #endif
    }
    
}

