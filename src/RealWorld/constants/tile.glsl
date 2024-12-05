/*! 
 *  @author    Dubsky Tomas
 */
#ifndef TILE_GLSL
#define TILE_GLSL
#include <RealShaders/CppIntegration.glsl>

constexpr int k_physicsStepsPerSecond = 100;

constexpr uvec2 uTilePx = {4, 4};
constexpr ivec2 iTilePx = ivec2(uTilePx);
constexpr vec2  TilePx  = vec2(uTilePx);
constexpr ivec2 k_tileLowZeroBits = {2, 2};

inline vec2  pxToTi(vec2 posPx)  { return floor(posPx / TilePx); }
inline float pxToTi(float posPx) { return floor(posPx / TilePx.x); }
inline ivec2 pxToTi(ivec2 posPx) { return posPx >> k_tileLowZeroBits; }
inline int   pxToTi(int posPx)   { return posPx >> k_tileLowZeroBits.x; }

inline vec2  tiToPx(vec2 posTi)  { return posTi * TilePx; }
inline float tiToPx(float posTi) { return posTi * TilePx.x; }
inline ivec2 tiToPx(ivec2 posTi) { return posTi << k_tileLowZeroBits; }
inline int   tiToPx(int posTi)   { return posTi << k_tileLowZeroBits.x; }

#define LAYER_TYPE x
#define LAYER_VARIANT y

#define L_T LAYER_TYPE
#define L_V LAYER_VARIANT

// Blocks
constexpr uint k_stoneBl          = 0;
constexpr uint k_dirtBl           = 1;
constexpr uint k_grassBl          = 2;
constexpr uint k_coldStoneBl      = 3;
constexpr uint k_sandBl           = 4;
constexpr uint k_coldDirtBl       = 5;
constexpr uint k_coldGrassBl      = 6;
constexpr uint k_mudBl            = 7;
constexpr uint k_mudGrassBl       = 8;
constexpr uint k_dryGrassBl       = 9;
constexpr uint k_hallowStoneBl    = 10;
constexpr uint k_hallowDirtBl     = 11;
constexpr uint k_hallowGrassBl    = 12;
constexpr uint k_highlighterBl    = 223;
// Fluids
constexpr uint k_firstNonsolidBl  = 224; // 0b1110'0000
constexpr uint k_waterBl          = k_firstNonsolidBl;
constexpr uint k_lavaBl           = 225;
constexpr uint k_steamBl          = 226;
constexpr uint k_fireBl           = 227;
constexpr uint k_smokeBl          = 228;
constexpr uint k_droppedLeafBl    = 229;
constexpr uint k_droppedDryLeafBl = 230;
constexpr uint k_acidBl           = 253;
constexpr uint k_airBl            = 255;
constexpr uint k_neverBl          = ~0;


// Walls
constexpr uint k_stoneWl          = 0;
constexpr uint k_dirtWl           = 1;
constexpr uint k_grassWl          = 2;
constexpr uint k_coldStoneWl      = 3;
constexpr uint k_sandWl           = 4;
constexpr uint k_coldDirtWl       = 5;
constexpr uint k_coldGrassWl      = 6;
constexpr uint k_mudWl            = 7;
constexpr uint k_mudGrassWl       = 8;
constexpr uint k_dryGrassWl       = 9;
constexpr uint k_hallowStoneWl    = 10;
constexpr uint k_hallowDirtWl     = 11;
constexpr uint k_hallowGrassWl    = 12;
constexpr uint k_highlighterWl    = 223;
// Vegetation
constexpr uint k_firstNonsolidWl  = 224; // 0b1110'0000
constexpr uint k_oakWoodWl        = k_firstNonsolidWl;
constexpr uint k_acaciaWoodWl     = 225;
constexpr uint k_coniferousWoodWl = 226;
constexpr uint k_palmWoodWl       = 227;

