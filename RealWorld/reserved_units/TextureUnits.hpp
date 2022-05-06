﻿/*! 
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

const RE::TextureUnit TEX_UNIT_LIGHTS_LIGHT = {6};			/**< Texture unit reserved for lights of tiles */
const RE::TextureUnit TEX_UNIT_LIGHTS_TRANSLU = {7};		/**< Texture unit reserved for translucency of tiles */
const RE::TextureUnit TEX_UNIT_LIGHTS_COMPUTED = {8};		/**< Texture unit reserved for computed lighting of tiles */

const RE::TextureUnit TEX_UNIT_GEN_TILES[] = {{9}, {10}};
const RE::TextureUnit TEX_UNIT_GEN_MATERIAL = {11};