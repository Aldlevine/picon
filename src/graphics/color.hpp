#pragma once

#include "utils/bit_utils.hpp"
#include "utils/traits.hpp"

#include <climits>
#include <cstdint>
#include <tuple>

namespace picon::graphics::color
{
    struct Channel
    {
        std::size_t size;
        std::size_t offset{0};
    };


    struct R: public Channel{};
    struct G: public Channel{};
    struct B: public Channel{};
    struct A: public Channel{};
    struct L: public Channel{};


    template <typename T_Type>
    concept ChannelType = std::is_base_of_v<Channel, std::remove_cvref_t<T_Type>>;

    template <typename T_Channel, typename T_Type>
    concept ChannelOfType = 
        std::is_base_of_v<Channel, std::remove_cvref_t<T_Channel>> &&
        std::is_base_of_v<T_Channel, std::remove_cvref_t<T_Type>>;

    template <auto... t_channels>
    concept AllValidChannels = (ChannelType<decltype(t_channels)> && ... && true);

    template <auto... t_channels>
    concept AllUniqueChannels = utils::unique_types<decltype(t_channels)...>;

    template <typename T_Value, auto... t_channels>
    concept ValueFitsChannels = ((t_channels.size + t_channels.offset) + ... + 0) <= sizeof(T_Value) * CHAR_BIT;


    template <typename T_Value, auto... t_channels>
    requires(
        AllValidChannels<t_channels...> &&
        AllUniqueChannels<t_channels...> &&
        ValueFitsChannels<T_Value, t_channels...>
    )
    struct Color
    {
        using Value = T_Value;

        T_Value value{};

        /// total number of channels.
        constexpr static std::size_t num_channels = sizeof...(t_channels);

        /// whether or not color has the given channel type.
        template <ChannelType T_Channel>
        constexpr static bool has_channel
        {
            (ChannelOfType<T_Channel, decltype(t_channels)> || ... || false)
        };

        /// index for the given channel type.
        template <ChannelType T_Channel>
        requires((has_channel<T_Channel>))
        constexpr static auto channel_index
        {
            []() -> std::size_t
            {
                constexpr std::array is_type{ ChannelOfType<T_Channel, decltype(t_channels)>... };
                for (std::size_t i = 0; i < is_type.size(); ++i)
                {
                    if (is_type[i]) { return i; }
                }
                return -1;
            }()
        };
        
        /// get channel at index.
        template <std::size_t t_index>
        requires((t_index < num_channels))
        constexpr static auto channel_at {
            []() -> std::tuple_element_t<t_index, std::tuple<decltype(t_channels)...>>
            {
                constexpr std::array channel_sizes{ t_channels.size ... };
                constexpr std::array channel_offsets{ t_channels.offset ... };
                std::size_t offset = [&]<std::size_t... t_i>(std::index_sequence<t_i...>){
                    return ((channel_sizes[t_i + t_index] + channel_offsets[t_i + t_index]) + ... + 0);
                }(std::make_index_sequence<num_channels - t_index>());

                constexpr auto raw_result = std::get<t_index>(std::tuple{t_channels...});
                return { raw_result.size, offset };
            }()
        };


        /// get channel for channel type.
        template <ChannelType T_Channel>
        requires((has_channel<T_Channel>))
        constexpr static auto channel
        {
            channel_at<channel_index<T_Channel>>
        };

        /// default construct to 0.
        Color() = default;

        operator T_Value() const { return value; }
        
        /// construct with all components.
        template <std::convertible_to<Value>... T_Args>
        requires(sizeof...(T_Args) == num_channels)
        constexpr Color(T_Args... p_args)
        {
            value =
                [&]<std::size_t... t_i>(std::index_sequence<t_i...>) {
                    return (utils::setBits<channel_at<t_i>.offset, channel_at<t_i>.size>(p_args) | ... | 0);
                }(std::index_sequence_for<T_Args...>{});
        }

        /// get value of given channel.
        template <ChannelType T_Channel>
        requires(has_channel<T_Channel>)
        constexpr T_Value get() const
        {
            return utils::getBits<channel<T_Channel>.offset, channel<T_Channel>.size>(value);
        }
    };


    /// concept matching any Color<...>.
    template <typename T_Type>
    concept ColorType = requires(T_Type& t)
    {
        { []<typename T_Value, auto... t_channels>(Color<T_Value, t_channels...>){}(t) };
    };

    
    using GS4 = Color<std::uint8_t, L{4}>;
    using GS4A1 = Color<std::uint8_t, L{4}, A{1}>;
    using R5G6B5 = Color<std::uint16_t, R{5}, G{6}, B{5}>;
    using R5G5B5A1 = Color<std::uint16_t, R{5}, G{5}, B{5}, A{1}>;


    static_assert(R5G5B5A1{1, 2, 3, 1}.get<R>() == 1);
    static_assert(R5G5B5A1{1, 2, 3, 1}.get<G>() == 2);
    static_assert(R5G5B5A1{1, 2, 3, 1}.get<B>() == 3);
    static_assert(R5G5B5A1{1, 2, 3, 1}.get<A>() == 1);
    
    static_assert(!R5G5B5A1::has_channel<L>);
    static_assert(R5G5B5A1::has_channel<R>);
    static_assert(R5G5B5A1::has_channel<G>);
    static_assert(R5G5B5A1::has_channel<B>);
    static_assert(R5G5B5A1::has_channel<A>);

    static_assert(R5G5B5A1::channel<R>.size == 5);
    static_assert(R5G5B5A1::channel<G>.size == 5);
    static_assert(R5G5B5A1::channel<B>.size == 5);
    static_assert(R5G5B5A1::channel<A>.size == 1);

    static_assert(R5G5B5A1::channel<R>.offset == 16);
    static_assert(R5G5B5A1::channel<G>.offset == 11);
    static_assert(R5G5B5A1::channel<B>.offset == 6);
    static_assert(R5G5B5A1::channel<A>.offset == 1);

    static_assert(GS4::has_channel<L>);
    static_assert(!GS4::has_channel<R>);
    static_assert(!GS4::has_channel<G>);
    static_assert(!GS4::has_channel<B>);
    static_assert(!GS4::has_channel<A>);

    static_assert(GS4::channel<L>.size == 4);
    static_assert(GS4::channel<L>.offset == 4);

} // namespace picon::graphics::color