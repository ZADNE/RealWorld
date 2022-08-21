/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/textures/ImageUnit.hpp>

const RE::ImageUnit IMG_UNIT_VOLATILE = { 0 };			/**< Image unit without reservation; must stay active */

const RE::ImageUnit IMG_UNIT_WORLD = { 1 };				/**< Image unit reserved for the world image */

const RE::ImageUnit IMG_UNIT_GEN_TILES[] = { {2}, {3} };	/**< Image units reserved for CA ping pong of chunk generation */
const RE::ImageUnit IMG_UNIT_GEN_MATERIAL = { 4 };		/**< Image unit reserved for the material of chunk generation */

const RE::ImageUnit IMG_UNIT_LIGHT = { 5 };
const RE::ImageUnit IMG_UNIT_TRANSLU = { 6 };
const RE::ImageUnit IMG_UNIT_SHADOWS = { 7 };