constexpr uint k_tallGrassWl      = 240;
constexpr uint k_coldTallGrassWl  = 241;
constexpr uint k_mudTallGrassWl   = 242;
constexpr uint k_dryTallGrassWl   = 243;
constexpr uint k_leafWl           = 244;
constexpr uint k_dryLeafWl        = 245;
constexpr uint k_needleWl         = 246;
constexpr uint k_palmLeafWl       = 247;
constexpr uint k_withyWl          = 248;
constexpr uint k_cactusWl         = 249;
constexpr uint k_burningWoodWl    = 250;
constexpr uint k_burntWoodWl      = 251;
constexpr uint k_hallowWoodWl     = 252;

constexpr uint k_airWl            = 255;
constexpr uint k_neverWl          = ~0;


// Dual tiles (both block and wall)
constexpr uvec2 k_stone           = {k_stoneBl,         k_stoneWl};
constexpr uvec2 k_dirt            = {k_dirtBl,          k_dirtWl};
constexpr uvec2 k_grass           = {k_grassBl,         k_grassWl};
constexpr uvec2 k_coldStone       = {k_coldStoneBl,     k_coldStoneWl};
constexpr uvec2 k_sand            = {k_sandBl,          k_sandWl};
constexpr uvec2 k_coldDirt        = {k_coldDirtBl,      k_coldDirtWl};
constexpr uvec2 k_coldGrass       = {k_coldGrassBl,     k_coldGrassWl};
constexpr uvec2 k_mud             = {k_mudBl,           k_mudWl};
constexpr uvec2 k_mudGrass        = {k_mudGrassBl,      k_mudGrassWl};
constexpr uvec2 k_dryGrass        = {k_dryGrassBl,      k_dryGrassWl};
constexpr uvec2 k_hallowStone     = {k_hallowStoneBl,   k_hallowStoneWl};
constexpr uvec2 k_hallowDirt      = {k_hallowDirtBl,    k_hallowDirtWl};
constexpr uvec2 k_hallowGrass     = {k_hallowGrassBl,   k_hallowGrassWl};
constexpr uvec2 k_highlighter     = {k_highlighterBl,   k_highlighterWl};

constexpr uvec2 k_firstNonsolid   = {k_firstNonsolidBl, k_firstNonsolidWl};

constexpr uvec2 k_air             = {k_airBl,           k_airWl};
constexpr uvec2 k_never           = {k_neverBl,         k_neverWl};

inline uint  basicType(uint type)           { return type & 0xffu; }
inline uvec2 basicType(uvec2 tileType)      { return tileType & 0xffu; }
inline uint  extendedType(uint type)        { return type & 0xff00u; }
inline uvec2 extendedType(uvec2 tileType)   { return tileType & 0xffu; }

inline bool  isBasicType(uint type)         { return basicType(type) == type; }
inline bvec2 isBasicType(uvec2 tileType)    { return equal(basicType(tileType), tileType); }

inline bool  isSolidBlock(uint blockType)   { return isBasicType(blockType) && blockType < k_firstNonsolidBl; }
inline bool  isNonsolidBlock(uint blockType){ return isBasicType(blockType) && blockType >= k_firstNonsolidBl; }
inline bool  isAirBlock(uint blockType)     { return blockType == k_airBl; }

inline bool  isSolidWall(uint wallType)     { return isBasicType(wallType) && wallType < k_firstNonsolidWl; }
inline bool  isNonsolidWall(uint wallType)  { return isBasicType(wallType) && wallType >= k_firstNonsolidWl; }
inline bool  isAirWall(uint wallType)       { return wallType == k_airWl; }

inline bvec2 isSolidTile(uvec2 tileType)    { return bvec2(uvec2(isBasicType(tileType)) & uvec2(lessThan(tileType, k_firstNonsolid))); }
inline bvec2 isNonsolidTile(uvec2 tileType) { return bvec2(uvec2(isBasicType(tileType)) & uvec2(greaterThanEqual(tileType, k_firstNonsolid))); }
inline bvec2 isAirTile(uvec2 tileType)      { return equal(tileType, k_air); }

const uint k_looseTypeBit = 0x100;
inline bool  isLooseType(uint type)         { return bool(type & k_looseTypeBit); }
inline bvec2 isLooseType(uvec2 tileType)    { return bvec2(tileType & k_looseTypeBit); }

#endif // !TILE_GLSL