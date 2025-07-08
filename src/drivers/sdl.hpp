#pragma once

#if defined(PICON_PLATFORM_LINUX)

#include "graphics/color.hpp"
#include "graphics/image.hpp"
#include "utils/bit_utils.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <cmath>
#include <cstddef>
#include <cstdlib>

namespace picon::drivers
{

    template <graphics::color::ColorType T_Color, std::size_t t_width, std::size_t t_height>
    struct SdlDriver
    {
    public:
        using FrameBufferData = graphics::ImageData<T_Color, t_width, t_height>;
        using FrameBuffer = graphics::Image<T_Color>;

        bool integer_scaling{false};
        
        std::array<FrameBufferData, 2> frame_buffer_data{};
        std::array<FrameBuffer, 2> frame_buffers{ frame_buffer_data[0], frame_buffer_data[1], };
        std::size_t front_buffer_idx{};

        SDL_Window* window{};
        SDL_Renderer* renderer{};
        bool is_software_renderer{};
        std::array<SDL_Surface*, 2> frame_buffer_surfaces{};
        std::array<SDL_Texture*, 2> frame_buffer_textures{};

        SDL_Palette* sdl_palette{};

        static constexpr SDL_PixelFormat sdl_pixel_format = [](){
            if constexpr (std::same_as<T_Color, graphics::color::GS4>) { return SDL_PIXELFORMAT_INDEX8; }
            else if constexpr (std::same_as<T_Color, graphics::color::GS4A1>) { return SDL_PIXELFORMAT_INDEX8; }
            else if constexpr (std::same_as<T_Color, graphics::color::R5G6B5>) { return SDL_PIXELFORMAT_RGB565; }
            else if constexpr (std::same_as<T_Color, graphics::color::R5G5B5A1>) { return SDL_PIXELFORMAT_RGBA5551; }
            else { static_assert(false, "unsupported pixel format"); }
        }();

    public:
        void init()
        {
            SDL_Init(SDL_INIT_VIDEO);

            SDL_Log("Available renderer drivers:");
            for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
                SDL_Log("%d. %s", i + 1, SDL_GetRenderDriver(i));
            }

            window = SDL_CreateWindow("", t_width * 2, t_height * 2, SDL_WINDOW_RESIZABLE);
            if (window == nullptr)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
                abort();
            }

