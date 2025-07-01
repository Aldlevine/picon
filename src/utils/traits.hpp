#pragma once

#include <concepts>

namespace picon::utils
{

    namespace internal
    {
        template <typename... T_Rest>
        struct unique_types: public std::false_type {};

        template <typename T_Type>
        struct unique_types<T_Type>: public std::true_type {};

        template <typename T_Type, typename... T_Rest>
        requires (!(std::same_as<T_Type, T_Rest> || ... || false) && unique_types<T_Rest...>::value)
        struct unique_types<T_Type, T_Rest...>: public std::true_type {};
    } // detail

    /// whether or not the parameter pack `T_Types...` contains only unique types.
    /// @tparam T_Types
    template <typename... T_Types>
    constexpr bool unique_types = internal::unique_types<T_Types...>::value;
} // picon:: utils