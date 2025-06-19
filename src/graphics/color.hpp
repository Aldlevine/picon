#pragma once

#include "utils/bit_utils.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace picon::graphics::color
{
    struct GS4;
    struct GS4A1;
    struct R5G6B5;
    struct R5G5B5A1;
    
    struct Config
    {
        constexpr static std::size_t max_channels{4};

        std::array<std::size_t, max_channels> channel_size{};
        std::intmax_t alpha_channel{-1};
        
        constexpr std::size_t numChannels() const
        {
            for (std::size_t i = 0; i < max_channels; ++i)
            {
                if (channel_size[max_channels] == 0) { return i; }
            }
            return max_channels;
        }

        constexpr std::size_t alphaBits() const
        {
            if (alpha_channel < 0)
            {
                return 0;
            }
            else
            {
                return channel_size[alpha_channel];
            }
        }

        template <std::size_t t_channel>
        constexpr std::size_t channelOffset()
        {
            static_assert(t_channel <= numChannels());

            std::size_t result{0};
            for (std::size_t i = t_channel; i >= 0; --i)
            {
                result += channel_size[i];
            }
            return result;
        }
    };
    
    template <typename T_Value, Config t_config>
    struct ColorBase
    {
        static constexpr auto config = t_config;

        T_Value value{};

        template <std::size_t t_channel>
        constexpr auto get() const
        {
            return utils::getBits<config.channelOffset<t_channel>(), config.channel_size[t_channel]>(value);
        }
    };

    template <typename T_Type>
    concept Color = requires(T_Type t) {
        {[]<typename T_Value, Config t_config>(ColorBase<T_Value, t_config>){}(t)};
    };

    /// grayscale 4 bit, stored on LSB of 8 bits.
    struct GS4 : public ColorBase<std::uint8_t, {
        .channel_size = {4},
    }>
    {

        constexpr GS4() = default;

        constexpr GS4(std::uint8_t p_gs) : ColorBase{
            utils::setBits<4, 4>(p_gs)
        } {}

        constexpr operator std::uint8_t() const { return value; }
        constexpr operator GS4A1() const;

        constexpr auto gs() const { return utils::getBits<4, 4>(value); }
    };


    /// grayscale 4 bit, alpha 1 bit, stored on LSB of 8 bits.
    struct GS4A1 : public ColorBase<std::uint8_t, {
        .channel_size = {4, 1},
        .alpha_channel = 1,
    }>
    {
        constexpr GS4A1() = default;

        constexpr GS4A1(std::uint8_t p_gs, std::uint8_t p_a) : ColorBase{static_cast<uint8_t>(
            utils::setBits<5, 4>(p_gs) |
            utils::setBits<1, 1>(p_a)
        )}{}

        constexpr operator std::uint8_t() const { return value; }
        constexpr operator GS4() const;

        constexpr auto gs() const { return utils::getBits<5, 4>(value); }
        constexpr auto a() const { return utils::getBits<1, 1>(value); }
    };

    
    /// red 5 bit, green 6 bit, blue 5 bit.
    struct R5G6B5 : public ColorBase<std::uint16_t, {
        .channel_size = {5, 6, 5},
    }>
    {
        constexpr R5G6B5() = default;
        constexpr R5G6B5(std::uint16_t p_r, std::uint16_t p_g, std::uint16_t p_b) : ColorBase{static_cast<uint16_t>(
            utils::setBits<16, 5>(p_r)  |
            utils::setBits<11, 6>(p_g) |
            utils::setBits<5, 5>(p_b) 
        )}{}

        constexpr operator std::uint16_t() const { return value; }
        constexpr operator R5G5B5A1() const;

        constexpr auto r() const { return utils::getBits<16, 5>(value); }
        constexpr auto g() const { return utils::getBits<11, 6>(value); }
        constexpr auto b() const { return utils::getBits<5, 5>(value); }
    };


    /// red 5 bit, green 5 bit, blue 5 bit, alpha 1 bit.
    struct R5G5B5A1 : public ColorBase<std::uint16_t, {
        .channel_size = {5, 5, 5, 1},
        .alpha_channel = 3,
    }>
    {
        constexpr R5G5B5A1() = default;
        constexpr R5G5B5A1(std::uint16_t p_r, std::uint16_t p_g, std::uint16_t p_b, std::uint16_t p_a) : ColorBase{static_cast<uint16_t>(
            utils::setBits<16, 5>(p_r) |
            utils::setBits<11, 5>(p_g) |
            utils::setBits<6, 5>(p_b) |
            utils::setBits<1, 1>(p_a)
        )}{}

        constexpr operator std::uint16_t() const { return value; }
        constexpr operator R5G6B5() const;

        constexpr auto r() const { return utils::getBits<16, 5>(value); }
        constexpr auto g() const { return utils::getBits<11, 5>(value); }
        constexpr auto b() const { return utils::getBits<6, 5>(value); }
        constexpr auto a() const { return utils::getBits<1, 1>(value); }
    };
    
    // conversions

    //
    
    inline constexpr GS4::operator GS4A1() const
    {
        return {gs(), 1};
    }

    inline constexpr GS4A1::operator GS4() const
    {
        return {gs()};
    }
    
    inline constexpr R5G6B5::operator R5G5B5A1() const
    {
        return {
            r(),
            static_cast<uint16_t>((g() >> 1)),
            b(),
            1,
        };
    }

    inline constexpr R5G5B5A1::operator R5G6B5() const
    {
        return {
            r(),
            static_cast<uint16_t>((g() << 1) | utils::getBits<1, 1>(g())),
            b(),
        };
    }

} // namespace picon::graphics