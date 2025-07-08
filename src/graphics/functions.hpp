#pragma once

#include "blend.hpp"
#include "color.hpp"
#include "convert.hpp"
#include "image.hpp"

#include "utils/types.hpp"

#include <algorithm>

namespace picon::graphics::fn
{

    /// generic fill.
    template <
        color::ColorType T_DstFormat,
        color::ColorType T_SrcFormat,
        color::blend::BlendMode<T_DstFormat, T_SrcFormat> T_Blend=color::blend::None
    >
    inline void fill(Image<T_DstFormat> p_dst, T_SrcFormat p_value, T_Blend p_blend = {})
    {
        std::ranges::fill(p_dst, color::convert<T_DstFormat>(p_value));
    }


    /// fill rect.
    template <
        color::ColorType T_DstFormat,
        color::ColorType T_SrcFormat,
        color::blend::BlendMode<T_DstFormat, T_SrcFormat> T_Blend=color::blend::None
    >
    inline void fillRect(
        Image<T_DstFormat> p_dst,
        std::size_t p_dst_x, std::size_t p_dst_y,
        std::size_t p_dst_w, std::size_t p_dst_h,
        T_SrcFormat p_value,
        T_Blend p_blend = {}
    )
    {
        for (std::size_t y = 0; y < p_dst_h; ++y)
        {
            std::fill(
                p_dst.rowBegin(p_dst_y + y) + p_dst_x,
                p_dst.rowBegin(p_dst_y + y) + p_dst_x + p_dst_w,
                color::convert<T_DstFormat>(p_value)
            );
        }
    }
    

    /// blit safe resize.
    /// returns true if dst dst rect is in view
    template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
    inline bool blitSafeSize(
        const Image<T_DstFormat> p_dst, utils::isize_t& r_dst_x, utils::isize_t& r_dst_y,
        const Image<T_SrcFormat> p_src, utils::isize_t& r_src_x, utils::isize_t& r_src_y, utils::isize_t& r_src_w, utils::isize_t& r_src_h
    )
    {
        // completely left of image
        if (-r_dst_x > r_src_w) { return false; }
        // completely right of image
        if (r_dst_x > static_cast<utils::isize_t>(p_dst.width)) { return false; }
        // completely above of image
        if (-r_dst_y > r_src_h) { return false; }
        // completely below of image
        if (r_dst_y > static_cast<utils::isize_t>(p_dst.height)) { return false; }
        

        // left of image
        if (r_dst_x < 0)
        {
            r_src_x = -r_dst_x;
            r_src_w = r_src_w + r_dst_x;
            r_dst_x = 0;
        }

        // right of image
        if (r_dst_x + r_src_w > static_cast<utils::isize_t>(p_dst.width))
        {
            r_src_w = p_dst.width - r_dst_x;
        }

        // above of image
        if (r_dst_y < 0)
        {
            r_src_y = -r_dst_y;
            r_src_h = r_src_h + r_dst_y;
            r_dst_y = 0;
        }

        // below image
        if (r_dst_y + r_src_h > static_cast<utils::isize_t>(p_dst.height))
        {
            r_src_h = p_dst.height - r_dst_y;
        }

        return true;
    }


    /// sized blit.
    template <
        color::ColorType T_DstFormat,
        color::ColorType T_SrcFormat,
        color::blend::BlendMode<T_DstFormat, T_SrcFormat> T_Blend=color::blend::None
    >
    inline void blit(
        Image<T_DstFormat> p_dst, std::size_t p_dst_x, std::size_t p_dst_y,
        const Image<T_SrcFormat> p_src, std::size_t p_src_x, std::size_t p_src_y, std::size_t p_src_w, std::size_t p_src_h,
        T_Blend p_blend={}
    )
    {
        for (std::size_t y = 0; y < p_src_h; ++y)
        {
            for (std::size_t x = 0; x < p_src_w; ++x)
            {
                const auto& src_px = p_src.at(p_src_x + x, p_src_y + y);
                p_blend(p_dst.at(p_dst_x + x, p_dst_y + y), src_px);
            }
        }
    }

    
    /// generic safe sized blit.
    template <
        color::ColorType T_DstFormat,
        color::ColorType T_SrcFormat,
        color::blend::BlendMode<T_DstFormat, T_SrcFormat> T_Blend=color::blend::None
    >
    inline void blitSafe(
        Image<T_DstFormat> p_dst, utils::isize_t p_dst_x, utils::isize_t p_dst_y,
        const Image<T_SrcFormat> p_src, utils::isize_t p_src_x, utils::isize_t p_src_y, utils::isize_t p_src_w, utils::isize_t p_src_h,
        T_Blend p_blend={}
    )
    {
        if (blitSafeSize(p_dst, p_dst_x, p_dst_y, p_src, p_src_x, p_src_y, p_src_w, p_src_h))
        {
            blit(p_dst, p_dst_x, p_dst_y, p_src, p_src_x, p_src_y, p_src_w, p_src_h, p_blend);
        }
    }


    /// generic safe full src blit.
    /// forwards to appropriate safe sized blit.
    template <
        color::ColorType T_DstFormat,
        color::ColorType T_SrcFormat,
        color::blend::BlendMode<T_DstFormat, T_SrcFormat> T_Blend=color::blend::None
    >
    inline void blit(
        Image<T_DstFormat> p_dst, std::size_t p_dst_x, std::size_t p_dst_y, const Image<T_SrcFormat> p_src,
        T_Blend p_blend={}
    )
    {
        blit(p_dst, p_dst_x, p_dst_y, p_src, 0, 0, p_src.width, p_src.height, p_blend);
    }

    /// generic full src blit.
    /// forwards to appropriate sized blit.
    template <
        color::ColorType T_DstFormat,
        color::ColorType T_SrcFormat,
        color::blend::BlendMode<T_DstFormat, T_SrcFormat> T_Blend=color::blend::None
    >
    inline void blitSafe(
        Image<T_DstFormat> p_dst, utils::isize_t p_dst_x, utils::isize_t p_dst_y, const Image<T_SrcFormat> p_src,
        T_Blend p_blend={}
    )
    {
        blitSafe(p_dst, p_dst_x, p_dst_y, p_src, 0, 0, p_src.width, p_src.height, p_blend);
    }


} // namespace picon::graphics::fn