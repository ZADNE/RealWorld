#include <RealWorld/shaders/world_dynamics.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view setWithUpdate_vert =
#include "world_dynamics/setWithUpdate.vert"
;

std::string_view setWithUpdate_frag =
#include "external/float_hash.glsl"
#include "common/tileConstants.glsl"
#include "common/textureUnits.glsl"
#include "world_dynamics/setWithUpdate.frag"
;

std::string_view modify_comp =
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "world_dynamics/WorldDynamicsUBO.glsl"
#include "world_dynamics/tileLoadStore.glsl"
#include "world_dynamics/modify.comp"
;

std::string_view dynamics_comp =
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "world_dynamics/WorldDynamicsUBO.glsl"
#include "world_dynamics/tileLoadStore.glsl"
#include "world_dynamics/dynamics.comp"
;

std::string_view transform_comp =
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "world_dynamics/WorldDynamicsUBO.glsl"
#include "world_dynamics/tileLoadStore.glsl"
#include "world_dynamics/transform.comp"
;

std::string_view playerDynamics_comp =
#include "common/tileConstants.glsl"
#include "common/imageUnits.glsl"
#include "world_dynamics/tileLoad.glsl"
#include "world_dynamics/playerMovementUBO.glsl"
#include "world_dynamics/playerHitboxSSBO.glsl"
#include "world_dynamics/playerDynamics.comp"
;