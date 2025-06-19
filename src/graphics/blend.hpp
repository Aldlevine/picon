#pragma once

#include "color.hpp"

namespace picon::graphics::blend
{

    constexpr struct None
    {
        template <color::Color T_DstFormat, color::Color T_SrcFormat>
        constexpr static void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            r_dst = p_src;                
        }
    } none;


    constexpr struct Alpha
    {
        template <color::Color T_DstFormat, color::Color T_SrcFormat>
        requires(T_SrcFormat::config.alphaBits() == 0)
        constexpr static void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            None(r_dst, p_src);
        }

        template <color::Color T_DstFormat, color::Color T_SrcFormat>
        requires(T_SrcFormat::config.alphaBits() == 1)
        constexpr static void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            if (p_src.a() > 0) { r_dst = p_src; }
        }
    } alpha;

} // namespace picon::graphics::blend