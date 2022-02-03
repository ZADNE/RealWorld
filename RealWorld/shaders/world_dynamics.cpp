#include <RealWorld/shaders/world_dynamics.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view setWithUpdate_vert =
#include "world_dynamics/setWithUpdate.vert"
;

std::string_view setWithUpdate_frag =
#include "world_dynamics/setWithUpdate.frag"
;