/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/resources/ResourceIndex.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

namespace rw {

template<re::CompTimeString k_lit>
consteval re::ResourceID fontID() {
    return re::resourceID<"fonts/" + k_lit + ".ttf">();
};

template<re::CompTimeString k_lit>
consteval re::ResourceID textureID() {
    return re::resourceID<"textures/" + k_lit + ".png">();
};

} // namespace rw
