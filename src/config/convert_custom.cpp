#include "convert_custom.hpp"

#include "graphics/color.hpp"
#include "graphics/convert.hpp"

namespace picon::graphics::color
{
    #define MAKE_CONVERT_LUT(M_DstColor, M_SrcColor) \
    const ConvertLut<M_DstColor, M_SrcColor> convert_custom<M_DstColor, M_SrcColor>::lut = \
        []<std::size_t... t_i>(std::index_sequence<t_i...>){ \
            return ConvertLut<M_DstColor, M_SrcColor>{ \
                convert<M_DstColor, M_SrcColor, BypassCustom>(M_SrcColor::fromValue(t_i))... \
            }; \
        }(std::make_index_sequence<std::tuple_size_v<ConvertLut<M_DstColor, M_SrcColor>>>())
    
    MAKE_CONVERT_LUT(GS4, R5G6B5);
    MAKE_CONVERT_LUT(GS4, R5G5B5A1);
    MAKE_CONVERT_LUT(GS4, R4G4B4A4);

    #undef MAKE_CONVERT_LUT
} // namespace picon::graphics::color