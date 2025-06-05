#pragma once

#include "graphics/image.hpp"

#include <hardware/dma.h>

#include <cstddef>
#include <string.h>

namespace pg
{

    template <>
    struct ImageFormatTraits<ImageFormat::GS4>
    {
        using Data = std::uint8_t;
        static constexpr ImagePxRatio px_ratio{1, 1};
        static constexpr Data max_value{0x0F};
    };

    template <std::size_t t_width, std::size_t t_height>
    struct Image<ImageFormat::GS4, t_width, t_height>
        : public Image<ImageFormat::GS4, t_width, t_height, Image<ImageFormat::GS4, t_width, t_height>>
    {
        using This = Image<ImageFormat::GS4, t_width, t_height>;
        using Base = Image<ImageFormat::GS4, t_width, t_height, This>;

        using typename Base::Data;

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

            auto index = ((width * p_y) + p_x);
            return buffer.at(index);
        }

        void setPixel(std::size_t p_x, std::size_t p_y, Data p_value)
        {
            assert(p_x < width);
            assert(p_y < height);
            assert(p_value <= 0x0F);

            auto index = ((width * p_y) + p_x);
            buffer.at(index) = p_value;
            // // TODO: this transparency masking needs to be configurable
            // if (p_value < 0x0F)
            // {
            //     auto index = ((width * p_y) + p_x);
            //     buffer.at(index) = p_value;
            // }
        }

        void fill(Data p_value)
        {
            memset(buffer.data(), p_value, buffer.size());
        }

        template <std::size_t t_other_width, std::size_t t_other_height>
        void blit(
            std::size_t p_dst_x, std::size_t p_dst_y,
            const Image<image_format, t_other_width, t_other_height> &p_src,
            std::size_t p_src_x, std::size_t p_src_y,
            std::size_t p_src_w, std::size_t p_src_h)
        {
            // clang-format off
            if (p_src_x == -1) { p_src_x = 0; }
            if (p_src_y == -1) { p_src_y = 0; }
            if (p_src_w == -1) { p_src_w = p_src.width; }
            if (p_src_h == -1) { p_src_h = p_src.height; }
            // clang-format on

            assert(p_dst_x + p_src_w <= width);
            assert(p_dst_y + p_src_h <= height);

            const auto dma_channel = static_cast<uint>(dma_claim_unused_channel(true));
            auto dma_config = dma_channel_get_default_config(dma_channel);

            channel_config_set_read_increment(&dma_config, true);
            channel_config_set_write_increment(&dma_config, true);
            channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);

            for (size_t sy = 0; sy < p_src_h; ++sy)
            {
                auto dst_index = ((width * (p_dst_y + sy)) + p_dst_x);
                auto src_index = ((p_src.width * (p_src_y + sy)) + p_src_x);
                dma_channel_configure(dma_channel, &dma_config, buffer.data() + dst_index, p_src.buffer.data() + src_index, p_src_w, true);
                dma_channel_wait_for_finish_blocking(dma_channel);
            }

            dma_channel_unclaim(dma_channel);
        }
    };

} // namespace pg