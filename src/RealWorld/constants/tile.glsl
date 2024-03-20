/*! 
 *  @author    Dubsky Tomas
 */
#ifndef TILE_GLSL
#define TILE_GLSL

const uvec2 uTilePx = {4, 4};
const ivec2 iTilePx = ivec2(uTilePx);
const vec2  TilePx  = vec2(uTilePx);
const ivec2 k_tileLowZeroBits = {2, 2};

vec2  pxToTi(vec2 posPx)  { return floor(posPx / TilePx); }
float pxToTi(float posPx) { return floor(posPx / TilePx.x); }
ivec2 pxToTi(ivec2 posPx) { return posPx >> k_tileLowZeroBits; }
int   pxToTi(int posPx)   { return posPx >> k_tileLowZeroBits.x; }

vec2  tiToPx(vec2 posTi)  { return posTi * TilePx; }
float tiToPx(float posTi) { return posTi * TilePx.x; }
ivec2 tiToPx(ivec2 posTi) { return posTi << k_tileLowZeroBits; }
int   tiToPx(int posTi)   { return posTi << k_tileLowZeroBits.x; }

#define LAYER_TYPE x
#define LAYER_VARIANT y

#define L_T LAYER_TYPE
#define L_V LAYER_VARIANT

// Blocks
const uint k_stoneBl =          0;
const uint k_dirtBl =           1;
const uint k_grassBl =          2;
const uint k_coldStoneBl =      3;
const uint k_sandBl =           4;
const uint k_coldDirtBl =       5;
const uint k_coldGrassBl =      6;
const uint k_mudBl =            7;
const uint k_mudGrassBl =       8;
const uint k_dryGrassBl =       9;
const uint k_hallowStoneBl =    10;
const uint k_hallowDirtBl =     11;
const uint k_hallowGrassBl =    12;
const uint k_highlighterBl =    223;
// Fluids
const uint k_firstNonsolidBl =  224; // 0b1110'0000
const uint k_waterBl =          k_firstNonsolidBl;
const uint k_lavaBl =           225;
const uint k_steamBl =          226;
const uint k_fireBl =           227;
const uint k_smokeBl =          228;
const uint k_acidBl =           254;
const uint k_airBl =            255;
const uint k_neverBl =          256;


// Walls
const uint k_stoneWl =          0;
const uint k_dirtWl =           1;
const uint k_grassWl =          2;
const uint k_coldStoneWl =      3;
const uint k_sandWl =           4;
const uint k_coldDirtWl =       5;
const uint k_coldGrassWl =      6;
const uint k_mudWl =            7;
const uint k_mudGrassWl =       8;
const uint k_dryGrassWl =       9;
const uint k_hallowStoneWl =    10;
const uint k_hallowDirtWl =     11;
const uint k_hallowGrassWl =    12;
const uint k_highlighterWl =    223;
// Vegetation
const uint k_firstNonsolidWl =  224; // 0b1110'0000
const uint k_oakWoodWl =        k_firstNonsolidWl;
const uint k_acaciaWoodWl =     225;
const uint k_coniferousWoodWl = 226;
const uint k_tallGrassWl =      227;
const uint k_coldTallGrassWl =  228;
const uint k_mudTallGrassWl =   229;
const uint k_dryTallGrassWl =   230;
const uint k_leafWl =           231;
const uint k_dryLeafWl =        232;
const uint k_needleWl =         233;
const uint k_withyWl =          234;
const uint k_cactusWl =         235;
const uint k_airWl =            255;
const uint k_neverWl =          256;


// Dual tiles (both block and wall)
const uvec2 k_stone =           {k_stoneBl,         k_stoneWl};
const uvec2 k_dirt =            {k_dirtBl,          k_dirtWl};
const uvec2 k_grass =           {k_grassBl,         k_grassWl};
const uvec2 k_coldStone =       {k_coldStoneBl,     k_coldStoneWl};
const uvec2 k_sand =            {k_sandBl,          k_sandWl};
const uvec2 k_coldDirt =        {k_coldDirtBl,      k_coldDirtWl};
const uvec2 k_coldGrass =       {k_coldGrassBl,     k_coldGrassWl};
const uvec2 k_mud =             {k_mudBl,           k_mudWl};
const uvec2 k_mudGrass =        {k_mudGrassBl,      k_mudGrassWl};
const uvec2 k_dryGrass =        {k_dryGrassBl,      k_dryGrassWl};
const uvec2 k_hallowStone =     {k_hallowStoneBl,   k_hallowStoneWl};
const uvec2 k_hallowDirt =      {k_hallowDirtBl,    k_hallowDirtWl};
const uvec2 k_hallowGrass =     {k_hallowGrassBl,   k_hallowGrassWl};
const uvec2 k_highlighter =     {k_highlighterBl,   k_highlighterWl};

const uvec2 k_firstNonsolid =   {k_firstNonsolidBl, k_firstNonsolidWl};

const uvec2 k_air =             {k_airBl,           k_airWl};
const uvec2 k_never =           {k_neverBl,         k_neverWl};


bool isSolidBlock(uint blockType){ return blockType < k_firstNonsolidBl; }
bool isNonsolidBlock(uint blockType){ return blockType >= k_firstNonsolidBl; }
bool isAirBlock(uint blockType){ return blockType == k_airBl; }

bool isSolidWall(uint wallType){ return wallType < k_firstNonsolidWl; }
bool isNonsolidWall(uint wallType){ return wallType >= k_firstNonsolidWl; }
bool isAirWall(uint wallType){ return wallType == k_airWl; }

bvec2 isSolidTile(uvec2 tileType){ return lessThan(tileType, k_firstNonsolid); }
bvec2 isNonsolidTile(uvec2 tileType){ return greaterThanEqual(tileType, k_firstNonsolid); }
bvec2 isAirTile(uvec2 tileType){ return equal(tileType, k_air); }

#endif // !TILE_GLSL