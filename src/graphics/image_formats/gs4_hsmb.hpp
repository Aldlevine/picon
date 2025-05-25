#pragma once

#include "graphics/image.hpp"

#include <cstddef>

namespace pg
{

    template <>
    struct ImageFormatTraits<ImageFormat::GS4_HMSB>
    {
        using Data = std::uint8_t;
        static constexpr ImagePxRatio px_ratio{1, 2};
        static constexpr Data max_value{0xF};
    };

    template <std::size_t t_width, std::size_t t_height>
    struct Image<ImageFormat::GS4_HMSB, t_width, t_height>
        : public Image<ImageFormat::GS4_HMSB, t_width, t_height, Image<ImageFormat::GS4_HMSB, t_width, t_height>>
    {
        using This = Image<ImageFormat::GS4_HMSB, t_width, t_height>;
        using Base = Image<ImageFormat::GS4_HMSB, t_width, t_height, This>;

        using typename Base::Data;
        using typename Base::BlendMode;

        using Base::image_format;
        using Base::px_ratio;
        using Base::max_value;
        using Base::width;
        using Base::height;

        using Base::buffer;

        constexpr Data getPixel(std::size_t p_x, std::size_t p_y) const
        {
            assert(p_x < width);
            assert(p_y < height);

            auto index = ((width * p_y) + p_x) / 2;

            if (p_x % 2 == 0)
            {
                return buffer.at(index) >> 4;
            }
            else
            {
                return buffer.at(index) & 0x0F;
            }
        }

        constexpr void setPixel(std::size_t p_x, std::size_t p_y, Data p_value, BlendMode p_blend_mode = {})
        {
            assert(p_x < width);
            assert(p_y < height);
            assert(p_value <= 0x0F);

            auto index = ((width * p_y) + p_x) / 2;

            if (p_x % 2 == 0)
            {
                auto dst = (buffer.at(index) & 0xF0) >> 4;
                auto tgt = p_blend_mode ? p_blend_mode(dst, p_value) : p_value;
                buffer.at(index) = (buffer.at(index) & 0X0F) | (tgt << 4);
            }
            else
            {
                auto dst = buffer.at(index) & 0x0F;
                auto tgt = p_blend_mode ? p_blend_mode(dst, p_value) : p_value;
                buffer.at(index) = (buffer.at(index) & 0xF0) | (tgt & 0x0F);
            }
        }
    };

} // namespace pg