﻿#include <RealWorld/shaders/shaders.hpp>

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

const char* shaders::standard_vert =
#include "standard.vert"
;

const char* shaders::standardOut1_frag =
#include "standardOut1.frag"
;

const char* shaders::texture_frag =
#include "texture.frag"
;

const char* shaders::data_vert =
#include "data.vert"
;

const char* shaders::worldDebug_frag =
#include "worldDebug.frag"
;

const char* shaders::setWithUpdate_vert =
#include "setWithUpdate.vert"
;

const char* shaders::setWithUpdate_frag =
#include "setWithUpdate.frag"
;

const char* WGS::worldPT_vert =
#include "worldPT.vert"
;

const char* WGS::chunkGen_vert =
#include "chunkGen.vert"
;

const char* WGS::set_frag =
#include "set.frag"
;

const char* WGS::basicTerrain_frag =
#include "basicTerrain.frag"
;

const char* WGS::selectVariation_frag =
#include "selectVariation.frag"
;

const char* WGS::cellularAutomaton_frag =
#include "cellularAutomaton.frag"
;