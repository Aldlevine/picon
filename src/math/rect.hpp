#pragma once

#include "point.hpp"
#include <algorithm>
#include <optional>

namespace picon::math
{

    template <typename T_Unit>
    struct Rect
    {
        Point<T_Unit> position{};
        Point<T_Unit> size{};

        T_Unit area()
        {
            return size.x * size.y;
        }

        Rect normalized()
        {
            if (size.x < 0)
            {
                position.x += size.x;
                size.x = -size.x;
            }

            if (size.y < 0)
            {
                position.y += size.y;
                size.y = -size.y;
            }
        }

        std::optional<Rect> intersect(Rect p_rhs)
        {
            const auto x = std::max(position.x, p_rhs.position.x);
            const auto end_x = std::min(position.x + size.x, p_rhs.position.x + p_rhs.size.x);
            if (end_x < x) { return std::nullopt; }

            const auto y = std::max(position.y, p_rhs.position.y);
            const auto end_y = std::min(position.y + size.y, p_rhs.position.y + p_rhs.size.y);
            if (end_y < y) { return std::nullopt; }

            return {{x, y}, {end_x - x, end_y - y}};
        }
    };

} // namespace picon::math