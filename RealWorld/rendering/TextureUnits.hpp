﻿#pragma once
#include <RealEngine/graphics/textures/TextureUnit.hpp>

/**
 * @brief 0: volatile unit, used by sprite batches, font batches etc.; MUST STAY ACTIVE BY DEFAULT
 *
 * Reset active texture unit back to this after you have worked with other units.
*/
const RE::TextureUnit TEX_UNIT_VOLATILE = {0};
//1: block atlas
const RE::TextureUnit TEX_UNIT_BLOCK_ATLAS = {1};
//2: wall atlas
const RE::TextureUnit TEX_UNIT_WALL_ATLAS = {2};
//3: world texture
const RE::TextureUnit TEX_UNIT_WORLD_TEXTURE = {3};

//4: worldDrawer: diaphragm
const RE::TextureUnit TEX_UNIT_DIAPHRAGM = {4};
//5: worldDrawer: lighting
const RE::TextureUnit TEX_UNIT_LIGHTING = {5};
//6: worldDrawer: finished (diaphragm and lighting combined)
const RE::TextureUnit TEX_UNIT_LIGHT_FINISHED = {6};

const RE::TextureUnit TEX_UNIT_GEN_TILES[] = {{7}, {8}};
const RE::TextureUnit TEX_UNIT_GEN_MATERIAL = {9};