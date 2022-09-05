/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/textures/TextureUnit.hpp>


constexpr RE::TextureUnit TEX_UNIT_VOLATILE = { 0 };                /**< Texture unit without reservation; must stay active */

constexpr RE::TextureUnit TEX_UNIT_WORLD_TEXTURE = { 1 };           /**< Texture unit reserved for the world texture */
constexpr RE::TextureUnit TEX_UNIT_BLOCK_ATLAS = { 2 };             /**< Texture unit reserved for the block atlas */
constexpr RE::TextureUnit TEX_UNIT_WALL_ATLAS = { 3 };              /**< Texture unit reserved for the wall atlas */
constexpr RE::TextureUnit TEX_UNIT_BLOCK_LIGHT_ATLAS = { 4 };       /**< Texture unit reserved for the block atlas */
constexpr RE::TextureUnit TEX_UNIT_WALL_LIGHT_ATLAS = { 5 };        /**< Texture unit reserved for the wall atlas */

constexpr RE::TextureUnit TEX_UNIT_LIGHT = { 6 };
constexpr RE::TextureUnit TEX_UNIT_TRANSLU = { 7 };
constexpr RE::TextureUnit TEX_UNIT_SHADOWS = { 8 };

constexpr RE::TextureUnit TEX_UNIT_GEN_TILES[] = { {9}, {10} };
constexpr RE::TextureUnit TEX_UNIT_GEN_MATERIAL = { 11 };
