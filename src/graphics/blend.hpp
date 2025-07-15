#pragma once

#include "color.hpp"
#include "convert.hpp"

namespace picon::graphics::color::blend
{
    template <typename T_BlendMode, typename T_DstColor, typename T_SrcColor>
    concept BlendMode =
        ColorType<T_DstColor> &&
        ColorType<T_SrcColor> &&
        requires(T_DstColor dst, T_SrcColor src, T_BlendMode blend)
        {
            { blend(dst, src) };
        };

    constexpr struct None
    {
        template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
        static constexpr void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            r_dst = convert<T_DstFormat, T_SrcFormat>(p_src);                
        }
    } none;


    constexpr struct Alpha
    {
        template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
        requires(!T_SrcFormat::template has_channel<A>)
        static constexpr void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            None::operator()(r_dst, p_src);
        }

        template <color::ColorType T_DstFormat, color::ColorType T_SrcFormat>
        requires(T_SrcFormat::template channel<A>.size == 1)
        static constexpr void operator()(T_DstFormat& r_dst, T_SrcFormat p_src)
        {
            if (p_src.template get<A>() > 0) { r_dst = convert<T_DstFormat, T_SrcFormat>(p_src); }
        }
    } alpha;

} // namespace picon::graphics::blend