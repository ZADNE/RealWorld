/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/buffers/TypedBuffer.hpp>

 //ATOMIC COUNTER BUFFERS



//SHADER STORAGE BUFFERS

const RE::BufferTypedIndex STRG_BUF_PLAYER = { RE::BufferType::SHADER_STORAGE, 0u };
const RE::BufferTypedIndex STRG_BUF_ACTIVECHUNKS = { RE::BufferType::SHADER_STORAGE, 1u };
const RE::BufferTypedIndex STRG_BUF_EXTERNALLIGHTS = { RE::BufferType::SHADER_STORAGE, 2u };


//UNIFORM BUFFERS

//                                                        0 reserved by RealEngine
const RE::BufferTypedIndex UNIF_BUF_PLAYERMOVEMENT = { RE::BufferType::UNIFORM, 1u };
const RE::BufferTypedIndex UNIF_BUF_CHUNKGEN = { RE::BufferType::UNIFORM, 2u };
const RE::BufferTypedIndex UNIF_BUF_WORLDDRAWER = { RE::BufferType::UNIFORM, 3u };
const RE::BufferTypedIndex UNIF_BUF_WORLDDYNAMICS = { RE::BufferType::UNIFORM, 4u };

//TRANSFORM FEEDBACK BUFFERS

