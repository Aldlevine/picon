#pragma once

#include "color.hpp"
#include "utils/bit_utils.hpp"

#include <cstddef>
// #include <tuple>
#include <utility>

namespace picon::graphics::color
{
    // template <std::size_t t_color_channels>
    // using ChannelReduceWeight = std::array<std::size_t, t_color_channels>;

    // namespace channel_reduce
    // {
    //     template <std::size_t t_color_channels>
    //     constexpr auto uniform = []<std::size_t... t_i>(std::index_sequence<t_i...>){
    //         return ChannelReduceWeight<t_color_channels>{(std::ignore = t_i, 1)...};
    //     }(std::make_index_sequence<t_color_channels>());

    //     constexpr ChannelReduceWeight<3> itu_rec_601 = {299, 587, 114};
    // }
    
    /// convert from color to same color.
    template <ColorType T_DstColor, ColorType T_SrcColor>
    requires(
        std::same_as<std::remove_cvref_t<T_DstColor>, std::remove_cvref_t<T_SrcColor>>
    )
    constexpr T_DstColor convert(T_SrcColor p_src)
    {
        return p_src;
    }


    /// convert to / from same number of color channels.
    template<ColorType T_DstColor, ColorType T_SrcColor>
    requires(
        !std::same_as<std::remove_cvref_t<T_DstColor>, std::remove_cvref_t<T_SrcColor>> &&
        T_DstColor::num_color_channels == T_SrcColor::num_color_channels
    )
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        if constexpr (!T_DstColor::has_alpha_channel)
        {
            return [&]<std::size_t... t_i>(std::index_sequence<t_i...>)
            {
                return T_DstColor{
                    (utils::resizeBits<
                        T_DstColor::template color_bits<t_i>,
                        T_SrcColor::template color_bits<t_i>
                    >(p_src.template getColor<t_i>()))...,
                };
            }(std::make_index_sequence<T_DstColor::num_color_channels>());
        }

        else if (!T_SrcColor::has_alpha_channel == 0)
        {
            return [&]<std::size_t... t_i>(std::index_sequence<t_i...>)
            {
                return T_DstColor{
                    (utils::resizeBits<
                        T_DstColor::template color_bits<t_i>,
                        T_SrcColor::template color_bits<t_i>
                    >(p_src.template getColor<t_i>()))...,
                    utils::bits<T_DstColor::alpha_bits>,
                };
            }(std::make_index_sequence<T_DstColor::num_color_channels>());
        }

        else if (T_SrcColor::has_alpha_channel)
        {
            return [&]<std::size_t... t_i>(std::index_sequence<t_i...>)
            {
                return T_DstColor{
                    (utils::resizeBits<
                        T_DstColor::template color_bits<t_i>,
                        T_SrcColor::template color_bits<t_i>
                    >(p_src.template getColor<t_i>()))...,
                    utils::resizeBits<
                        T_DstColor::alpha_bits,
                        T_SrcColor::alpha_bits
                    >(p_src.getAlpha()),
                };
            }(std::make_index_sequence<T_DstColor::num_color_channels>());
        }
    }


    /// convert from gs to 2+ color channels.
    template <ColorType T_DstColor, ColorType T_SrcColor>

    requires(
        T_DstColor::num_color_channels >= 2 &&
        T_SrcColor::num_color_channels == 1
    )
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        if constexpr (!T_DstColor::has_alpha_channel)
        {
            return [&]<std::size_t... t_i>(std::index_sequence<t_i...>)
            {
                return T_DstColor{
                    (utils::resizeBits<
                        T_DstColor::template color_bits<t_i>,
                        T_SrcColor::template color_bits<0>
                    >(p_src.template getColor<0>()))...,
                };
            }(std::make_index_sequence<T_DstColor::num_color_channels>());
        }

        else if (!T_SrcColor::has_alpha_channel)
        {
            return [&]<std::size_t... t_i>(std::index_sequence<t_i...>)
            {
                return T_DstColor{
                    (utils::resizeBits<
                        T_DstColor::template color_bits<t_i>,
                        T_SrcColor::template color_bits<0>
                    >(p_src.template getColor<0>()))...,
                    utils::bits<T_DstColor::alpha_bits>,
                };
            }(std::make_index_sequence<T_DstColor::num_color_channels>());
        }

        else if (T_SrcColor::has_alpha_channels)
        {
            return [&]<std::size_t... t_i>(std::index_sequence<t_i...>)
            {
                return T_DstColor{
                    (utils::resizeBits<
                        T_DstColor::template color_bits<t_i>,
                        T_SrcColor::template color_bits<0>
                    >(p_src.template getColor<0>()))...,
                    utils::resizeBits<
                        T_DstColor::alpha_bits,
                        T_SrcColor::alpha_bits
                    >(p_src.getAlpha()),
                };
            }(std::make_index_sequence<T_DstColor::num_color_channels>());
        }
    }
    
    /// convert from rgb to gs.
    /// uses interp0 lane 0.
    /// TODO: this should work for color formats with a different order
    template<ColorType T_DstColor, ColorType T_SrcColor>
    requires(
        T_DstColor::num_color_channels == 1 &&
        T_SrcColor::num_color_channels == 3
    )
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        auto config = interp_default_config();
        interp_config_set_blend(&config, true);
        interp_set_config(interp0, 0, &config);

        interp0->accum[1] = utils::bits<8> * 3 / 10;
        interp0->base01 = p_src.template get<0>();
        interp0->accum[1] = utils::bits<8> * 59 / 100;
        interp0->base01 = interp0->peek[1] << 16 | p_src.template get<1>();
        interp0->accum[1] = utils::bits<8> * 11 / 100;
        interp0->base01 = interp0->peek[1] << 16 | p_src.template get<2>();

        if constexpr (!T_DstColor::has_alpha_channel)
        {
            return {
                utils::resizeBits<
                    T_DstColor::template color_bits<0>,
                    T_SrcColor::template color_bits<0>
                >(interp0->pop[1]),
            };
        }

        else if constexpr (!T_SrcColor::has_alpha_channel)
        {
            return {
                utils::resizeBits<
                    T_DstColor::template color_bits<0>,
                    T_SrcColor::template color_bits<0>
                >(interp0->pop[1]),
                utils::bits<T_DstColor::alpha_bits>,
            };
        }

        else if constexpr (T_SrcColor::has_alpha_channel)
        {
            return {
                utils::resizeBits<
                    T_DstColor::template color_bits<0>,
                    T_SrcColor::template color_bits<0>
                >(interp0->pop[1]),
                utils::resizeBits<
                    T_DstColor::alpha_bits,
                    T_SrcColor::alpha_bits
                >(p_src.getAlpha()()),
            };
        }
    }

}