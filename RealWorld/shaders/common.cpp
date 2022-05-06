/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/shaders/common.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view standard_vert =
#include "common/colorUV.vert"
;

std::string_view texture_frag =
#include "common/texture.frag"
;