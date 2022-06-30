/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/textures/TextureUnit.hpp>


const RE::TextureUnit TEX_UNIT_VOLATILE = {0};				/**< Texture unit without reservation; must stay active */

const RE::TextureUnit TEX_UNIT_WORLD_TEXTURE = {1};			/**< Texture unit reserved for the world texture */
const RE::TextureUnit TEX_UNIT_BLOCK_ATLAS = {2};			/**< Texture unit reserved for the block atlas */
const RE::TextureUnit TEX_UNIT_WALL_ATLAS = {3};			/**< Texture unit reserved for the wall atlas */
const RE::TextureUnit TEX_UNIT_BLOCK_LIGHT_ATLAS = {4};		/**< Texture unit reserved for the block atlas */
const RE::TextureUnit TEX_UNIT_WALL_LIGHT_ATLAS = {5};		/**< Texture unit reserved for the wall atlas */

const RE::TextureUnit TEX_UNIT_LIGHT = {6};
const RE::TextureUnit TEX_UNIT_TRANSLU = {7};
const RE::TextureUnit TEX_UNIT_SHADOWS = {8};

const RE::TextureUnit TEX_UNIT_GEN_TILES[] = {{9}, {10}};
const RE::TextureUnit TEX_UNIT_GEN_MATERIAL = {11};
