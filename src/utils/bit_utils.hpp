#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

namespace picon::utils
{
    /// bits from 0 to t_num_bits set to 1.
    /// @tparam t_num_bits
    template <std::size_t t_num_bits>
    constexpr std::size_t bits = []<std::size_t ...I>(std::index_sequence<I...>)
    {
        return ((1 << I) | ... | 0);
    }(std::make_index_sequence<t_num_bits>());

    /// set t_num_bits to p_n starting at t_index.
    /// @tparam t_index
    /// @tparam t_num_bits
    template <std::size_t t_index, std::size_t t_num_bits>
    constexpr auto setBits(auto p_n) -> decltype(p_n)
    {
        return (p_n & bits<t_num_bits>) << (t_index - t_num_bits);
    }

    /// get t_num_bits from p_n starting at t_index.
    /// @tparam t_index
    /// @tparam t_num_bits
    template <std::size_t t_index, std::size_t t_num_bits>
    constexpr auto getBits(auto p_n) -> decltype(p_n)
    {
        return (p_n >> (t_index - t_num_bits)) & bits<t_num_bits>;
    }

    /// repeat t_src_bits from MSB to fill t_dst_bits.
    /// @tparam t_dst_bits
    /// @tparam t_src_bits
    template <std::size_t t_dst_bits, std::size_t t_src_bits>
    constexpr auto repeatBits(auto p_n) -> decltype(p_n)
    {

        static_assert(t_dst_bits > t_src_bits);
        
        constexpr auto new_bits = t_dst_bits - t_src_bits;
        constexpr auto whole_repeats = new_bits / t_src_bits;
        constexpr auto mod_bits = new_bits % t_src_bits;
        constexpr auto repeat_bits = std::min(t_src_bits, new_bits);

        decltype(p_n) new_whole = 0;

        for (std::size_t i = 0; i < whole_repeats; ++i)
        {
            const auto base = p_n >> (t_src_bits - repeat_bits);
            new_whole |= base << ((repeat_bits * i) + mod_bits);
        }

        if constexpr (mod_bits > 0)
        {
            const auto new_mod = (p_n) >> (t_src_bits - mod_bits);
            return (p_n << new_bits) | new_whole | new_mod;
        }
        else
        {
            return (p_n << new_bits) | new_whole;
        }
    }

    /// determines if `resizeBits` should use lookup table
    /// @tparam t_dst_bits
    /// @tparam t_src_bits
    template <std::size_t t_dst_bits, std::size_t t_src_bits>
    constexpr bool resize_bits_use_lut{true};

    /// resizes t_src_bits to fill t_dst_bits.
    /// reduction truncates, expansion evenly distributes and correctly saturates.
    /// @tparam t_dst_bits
    /// @tparam t_src_bits
    template <std::size_t t_dst_bits, std::size_t t_src_bits>
    constexpr auto resizeBits(auto p_n) -> decltype(p_n)
    {
        if constexpr (t_dst_bits == t_src_bits)
        {
            return p_n;
        }
        else if constexpr(t_dst_bits < t_src_bits)
        {
            return p_n >> (t_src_bits - t_dst_bits);
        }
        else if constexpr(t_dst_bits > t_src_bits)
        {
            if constexpr (resize_bits_use_lut<t_dst_bits, t_src_bits>)
            {
                static constexpr auto lut{[]<std::size_t... t_i>(std::index_sequence<t_i...>){
                    return std::array {
                        repeatBits<t_dst_bits, t_src_bits>(static_cast<decltype(p_n)>(t_i))...
                    };
                }(std::make_index_sequence<bits<t_src_bits> + 1>())};

                return lut[p_n];
            }
            else
            {
                return repeatBits<t_dst_bits, t_src_bits>(p_n);
            }
        }
    }

    namespace test::resizeBits_
    {
        constexpr auto x = resizeBits<3, 2>(0b11);

        // 2 to 3
        static_assert(resizeBits<3, 2>(0b00) == 0b000);
        static_assert(resizeBits<3, 2>(0b01) == 0b010);
        static_assert(resizeBits<3, 2>(0b10) == 0b101);
        static_assert(resizeBits<3, 2>(0b11) == 0b111);

        // 2 to 4
        static_assert(resizeBits<4, 2>(0b00) == 0b0000);
        static_assert(resizeBits<4, 2>(0b01) == 0b0101);
        static_assert(resizeBits<4, 2>(0b10) == 0b1010);
        static_assert(resizeBits<4, 2>(0b11) == 0b1111);

