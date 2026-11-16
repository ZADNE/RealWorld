/**
 *  @author    Dubsky Tomas
 */
export module realworld:resources.ResourceIndex;
import :resources.ResourceIndex_gen;

export namespace rw {

template<re::CompTimeString k_lit>
consteval re::ResourceID fontID() {
    return re::resourceID<"fonts/" + k_lit + ".ttf">();
};

template<re::CompTimeString k_lit>
consteval re::ResourceID textureID() {
    return re::resourceID<"textures/" + k_lit + ".png">();
};

} // namespace rw
