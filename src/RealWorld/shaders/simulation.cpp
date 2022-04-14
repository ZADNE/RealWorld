﻿#include <RealWorld/shaders/simulation.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

std::string_view modify_comp =
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "simulation/WorldDynamicsUBO.glsl"
#include "simulation/tile_load_store.glsl"
#include "simulation/modify.comp"
;

std::string_view tileTransformations_comp =
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "simulation/ActiveChunksSSBO.glsl"
#include "simulation/WorldDynamicsUBO.glsl"
#include "simulation/tile_load_store.glsl"
#include "simulation/tile_transformations.comp"
;

std::string_view fluidDynamics_comp =
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "simulation/WorldDynamicsUBO.glsl"
#include "simulation/tile_load_store.glsl"
#include "simulation/fluid_dynamics.comp"
;

std::string_view playerMovement_comp =
#include "common/tile_constants.glsl"
#include "common/image_units.glsl"
#include "simulation/tile_load.glsl"
#include "simulation/PlayerMovementUBO.glsl"
#include "simulation/PlayerHitboxSSBO.glsl"
#include "simulation/player_movement.comp"
;

std::string_view continuityAnalyzer_comp =
#include "common/tile_constants.glsl"
#include "simulation/ActiveChunksSSBO.glsl"
#include "simulation/continuity_analyzer.comp"
;
