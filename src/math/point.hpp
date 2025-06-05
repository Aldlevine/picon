#pragma once

namespace pg
{

    template <typename T_Unit>
    struct Point
    {
        T_Unit x{};
        T_Unit y{};
        
        #define BIN_OP(p_op) \
        friend constexpr Point operator p_op(const Point& p_lhs, const Point& p_rhs) \
        { return {p_lhs.x p_op p_rhs.x, p_lhs.y p_op p_rhs.y}; } \
        friend constexpr Point operator p_op(const Point& p_lhs, const T_Unit& p_rhs) \
        { return {p_lhs.x p_op p_rhs, p_lhs.y p_op p_rhs}; }
        BIN_OP(+)
        BIN_OP(-)
        BIN_OP(*)
        BIN_OP(/)
        BIN_OP(%)
        BIN_OP(|)
        BIN_OP(&)
        BIN_OP(<<)
        BIN_OP(>>)
        #undef BIN_OP
        
        #define BIN_BOOL_OP(p_op) \
        friend constexpr Point<bool> operator p_op(const Point& p_lhs, const Point& p_rhs) \
        { return {p_lhs.x p_op p_rhs.x, p_lhs.y p_op p_rhs.y}; } \
        friend constexpr Point<bool> operator p_op(const Point& p_lhs, const T_Unit& p_rhs) \
        { return {p_lhs.x p_op p_rhs, p_lhs.y p_op p_rhs}; }
        BIN_BOOL_OP(<)
        BIN_BOOL_OP(<=)
        BIN_BOOL_OP(==)
        BIN_BOOL_OP(>=)
        BIN_BOOL_OP(>)
        BIN_BOOL_OP(!=)
        BIN_BOOL_OP(&&)
        BIN_BOOL_OP(||)
        #undef BIN_BOOL_OP
    };

} // namespace pg