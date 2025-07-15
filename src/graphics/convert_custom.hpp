#pragma once

#include <type_traits>

namespace picon::graphics::color
{
    template <typename T_DstColor, typename T_SrcColor>
    struct convert_custom : std::false_type{};
}
