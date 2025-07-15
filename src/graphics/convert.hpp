#pragma once

#include "color.hpp"
#include "convert_custom.hpp" 

#include "utils/bit_utils.hpp"
#include <concepts>

#ifdef PICON_PLATFORM_PICO
#include <hardware/interp.h>
#endif

#include <cstdint>
#include <cstddef>


namespace picon::graphics::color
{
    /// the number on non-alpha color channels in a given color.
    template <ColorType T_Color>
    inline constexpr std::size_t num_color_channels =
        T_Color::num_channels - T_Color::template has_channel<A>;

    static_assert(num_color_channels<GS4> == 1);
    static_assert(num_color_channels<GS4A1> == 1);
    static_assert(num_color_channels<R5G6B5> == 3);
    static_assert(num_color_channels<R5G5B5A1> == 3);

    /// whether two given colors have the same set of color channels.
    /// channels can be in any order and any size.
    /// neither, one, or both colors may have alpha channel.
    template <ColorType T_DstColor, ColorType T_SrcColor>
    inline constexpr bool same_set_color_channels =
        num_color_channels<T_DstColor> == num_color_channels<T_SrcColor> &&
        []<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>){
            return ((
                ChannelOfType<A, decltype(t_channels)> ||
                T_SrcColor::template has_channel<decltype(t_channels)>
            ) && ... && true);
        }(T_DstColor{});

    static_assert(same_set_color_channels<GS4, Color<std::uint16_t, L{16}>>);
    static_assert(!same_set_color_channels<GS4, Color<std::uint16_t, R{16}>>);

    static_assert(same_set_color_channels<GS4, GS4A1>);
    static_assert(!same_set_color_channels<GS4, R5G6B5>);
    static_assert(!same_set_color_channels<GS4, R5G5B5A1>);

    static_assert(same_set_color_channels<GS4A1, GS4>);
    static_assert(!same_set_color_channels<GS4A1, R5G6B5>);
    static_assert(!same_set_color_channels<GS4A1, R5G5B5A1>);

    static_assert(same_set_color_channels<R5G6B5, R5G6B5>);
    static_assert(!same_set_color_channels<R5G6B5, GS4>);
    static_assert(!same_set_color_channels<R5G6B5, GS4A1>);

    static_assert(same_set_color_channels<R5G5B5A1, R5G6B5>);
    static_assert(!same_set_color_channels<R5G5B5A1, GS4>);
    static_assert(!same_set_color_channels<R5G5B5A1, GS4A1>);

    static_assert(same_set_color_channels<R5G5B5A1, Color<std::uint32_t, A{8}, B{8}, G{8}, R{8}>>);

    /// use as 3rd template parameter to convert<> to signal bypass of custom_convert.
    struct BypassCustom {};

    /// matches BypassCustom, used as a signal to bypass custom_convert.
    /// should be included on all builtin / default convert functions.
    template <typename T_Type>
    concept BypassCustomType = std::same_as<BypassCustom, T_Type>;

    /// convert between colors.
    template <ColorType T_DstColor, ColorType T_SrcColor>
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        if constexpr (convert_custom<T_DstColor, T_SrcColor>::value)
        {
            return convert_custom<T_DstColor, T_SrcColor>::operator()(p_src);
        }
        else
        {
            return convert<T_DstColor, T_SrcColor, BypassCustom>(p_src);
        }
    }

    
    /// convert between colors with the same set of color channels.
    template<ColorType T_DstColor, ColorType T_SrcColor, BypassCustomType>
    requires(same_set_color_channels<T_DstColor, T_SrcColor>)
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        if constexpr (T_DstColor::template has_channel<A> && !T_SrcColor::template has_channel<A>)
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>) -> T_DstColor
            {
                return {
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return utils::bits<t_channels.size>;
                        }
                        else
                        {
                            return utils::resizeBits<
                                T_DstColor::template channel<decltype(t_channels)>.size,
                                T_SrcColor::template channel<decltype(t_channels)>.size
                            >(static_cast<T_DstColor::Value>(p_src.template get<decltype(t_channels)>()));
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }
        else 
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>) -> T_DstColor
            {
                return {
                    (utils::resizeBits<
                        T_DstColor::template channel<decltype(t_channels)>.size,
                        T_SrcColor::template channel<decltype(t_channels)>.size
                    >(static_cast<T_DstColor::Value>(p_src.template get<decltype(t_channels)>())))
                    ...,
                };
            }(T_DstColor{});
        }
    }
    
    // same colors, from A to Non-A
    static_assert(convert<GS4, GS4A1, BypassCustom>({3, 1}).get<L>() == 3);
    static_assert(convert<R5G6B5, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<R>() == 3);
    static_assert(convert<R5G6B5, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<G>() == 8);
    static_assert(convert<R5G6B5, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<B>() == 5);

    // same colors, from A to A
    static_assert(convert<GS4A1, GS4A1, BypassCustom>({3, 1}).get<L>() == 3);
    static_assert(convert<GS4A1, GS4A1, BypassCustom>({3, 1}).get<A>() == 1);
    static_assert(convert<R5G5B5A1, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<R>() == 3);
    static_assert(convert<R5G5B5A1, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<G>() == 4);
    static_assert(convert<R5G5B5A1, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<B>() == 5);
    static_assert(convert<R5G5B5A1, R5G5B5A1, BypassCustom>({3, 4, 5, 1}).get<A>() == 1);

    // same colors, from Non-A to A
    static_assert(convert<GS4A1, GS4, BypassCustom>({3}).get<L>() == 3);
    static_assert(convert<GS4A1, GS4, BypassCustom>({3}).get<A>() == 1);
    static_assert(convert<R5G5B5A1, R5G6B5, BypassCustom>({3, 4, 5}).get<R>() == 3);
    static_assert(convert<R5G5B5A1, R5G6B5, BypassCustom>({3, 4, 5}).get<G>() == 2);
    static_assert(convert<R5G5B5A1, R5G6B5, BypassCustom>({3, 4, 5}).get<B>() == 5);
    static_assert(convert<R5G5B5A1, R5G6B5, BypassCustom>({3, 4, 5}).get<A>() == 1);

    /// convert from L(A) to RGB(A)
    // template<ColorType T_DstColor, ColorType T_SrcColor>
    template<ColorType T_DstColor, ColorType T_SrcColor, BypassCustomType>
    requires(
        num_color_channels<T_DstColor> == 3 &&
        T_DstColor::template has_channel<R> &&
        T_DstColor::template has_channel<G> &&
        T_DstColor::template has_channel<B> &&
        num_color_channels<T_SrcColor> == 1 &&
        T_SrcColor::template has_channel<L>
    )
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        if constexpr (T_DstColor::template has_channel<A> && !T_SrcColor::template has_channel<A>)
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>)
            {
                return T_DstColor{
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return utils::bits<t_channels.size>;
                        }
                        else
                        {
                            return utils::resizeBits<
                                T_DstColor::template channel<decltype(t_channels)>.size,
                                T_SrcColor::template channel<L>.size
                            >(static_cast<T_DstColor::Value>(p_src.template get<L>()));
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }
        else
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>)
            {
                return T_DstColor{
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return utils::resizeBits<
                                T_DstColor::template channel<decltype(t_channels)>.size,
                                T_SrcColor::template channel<decltype(t_channels)>.size
                            >(static_cast<T_DstColor::Value>(p_src.template get<decltype(t_channels)>()));
                        }
                        else
                        {
                            return utils::resizeBits<
                                T_DstColor::template channel<decltype(t_channels)>.size,
                                T_SrcColor::template channel<L>.size
                            >(static_cast<T_DstColor::Value>(p_src.template get<L>()));
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }

    }

    // L to RGB
    static_assert(convert<R5G6B5, GS4, BypassCustom>({0}).get<R>() == 0);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({0}).get<G>() == 0);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({0}).get<B>() == 0);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({2}).get<R>() == 4);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({2}).get<G>() == 8);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({2}).get<B>() == 4);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({4}).get<R>() == 8);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({4}).get<G>() == 17);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({4}).get<B>() == 8);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({6}).get<R>() == 12);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({6}).get<G>() == 25);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({6}).get<B>() == 12);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({8}).get<R>() == 17);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({8}).get<G>() == 34);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({8}).get<B>() == 17);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({10}).get<R>() == 21);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({10}).get<G>() == 42);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({10}).get<B>() == 21);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({12}).get<R>() == 25);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({12}).get<G>() == 51);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({12}).get<B>() == 25);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({14}).get<R>() == 29);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({14}).get<G>() == 59);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({14}).get<B>() == 29);

    static_assert(convert<R5G6B5, GS4, BypassCustom>({15}).get<R>() == 31);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({15}).get<G>() == 63);
    static_assert(convert<R5G6B5, GS4, BypassCustom>({15}).get<B>() == 31);

    
    /// convert from RGB(A) to L(A).
    /// follows ITU Rec. 601.
    /// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.601-7-201103-I!!PDF-E.pdf : Section 2.5.1.
    template<ColorType T_DstColor, ColorType T_SrcColor, BypassCustomType>
    requires(
        num_color_channels<T_DstColor> == 1 &&
        T_DstColor::template has_channel<L> &&
        num_color_channels<T_SrcColor> == 3 &&
        T_SrcColor::template has_channel<R> &&
        T_SrcColor::template has_channel<G> &&
        T_SrcColor::template has_channel<B>
    )
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        using MaxType = std::uintmax_t;
        const auto max_bits = sizeof(MaxType);
        const auto r = utils::resizeBits<max_bits, T_SrcColor::template channel<R>.size>(static_cast<MaxType>(p_src.template get<R>()));
        const auto g = utils::resizeBits<max_bits, T_SrcColor::template channel<G>.size>(static_cast<MaxType>(p_src.template get<G>()));
        const auto b = utils::resizeBits<max_bits, T_SrcColor::template channel<B>.size>(static_cast<MaxType>(p_src.template get<B>()));

        const typename T_DstColor::Value l_value =  utils::resizeBits<T_DstColor::template channel<L>.size, max_bits>(
            ((r * 299) + (g * 587) + (b * 114)) / 1000
        );

        if constexpr (T_DstColor::template has_channel<A> && !T_SrcColor::template has_channel<A>)
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>) -> T_DstColor
            {
                return {
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return static_cast<T_DstColor::Value>(utils::bits<t_channels.size>);
                        }
                        else if constexpr (ChannelOfType<L, decltype(t_channels)>)
                        {
                            return l_value;
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }
        else
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>) -> T_DstColor
            {
                return {
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return utils::resizeBits<
                                T_DstColor::template channel<decltype(t_channels)>.size,
                                T_SrcColor::template channel<decltype(t_channels)>.size
                            >(static_cast<T_DstColor::Value>(p_src.template get<decltype(t_channels)>()));
                        }
                        else if constexpr (ChannelOfType<L, decltype(t_channels)>)
                        {
                            return l_value;
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }
    }


    #if 0 // PICON CONVERT USE INTERP

    /// convert from RGB(A) to L(A).
    /// follows ITU Rec. 601.
    /// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.601-7-201103-I!!PDF-E.pdf : Section 2.5.1.
    /// on rpi pico uses interp0 lane0 for blending color channels.
    template<ColorType T_DstColor, ColorType T_SrcColor, BypassCustomType>
    requires(
        num_color_channels<T_DstColor> == 1 &&
        T_DstColor::template has_channel<L> &&
        num_color_channels<T_SrcColor> == 3 &&
        T_SrcColor::template has_channel<R> &&
        T_SrcColor::template has_channel<G> &&
        T_SrcColor::template has_channel<B> &&
        T_SrcColor::template channel<R>.size <= 16 &&
        T_SrcColor::template channel<G>.size <= 16 &&
        T_SrcColor::template channel<B>.size <= 16
    )
    inline constexpr T_DstColor convert(T_SrcColor p_src)
    {
        constexpr std::uint8_t r_coeff = utils::bits<8> * 299 / 1000;
        constexpr std::uint8_t g_coeff = utils::bits<8> * 587 / 1000;
        constexpr std::uint8_t b_coeff = utils::bits<8> * 114 / 1000;

        typename T_SrcColor::Value l_value{};

        #ifdef PICON_PLATFORM_PICO
            constexpr auto has_interp = true;
        #else
            constexpr auto has_interp = false;
        #endif

        // if constexpr (std::is_constant_evaluated() || !has_interp)
        if constexpr (true)
        {
            const auto r = utils::resizeBits<16, T_SrcColor::template channel<R>.size>(p_src.template get<R>());
            const auto g = utils::resizeBits<16, T_SrcColor::template channel<G>.size>(p_src.template get<G>());
            const auto b = utils::resizeBits<16, T_SrcColor::template channel<B>.size>(p_src.template get<B>());

            const auto r_w = r * r_coeff / utils::bits<8>;
            const auto g_w = g * g_coeff / utils::bits<8>;
            const auto b_w = b * b_coeff / utils::bits<8>;

            l_value = utils::resizeBits<T_DstColor::template channel<L>.size, 16>(r_w + g_w + b_w);
        }
        else
        {
            #if defined(PICON_PLATFORM_PICO)
                auto config = interp_default_config();
                interp_set_config(interp0, 1, &config);
                interp_config_set_blend(&config, true);
                interp_set_config(interp0, 0, &config);

                if constexpr (
                    p_src.template channel<R>.size <= 16 &&
                    p_src.template channel<G>.size <= 16 &&
                    p_src.template channel<B>.size <= 16
                ){
                    interp0->base[0] = 0;

                    interp0->accum[1] = r_coeff;
                    interp0->base[1] =
                        utils::resizeBits<16, p_src.template channel<R>.size>(p_src.template get<R>());
                    l_value = interp0->peek[1];

                    interp0->accum[1] = g_coeff;
                    interp0->base[1] =
                        utils::resizeBits<16, p_src.template channel<G>.size>(p_src.template get<G>());
                    l_value += interp0->peek[1];

                    interp0->accum[1] = b_coeff;
                    interp0->base[1] =
                        utils::resizeBits<16, p_src.template channel<B>.size>(p_src.template get<B>());
                    l_value += interp0->peek[1];

                    l_value = utils::resizeBits<T_DstColor::template channel<L>.size, 16>(l_value);
                }
            #endif
        }

        if constexpr (T_DstColor::template has_channel<A> && !T_SrcColor::template has_channel<A>)
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>) -> T_DstColor
            {
                return {
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return static_cast<T_DstColor::Value>(utils::bits<t_channels.size>);
                        }
                        else if constexpr (ChannelOfType<L, decltype(t_channels)>)
                        {
                            return l_value;
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }
        else
        {
            return [&]<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>) -> T_DstColor
            {
                return {
                    [&](){
                        if constexpr (ChannelOfType<A, decltype(t_channels)>)
                        {
                            return utils::resizeBits<
                                T_DstColor::template channel<decltype(t_channels)>.size,
                                T_SrcColor::template channel<decltype(t_channels)>.size
                            >(static_cast<T_DstColor::Value>(p_src.template get<decltype(t_channels)>()));
                        }
                        else if constexpr (ChannelOfType<L, decltype(t_channels)>)
                        {
                            return l_value;
                        }
                    }()
                    ...,
                };
            }(T_DstColor{});
        }

    }
    #endif // PICON CONVERT USE INTERP

    // RGB to L
    static_assert(convert<GS4, R5G6B5, BypassCustom>({0, 0, 0}).get<L>() == 0);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({4, 8, 4}).get<L>() == 2);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({8, 16, 8}).get<L>() == 4);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({12, 24, 12}).get<L>() == 6);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({16, 32, 16}).get<L>() == 8);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({20, 40, 20}).get<L>() == 10);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({24, 48, 24}).get<L>() == 12);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({28, 56, 28}).get<L>() == 14);
    static_assert(convert<GS4, R5G6B5, BypassCustom>({31, 63, 31}).get<L>() == 15);

    // RGBA to L
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({0, 0, 0, 1}).get<L>() == 0);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({4, 4, 4, 1}).get<L>() == 2);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({8, 8, 8, 1}).get<L>() == 4);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({12, 12, 12, 1}).get<L>() == 6);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({16, 16, 16, 1}).get<L>() == 8);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({20, 20, 20, 1}).get<L>() == 10);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({24, 24, 24, 1}).get<L>() == 12);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({28, 28, 28, 1}).get<L>() == 14);
    static_assert(convert<GS4, R5G5B5A1, BypassCustom>({31, 31, 31, 1}).get<L>() == 15);

    // RGB to LA
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({0, 0, 0}).get<L>() == 0);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({4, 8, 4}).get<L>() == 2);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({8, 16, 8}).get<L>() == 4);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({12, 24, 12}).get<L>() == 6);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({16, 32, 16}).get<L>() == 8);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({20, 40, 20}).get<L>() == 10);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({24, 48, 24}).get<L>() == 12);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({28, 56, 28}).get<L>() == 14);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({31, 63, 31}).get<L>() == 15);
    
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({0, 0, 0}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({4, 8, 4}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({8, 16, 8}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({12, 24, 12}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({16, 32, 16}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({20, 40, 20}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({24, 48, 24}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({28, 56, 28}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G6B5, BypassCustom>({31, 63, 31}).get<A>() == 1);

    // RGBA to LA
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({0, 0, 0, 1}).get<L>() == 0);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({4, 4, 4, 1}).get<L>() == 2);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({8, 8, 8, 1}).get<L>() == 4);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({12, 12, 12, 1}).get<L>() == 6);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({16, 16, 16, 1}).get<L>() == 8);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({20, 20, 20, 1}).get<L>() == 10);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({24, 24, 24, 1}).get<L>() == 12);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({28, 28, 28, 1}).get<L>() == 14);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({31, 31, 31, 1}).get<L>() == 15);

    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({0, 0, 0, 0}).get<A>() == 0);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({4, 4, 4, 1}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({8, 8, 8, 0}).get<A>() == 0);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({12, 12, 12, 1}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({16, 16, 16, 0}).get<A>() == 0);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({20, 20, 20, 1}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({24, 24, 24, 0}).get<A>() == 0);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({28, 28, 28, 1}).get<A>() == 1);
    static_assert(convert<GS4A1, R5G5B5A1, BypassCustom>({31, 31, 31, 0}).get<A>() == 0);
} // namespace picon::graphics::color