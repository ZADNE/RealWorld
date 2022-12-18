/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

constexpr uint32_t TEX_UNIT_VOLATILE = {20};                /**< Texture unit without reservation; must stay active */

constexpr uint32_t TEX_UNIT_WORLD_TEXTURE = {21};           /**< Texture unit reserved for the world texture */
constexpr uint32_t TEX_UNIT_BLOCK_ATLAS = {22};             /**< Texture unit reserved for the block atlas */
constexpr uint32_t TEX_UNIT_WALL_ATLAS = {23};              /**< Texture unit reserved for the wall atlas */
constexpr uint32_t TEX_UNIT_BLOCK_LIGHT_ATLAS = {24};       /**< Texture unit reserved for the block atlas */
constexpr uint32_t TEX_UNIT_WALL_LIGHT_ATLAS = {25};        /**< Texture unit reserved for the wall atlas */

constexpr uint32_t TEX_UNIT_LIGHT = {26};
constexpr uint32_t TEX_UNIT_TRANSLU = {27};
constexpr uint32_t TEX_UNIT_SHADOWS = {28};

constexpr uint32_t TEX_UNIT_GEN_TILES[] = {{29}, {30}};
constexpr uint32_t TEX_UNIT_GEN_MATERIAL = {31};
