#pragma once

#include <cstddef>
#include <utility>

namespace picon::utils
{
    /// bits from 0 to t_num_bits set to 1.
    template <std::size_t t_num_bits>
    constexpr std::size_t bits = []<std::size_t ...I>(std::index_sequence<I...>)
    {
        return ((1 << I) | ... | 0);
    }(std::make_index_sequence<t_num_bits>());

    
    /// set t_num_bits to p_n starting at t_index.
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

} // napespace picon::utils