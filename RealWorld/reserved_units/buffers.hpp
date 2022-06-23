/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/buffers/TypedBuffer.hpp>

 //SHADER STORAGE BUFFERS

const GLuint STRG_BUF_PLAYER = 0u;			/**< Binding point for the SSBO that holds player's position, velocity etc. */
const GLuint STRG_BUF_ACTIVECHUNKS = 1u;	/**< Binding point for the SSBO that holds info about active chunks */

//UNIFORM BUFFERS

//0 reserved by RealEngine
const GLuint UNIF_BUF_PLAYERMOVEMENT = 1u;
const GLuint UNIF_BUF_CHUNKGEN = 2u;
const GLuint UNIF_BUF_WORLDDRAWER = 3u;
const GLuint UNIF_BUF_WORLDDYNAMICS = 4u;