        // 2 to 5
        static_assert(resizeBits<5, 2>(0b00) == 0b00000);
        static_assert(resizeBits<5, 2>(0b01) == 0b01010);
        static_assert(resizeBits<5, 2>(0b10) == 0b10101);
        static_assert(resizeBits<5, 2>(0b11) == 0b11111);

        // 2 to 6
        static_assert(resizeBits<6, 2>(0b00) == 0b000000);
        static_assert(resizeBits<6, 2>(0b01) == 0b010101);
        static_assert(resizeBits<6, 2>(0b10) == 0b101010);
        static_assert(resizeBits<6, 2>(0b11) == 0b111111);

        // 2 to 7
        static_assert(resizeBits<7, 2>(0b00) == 0b0000000);
        static_assert(resizeBits<7, 2>(0b01) == 0b0101010);
        static_assert(resizeBits<7, 2>(0b10) == 0b1010101);
        static_assert(resizeBits<7, 2>(0b11) == 0b1111111);

        // 2 to 8
        static_assert(resizeBits<8, 2>(0b00) == 0b00000000);
        static_assert(resizeBits<8, 2>(0b01) == 0b01010101);
        static_assert(resizeBits<8, 2>(0b10) == 0b10101010);
        static_assert(resizeBits<8, 2>(0b11) == 0b11111111);

        // 3 to 4
        static_assert(resizeBits<4, 3>(0b000) == 0b0000);
        static_assert(resizeBits<4, 3>(0b001) == 0b0010);
        static_assert(resizeBits<4, 3>(0b010) == 0b0100);
        static_assert(resizeBits<4, 3>(0b011) == 0b0110);
        static_assert(resizeBits<4, 3>(0b100) == 0b1001);
        static_assert(resizeBits<4, 3>(0b101) == 0b1011);
        static_assert(resizeBits<4, 3>(0b110) == 0b1101);
        static_assert(resizeBits<4, 3>(0b111) == 0b1111);

        // 3 to 5
        static_assert(resizeBits<5, 3>(0b000) == 0b00000);
        static_assert(resizeBits<5, 3>(0b001) == 0b00100);
        static_assert(resizeBits<5, 3>(0b010) == 0b01001);
        static_assert(resizeBits<5, 3>(0b011) == 0b01101);
        static_assert(resizeBits<5, 3>(0b100) == 0b10010);
        static_assert(resizeBits<5, 3>(0b101) == 0b10110);
        static_assert(resizeBits<5, 3>(0b110) == 0b11011);
        static_assert(resizeBits<5, 3>(0b111) == 0b11111);

        // 3 to 6
        static_assert(resizeBits<6, 3>(0b000) == 0b000000);
        static_assert(resizeBits<6, 3>(0b001) == 0b001001);
        static_assert(resizeBits<6, 3>(0b010) == 0b010010);
        static_assert(resizeBits<6, 3>(0b011) == 0b011011);
        static_assert(resizeBits<6, 3>(0b100) == 0b100100);
        static_assert(resizeBits<6, 3>(0b101) == 0b101101);
        static_assert(resizeBits<6, 3>(0b110) == 0b110110);
        static_assert(resizeBits<6, 3>(0b111) == 0b111111);

        // 3 to 7
        static_assert(resizeBits<7, 3>(0b000) == 0b0000000);
        static_assert(resizeBits<7, 3>(0b001) == 0b0010010);
        static_assert(resizeBits<7, 3>(0b010) == 0b0100100);
        static_assert(resizeBits<7, 3>(0b011) == 0b0110110);
        static_assert(resizeBits<7, 3>(0b100) == 0b1001001);
        static_assert(resizeBits<7, 3>(0b101) == 0b1011011);
        static_assert(resizeBits<7, 3>(0b110) == 0b1101101);
        static_assert(resizeBits<7, 3>(0b111) == 0b1111111);

        // 3 to 8
        static_assert(resizeBits<8, 3>(0b000) == 0b00000000);
        static_assert(resizeBits<8, 3>(0b001) == 0b00100100);
        static_assert(resizeBits<8, 3>(0b010) == 0b01001001);
        static_assert(resizeBits<8, 3>(0b011) == 0b01101101);
        static_assert(resizeBits<8, 3>(0b100) == 0b10010010);
        static_assert(resizeBits<8, 3>(0b101) == 0b10110110);
        static_assert(resizeBits<8, 3>(0b110) == 0b11011011);
        static_assert(resizeBits<8, 3>(0b111) == 0b11111111);
    } // namespace test

} // namespace picon::utils