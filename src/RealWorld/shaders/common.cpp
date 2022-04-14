#include <RealWorld/shaders/common.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view standard_vert =
#include "common/standard.vert"
;

std::string_view standardOut1_frag =
#include "common/standard_out1.frag"
;

std::string_view texture_frag =
#include "common/texture.frag"
;