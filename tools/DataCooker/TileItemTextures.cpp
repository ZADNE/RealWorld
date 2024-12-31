/*!
 *  @author    Dubsky Tomas
 */
#include <bit>
#include <cstdint>
#include <random>
#include <type_traits>

#include <RealEngine/graphics/pipelines/Vertex.hpp>
#include <RealEngine/resources/PNGLoader.hpp>
#include <RealEngine/utility/Endianness.hpp>

#include <RealWorld/constants/item.hpp>

#include <DataCooker/TileItemTextures.hpp>

namespace rw::dc {

constexpr re::Color k_black{0, 0, 0, 255};
constexpr re::Color k_white{255, 255, 255, 255};
constexpr re::Color k_red{255, 0, 0, 255};
constexpr re::Color k_blue{0, 0, 255, 255};
constexpr re::Color k_magenta{255, 0, 255, 255};
constexpr re::Color k_transparent{0, 0, 0, 0};

consteval uint32_t uint32(re::Color col) {
    std::array<unsigned char, 4> arr{col.x, col.y, col.z, col.w};
    return std::bit_cast<uint32_t>(arr);
}

void composeTileItemTexture(
    const re::PNGLoader::PNGData& palette, const re::PNGLoader::PNGData& pattern,
    int first, int last, std::string_view outFilepath
) {
    // Prepare output PNG
    int tileCount = last - first + 1;
    re::PNGLoader::PNGData out{};
    out.dims = glm::uvec2{pattern.dims.x, pattern.dims.y * tileCount};
    out.texels.reserve(out.dims.x * out.dims.y * 4);
    out.shape.pivot                 = glm::vec2{pattern.dims} * 0.5f;
    out.shape.subimageDims          = pattern.dims;
    out.shape.subimagesSpritesCount = {1.0f, tileCount};
    size_t patternTexelCount        = pattern.dims.x * pattern.dims.y;
    assert(patternTexelCount * 4 == pattern.texels.size());
    const uint32_t* patternTexels =
        reinterpret_cast<const uint32_t*>(pattern.texels.data());

    // Prepare helper output lambdas
    auto putTile = [&](int tile, int variant) {
        const unsigned char* first =
            &palette.texels[(tile * palette.dims.x + variant) * 4];
        for (int i = 0; i < 4; ++i) { out.texels.push_back(*(first + i)); }
    };
    auto putColor = [&](re::Color col) {
        out.texels.push_back(col.r);
        out.texels.push_back(col.g);
        out.texels.push_back(col.b);
        out.texels.push_back(col.a);
    };

    // Initialize random number generation
    std::default_random_engine rng{1'735'399'038u};
    std::uniform_int_distribution<int> solidInnerDist{0, 11};
    std::uniform_int_distribution<int> solidOuterDist{12, 15};
    std::uniform_int_distribution<int> fluidDist{0, 15};

    // For each tile (reversed because of down-pointing Y axis)
    for (int tile = last; tile >= first; --tile) {
        // For each texel of pattern
        for (int texelIndex = 0; texelIndex < patternTexelCount; ++texelIndex) {
            switch (patternTexels[texelIndex]) {
            case uint32(k_black):       putTile(tile, solidOuterDist(rng)); break;
            case uint32(k_white):       putTile(tile, solidInnerDist(rng)); break;
            case uint32(k_blue):        putTile(tile, fluidDist(rng)); break;
            case uint32(k_transparent): putColor(k_transparent); break;
            default:                    putColor(k_magenta); break;
            }
        }
    }

    // Write PNG
    re::PNGLoader::save(std::string{outFilepath}, out);
}

void composeTileItemTextures(std::string_view inputDir, std::string_view outputDir) {
    std::string inDir{inputDir};
    std::string outDir{outputDir};

    // Blocks
    const re::PNGLoader::PNGData blocksPNG =
        re::PNGLoader::load(inDir + "/game_ready/textures/blockAtlas.png");
    composeTileItemTexture(
        blocksPNG,
        re::PNGLoader::load(inDir + "/cooker_input/textures/itemPatternB.png"),
        0, static_cast<int>(Block::LastUsedSolid),
        outDir + "/textures/itemAtlasB.png"
    );

    // Fluids
    composeTileItemTexture(
        blocksPNG,
        re::PNGLoader::load(inDir + "/cooker_input/textures/itemPatternF.png"),
        static_cast<int>(Block::FirstNonsolid),
        static_cast<int>(Block::LastUsedNonsolid),
        outDir + "/textures/itemAtlasF.png"
    );

    // Walls
    composeTileItemTexture(
        re::PNGLoader::load(inDir + "/game_ready/textures/wallAtlas.png"),
        re::PNGLoader::load(inDir + "/cooker_input/textures/itemPatternW.png"),
        0, static_cast<int>(Wall::LastUsedSolid),
        outDir + "/textures/itemAtlasW.png"
    );
}

} // namespace rw::dc
