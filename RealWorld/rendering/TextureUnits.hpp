#pragma once
#include <RealEngine/graphics/textures/TextureUnit.hpp>


const RE::TextureUnit TEX_UNIT_VOLATILE = {0};				/**< Texture unit without reservation; must stay active */

const RE::TextureUnit TEX_UNIT_BLOCK_ATLAS = {1};			/**< Texture unit reserved for the block atlas */
const RE::TextureUnit TEX_UNIT_WALL_ATLAS = {2};			/**< Texture unit reserved for the wall atlas */

const RE::TextureUnit TEX_UNIT_WORLD_TEXTURE = {3};			/**< Texture unit reserved for the world texture */

//4: worldDrawer: diaphragm
const RE::TextureUnit TEX_UNIT_DIAPHRAGM = {4};
//5: worldDrawer: lighting
const RE::TextureUnit TEX_UNIT_LIGHTING = {5};
//6: worldDrawer: finished (diaphragm and lighting combined)
const RE::TextureUnit TEX_UNIT_LIGHT_FINISHED = {6};

const RE::TextureUnit TEX_UNIT_GEN_TILES[] = {{7}, {8}};
const RE::TextureUnit TEX_UNIT_GEN_MATERIAL = {9};