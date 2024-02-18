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

#define TILE xyzw
#define TILE_TYPE xz
#define TILE_VAR yw
#define BLOCK xy
#define BLOCK_TYPE x
#define BLOCK_VAR y
#define WALL zw
#define WALL_TYPE z
#define WALL_VAR w

#define TL TILE
#define TL_T TILE_TYPE
#define TL_V TILE_VAR
#define BL BLOCK
#define BL_T BLOCK_TYPE
#define BL_V BLOCK_VAR
#define WL WALL
#define WL_T WALL_TYPE
#define WL_V WALL_VAR

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
const uint k_firstNonsolidBl =  224;//0b1110'0000
const uint k_waterBl =          224;
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
const uint k_firstNonsolidWl =  224;//0b1110'0000
const uint k_oakWoodWl =        224;
const uint k_acaciaWoodWl =     225;
const uint k_tallGrassWl =      226;
const uint k_wheatWl =          227;
const uint k_leafWl =           228;
const uint k_airWl =            255;
const uint k_neverWl =          256;


// Dual tiles (both block and wall)
const uvec4 k_stone =           {k_stoneBl,         0,  k_stoneWl,          0};
const uvec4 k_dirt =            {k_dirtBl,          0,  k_dirtWl,           0};
const uvec4 k_grass =           {k_grassBl,         0,  k_grassWl,          0};
const uvec4 k_coldStone =       {k_coldStoneBl,     0,  k_coldStoneWl,      0};
const uvec4 k_sand =            {k_sandBl,          0,  k_sandWl,           0};
const uvec4 k_coldDirt =        {k_coldDirtBl,      0,  k_coldDirtWl,       0};
const uvec4 k_coldGrass =       {k_coldGrassBl,     0,  k_coldGrassWl,      0};
const uvec4 k_mud =             {k_mudBl,           0,  k_mudWl,            0};
const uvec4 k_mudGrass =        {k_mudGrassBl,      0,  k_mudGrassWl,       0};
const uvec4 k_dryGrass =        {k_dryGrassBl,      0,  k_dryGrassWl,       0};
const uvec4 k_hallowStone =     {k_hallowStoneBl,   0,  k_hallowStoneWl,    0};
const uvec4 k_hallowDirt =      {k_hallowDirtBl,    0,  k_hallowDirtWl,     0};
const uvec4 k_hallowGrass =     {k_hallowGrassBl,   0,  k_hallowGrassWl,    0};
const uvec4 k_highlighter =     {k_highlighterBl,   0,  k_highlighterWl,    0};

// Fluid tiles (only blocks)
const uvec2 k_water =           {k_waterBl,         0};
const uvec2 k_lava =            {k_lavaBl,          0};
const uvec2 k_acid =            {k_acidBl,          0};
const uvec2 k_steam =           {k_steamBl,         0};
const uvec2 k_fire =            {k_fireBl,          0};
const uvec2 k_smoke =           {k_smokeBl,         0};

// Vegetation tiles (only walls)
const uvec2 k_oakWood =         {k_oakWoodWl,       0};
const uvec2 k_acaciaWood =      {k_acaciaWoodWl,    0};
const uvec2 k_tallGrass =       {k_tallGrassWl,     0};
const uvec2 k_wheat =           {k_wheatWl,         0};
const uvec2 k_leaf =            {k_leafWl,          0};

const uvec4 k_air =             {k_airBl,           0,  k_airWl,            0};
const uvec4 k_never =           {k_neverBl,         0,  k_neverWl,          0};

const uvec4 k_firstNonsolid =   {k_firstNonsolidBl, 0,  k_firstNonsolidWl,  0};


bool isSolidBlock(uint block_type){ return block_type < k_firstNonsolidBl; }
bool isNonsolidBlock(uint block_type){ return block_type >= k_firstNonsolidBl; }
bool isAirBlock(uint block_type){ return block_type == k_airBl; }

bool isSolidWall(uint wall_type){ return wall_type < k_firstNonsolidWl; }
bool isNonsolidWall(uint wall_type){ return wall_type >= k_firstNonsolidWl; }
bool isAirWall(uint wall_type){ return wall_type == k_airWl; }

bvec2 isSolidTile(uvec2 tile_type){ return lessThan(tile_type, k_firstNonsolid.TL_T); }
bvec2 isNonsolidTile(uvec2 tile_type){ return greaterThanEqual(tile_type, k_firstNonsolid.TL_T); }
bvec2 isAirTile(uvec2 tile_type){ return equal(tile_type, k_air.TL_T); }

#endif // !TILE_GLSL