            // try vulkan
            renderer = SDL_CreateRenderer(window, "vulkan");
            // fallback to default
            if (renderer == nullptr)
            {
                renderer = SDL_CreateRenderer(window, nullptr);
            }
            // fail
            if (renderer == nullptr)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
                abort();
            }

            if (std::string{SDL_GetRendererName(renderer)} == "software")
            {
                is_software_renderer = true;
            }


            if constexpr (sdl_pixel_format == SDL_PIXELFORMAT_INDEX8)
            {
                constexpr auto num_bits = T_Color::template channel<graphics::color::L>.size;
                constexpr auto num_colors = utils::bits<num_bits>;
                sdl_palette = SDL_CreatePalette(num_colors);
                for (std::size_t i = 0; i < num_colors; ++i)
                {
                    std::uint8_t value = utils::resizeBits<8, num_bits>(i);
                    sdl_palette->colors[i] = SDL_Color{value, value, value, utils::bits<8>};
                }
            }

            for (std::size_t i = 0; i < frame_buffers.size(); ++i)
            {
                frame_buffer_surfaces[i] =
                    SDL_CreateSurfaceFrom(
                        t_width,
                        t_height,
                        sdl_pixel_format,
                        frame_buffers[i].data(),
                        t_width * sizeof(typename FrameBuffer::Format::Value));

                if constexpr (sdl_pixel_format == SDL_PIXELFORMAT_INDEX8)
                {
                    SDL_SetSurfacePalette(frame_buffer_surfaces[i], sdl_palette);
                }

                if (!is_software_renderer)
                {
                    frame_buffer_textures[i] =
                        SDL_CreateTexture(
                            renderer,
                            sdl_pixel_format,
                            SDL_TEXTUREACCESS_STREAMING,
                            t_width,
                            t_height);
                        
                    SDL_SetTextureScaleMode(frame_buffer_textures[i], SDL_SCALEMODE_NEAREST);
                }
            }

        }

        void deinit()
        {
            for (std::size_t i = 0; i < frame_buffers.size(); ++i)
            {
                if (!is_software_renderer)
                {
                    SDL_DestroyTexture(frame_buffer_textures[i]);
                }
                SDL_DestroySurface(frame_buffer_surfaces[i]);
            }

            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }

        FrameBuffer& getFrontBuffer()
        {
            return frame_buffers[front_buffer_idx];
        }

        FrameBuffer& getBackBuffer()
        {
            return frame_buffers[(front_buffer_idx + 1) % frame_buffers.size()];
        }
        
        void swapBuffers()
        {
            const auto window_surface = SDL_GetWindowSurface(window);

            const auto window_aspect = static_cast<std::float_t>(window_surface->h) / window_surface->w;
            const auto buffer_aspect = static_cast<std::float_t>(t_height) / t_width;
            SDL_Rect dst_rect{ 0, 0, window_surface->w, window_surface->h };

            std::size_t scale = 0;
            if (window_aspect > buffer_aspect)
            {
                scale = window_surface->w / t_width;
            }
            else
            {
                scale = window_surface->h / t_height;
            }

            // integer scaling
            if (integer_scaling && scale > 0)
            {
                const auto target_height = t_height * scale;
                const auto target_width = t_width * scale;
                const auto diff_height = window_surface->h - target_height;
                const auto diff_width = window_surface->w - target_width;
                dst_rect.y = diff_height / 2;
                dst_rect.h = target_height;
                dst_rect.x = diff_width / 2;
                dst_rect.w = target_width;
            }
            // float scaling - bars above/below
            else if (window_aspect > buffer_aspect)
            {
                const auto target_height = window_surface->w * buffer_aspect;
                const auto diff_height = window_surface->h - target_height;
                dst_rect.y = diff_height / 2;
                dst_rect.h = target_height;
            }
            // float scaling - bars left/right
            else if (window_aspect < buffer_aspect)
            {
                const auto target_width = window_surface->h * (1.0f/buffer_aspect);
                const auto diff_width = window_surface->w - target_width;
                dst_rect.x = diff_width / 2;
                dst_rect.w = target_width;
            }

            if (is_software_renderer)
            {
                const auto back_buffer_surface = frame_buffer_surfaces[(front_buffer_idx + 1) % frame_buffer_surfaces.size()];
                
                SDL_ClearSurface(window_surface, 0, 0, 0, 1);
                SDL_StretchSurface(back_buffer_surface, nullptr, window_surface, &dst_rect, SDL_SCALEMODE_NEAREST);
                SDL_UpdateWindowSurface(window);
            }
            else
            {

                const SDL_FRect dst_frect = {
                    static_cast<std::float_t>(dst_rect.x),
                    static_cast<std::float_t>(dst_rect.y),
                    static_cast<std::float_t>(dst_rect.w),
                    static_cast<std::float_t>(dst_rect.h),
                };

                const auto back_buffer_texture = frame_buffer_textures[(front_buffer_idx + 1) % frame_buffer_surfaces.size()];
                typename T_Color::Value* pixels{};
                int pitch;
                SDL_LockTexture(back_buffer_texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
                assert(pitch == t_width * sizeof(typename T_Color::Value));
                std::copy(getBackBuffer().begin(), getBackBuffer().end(), pixels);
                SDL_UnlockTexture(back_buffer_texture);

                SDL_RenderClear(renderer);

                [[maybe_unused]] const auto sdl_render_success =
                    SDL_RenderTexture(renderer, back_buffer_texture, nullptr, &dst_frect);
                    
                #if !defined(NDEBUG)
                    if (!sdl_render_success)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not render texture: %s\n", SDL_GetError());
                        abort();
                    }
                #endif

                SDL_RenderPresent(renderer);
            }
            
            front_buffer_idx = (front_buffer_idx + 1) % frame_buffers.size();
        }
    };

} // namespace picon::drivers

#endif