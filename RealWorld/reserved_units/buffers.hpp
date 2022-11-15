/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/buffers/BufferTyped.hpp>

 //ATOMIC COUNTER BUFFERS

 //SHADER STORAGE BUFFERS
constexpr RE::BufferTypedIndex STRG_BUF_PLAYER = {RE::BufferType::SHADER_STORAGE, 0u};
constexpr RE::BufferTypedIndex STRG_BUF_ACTIVECHUNKS = {RE::BufferType::SHADER_STORAGE, 1u};
constexpr RE::BufferTypedIndex STRG_BUF_EXTERNALLIGHTS = {RE::BufferType::SHADER_STORAGE, 2u};

//UNIFORM BUFFERS
constexpr RE::BufferTypedIndex UNIF_BUF_VIEWPORT_MATRIX = {RE::BufferType::UNIFORM, 0u};
constexpr RE::BufferTypedIndex UNIF_BUF_PLAYERMOVEMENT = {RE::BufferType::UNIFORM, 1u};
constexpr RE::BufferTypedIndex UNIF_BUF_GENERATION = {RE::BufferType::UNIFORM, 2u};
constexpr RE::BufferTypedIndex UNIF_BUF_WORLDDRAWER = {RE::BufferType::UNIFORM, 3u};
constexpr RE::BufferTypedIndex UNIF_BUF_WORLDDYNAMICS = {RE::BufferType::UNIFORM, 4u};
constexpr RE::BufferTypedIndex UNIF_BUF_TILEPROPERTIES = {RE::BufferType::UNIFORM, 5u};

//TRANSFORM FEEDBACK BUFFERS

