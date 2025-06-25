#pragma once

#include "utils/bit_utils.hpp"

#include <hardware/interp.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace picon::graphics::color
{
    
    /// Color type which holds the configured color channels in a single T_Value.
    template <typename T_Value, std::array t_channels, std::intmax_t t_alpha_channel = -1>
    struct Color
    {
        /// channels and their sizes.
        static constexpr auto channels{t_channels};

        /// which channel is alpha (-1 if none).
        static constexpr auto alpha_channel{t_alpha_channel};

        /// number of alpha channels (0 or 1).
        static constexpr auto has_alpha_channel = alpha_channel >= 0;

        /// number of channels.
        static constexpr auto num_channels = channels.size();

        /// number of color channels.
        static constexpr auto num_color_channels = num_channels - has_alpha_channel;

        /// number of bits in alpha channel.
        static constexpr auto alpha_bits = alpha_channel == -1 ? 0 : channels[alpha_channel];

        /// which raw channel is color channel at index
        template <std::size_t t_channel>
        static constexpr auto color_channel =
            has_alpha_channel && t_channel > alpha_channel ?
                t_channel + 1 :
                t_channel;
        
        /// size of color channel
        template <std::size_t t_channel>
        static constexpr auto color_bits = channels[color_channel<t_channel>];

        /// bit offset of given channel within value.
        template <std::size_t t_channel>
        static constexpr auto channel_offset = [](){
            static_assert(t_channel <= num_channels);

            std::size_t result{0};
            for (std::size_t i = num_channels; i > t_channel; --i)
            {
                result += channels[i - 1];
            }
            return result;
        }();

        /// underlying value.
        T_Value value{};

        /// default construct to 0.
        constexpr Color() = default;

        // construct with all components.
        template <typename... T_Args>
        requires(sizeof...(T_Args) == num_channels)
        constexpr Color(T_Args... p_args)
        {
            value = [&]<std::size_t... t_i>(std::index_sequence<t_i...>) {
                return (utils::setBits<channel_offset<t_i>, channels[t_i]>(p_args) | ... | 0);
            }(std::make_index_sequence<sizeof...(T_Args)>());
        }

        /// get a given channel.
        template <std::size_t t_channel>
        constexpr auto get() const
        {
            return utils::getBits<channel_offset<t_channel>, channels[t_channel]>(value);
        }

        /// get a given color channel.
        template <std::size_t t_channel>
        constexpr auto getColor() const
        {
            return get<color_channel<t_channel>>();
        }

        /// get alpha channel.
        constexpr auto getAlpha() const
        {
            return get<alpha_channel>();
        }
        
        /// convert to underlying value.
        constexpr operator T_Value() const { return value; }
    };

    /// is T_Type a Color<...>
    template <typename T_Type>
    struct is_color_type : std::false_type {};

    /// is const Color<...> a Color<...>
    template <typename T_Value, std::array t_channels, std::intmax_t t_alpha_channel>
    struct is_color_type<const Color<T_Value, t_channels, t_alpha_channel>>: std::true_type {};

    /// is Color<...> a Color<...>
    template <typename T_Value, std::array t_channels, std::intmax_t t_alpha_channel>
    struct is_color_type<Color<T_Value, t_channels, t_alpha_channel>>: std::true_type {};
    
    /// concept matching any Color<...>.
    template <typename T_Type>
    concept ColorType = is_color_type<T_Type>::value;

    /// grayscale 4 bit, stored on LSB of 8 bits.
    using GS4 = Color<std::uint8_t, {4u}>;
    /// grayscale 4 bit, alpha 1 bit, stored on LSB of 8 bits.
    using GS4A1 = Color<std::uint8_t, {4u, 1u}, 1>;
    /// red 5 bit, green 6 bit, blue 5 bit.
    using R5G6B5 =  Color<std::uint16_t, {5u, 6u, 5u}>;
    /// red 5 bit, green 5 bit, blue 5 bit, alpha 1 bit.
    using R5G5B5A1 = Color<std::uint16_t, {5u, 5u, 5u, 1u}, 3>;

} // namespace picon::graphics