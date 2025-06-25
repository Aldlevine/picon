#pragma once

#include "color.hpp"
#include "convert.hpp"

namespace picon::graphics::color::blend
{

    constexpr struct None
    {
        template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
        constexpr static void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            r_dst = convert<T_DstFormat, T_SrcFormat>(p_src);                
        }
    } none;


    constexpr struct Alpha
    {
        template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
        requires(T_SrcFormat::alpha_bits == 0)
        constexpr static void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            None::operator()(r_dst, p_src);
        }

        template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
        requires(T_SrcFormat::alpha_bits == 1)
        constexpr static void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            if (p_src.getAlpha() > 0) { r_dst = convert<T_DstFormat, T_SrcFormat>(p_src); }
        }
    } alpha;

} // namespace picon::graphics::blend