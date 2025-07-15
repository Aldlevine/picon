#pragma once

#include "graphics/color.hpp"
#include "graphics/convert_custom.hpp"

namespace picon::graphics::color
{
    template <ColorType T_DstColor, ColorType T_SrcColor>
    using ConvertLut = std::array<T_DstColor, utils::bits<sizeof(typename T_SrcColor::Value) * CHAR_BIT> + 1>;
    
    #define MAKE_CONVERT_LUT(M_DstColor, M_SrcColor) \
    template <> \
    struct convert_custom<M_DstColor, M_SrcColor>: std::true_type \
    { \
        static const ConvertLut<M_DstColor, M_SrcColor> lut; \
        static M_DstColor operator()(M_SrcColor p_src){ return lut[p_src.value]; }\
    }

    MAKE_CONVERT_LUT(GS4, R5G6B5);
    MAKE_CONVERT_LUT(GS4, R5G5B5A1);
    MAKE_CONVERT_LUT(GS4, R4G4B4A4);

    #undef MAKE_CONVERT_LUT

} // namespace picon::graphics::color