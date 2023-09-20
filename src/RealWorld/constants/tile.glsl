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

const ivec2 iChunkTi =        {128, 128};

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

//Blocks
const uint STONE_BL =           0;
const uint DIRT_BL =            1;
const uint GRASS_BL =           2;
const uint COLD_STONE_BL =      3;
const uint SAND_BL =            4;
const uint COLD_DIRT_BL =       5;
const uint COLD_GRASS_BL =      6;
const uint MUD_BL =             7;
const uint MUD_GRASS_BL =       8;
const uint DRY_GRASS_BL =       9;
const uint HALLOW_STONE_BL =    10;
const uint HALLOW_DIRT_BL =     11;
const uint HALLOW_GRASS_BL =    12;
const uint WOOD_BL =            13;
const uint HIGHLIGHTER_BL =     223;
//Fluids
const uint FIRST_FLUID_BL =     224;//0b1110'0000
const uint WATER_BL =           224;
const uint LAVA_BL =            225;
const uint STEAM_BL =           226;
const uint FIRE_BL =            227;
const uint SMOKE_BL =           228;
const uint ACID_BL =            254;
const uint AIR_BL =             255;
const uint NEVER_BL =           256;


//Walls
const uint STONE_WL =           0;
const uint DIRT_WL =            1;
const uint GRASS_WL =           2;
const uint COLD_STONE_WL =      3;
const uint SAND_WL =            4;
const uint COLD_DIRT_WL =       5;
const uint COLD_GRASS_WL =      6;
const uint MUD_WL =             7;
const uint MUD_GRASS_WL =       8;
const uint DRY_GRASS_WL =       9;
const uint HALLOW_STONE_WL =    10;
const uint HALLOW_DIRT_WL =     11;
const uint HALLOW_GRASS_WL =    12;
const uint WOOD_WL =            13;
const uint HIGHLIGHTER_WL =     223;
const uint AIR_WL =             255;
const uint NEVER_WL =           256;



const uvec4 STONE =             {STONE_BL,          0,    STONE_WL,         0};
const uvec4 DIRT =              {DIRT_BL,           0,    DIRT_WL,          0};
const uvec4 GRASS =             {GRASS_BL,          0,    GRASS_WL,         0};
const uvec4 COLD_STONE =        {COLD_STONE_BL,     0,    COLD_STONE_WL,    0};
const uvec4 SAND =              {SAND_BL,           0,    SAND_WL,          0};
const uvec4 COLD_DIRT =         {COLD_DIRT_BL,      0,    COLD_DIRT_WL,     0};
const uvec4 COLD_GRASS =        {COLD_GRASS_BL,     0,    COLD_GRASS_WL,    0};
const uvec4 MUD =               {MUD_BL,            0,    MUD_WL,           0};
const uvec4 MUD_GRASS =         {MUD_GRASS_BL,      0,    MUD_GRASS_WL,     0};
const uvec4 DRY_GRASS =         {DRY_GRASS_BL,      0,    DRY_GRASS_WL,     0};
const uvec4 HALLOW_STONE =      {HALLOW_STONE_BL,   0,    HALLOW_STONE_WL,  0};
const uvec4 HALLOW_DIRT =       {HALLOW_DIRT_BL,    0,    HALLOW_DIRT_WL,   0};
const uvec4 HALLOW_GRASS =      {HALLOW_GRASS_BL,   0,    HALLOW_GRASS_WL,  0};
const uvec4 WOOD =              {WOOD_BL,           0,    WOOD_WL,          0};
const uvec4 HIGHLIGHTER =       {HIGHLIGHTER_BL,    0,    HIGHLIGHTER_WL,   0};

const uvec2 WATER =             {WATER_BL,          0};
const uvec2 LAVA =              {LAVA_BL,           0};
const uvec2 ACID =              {ACID_BL,           0};
const uvec2 STEAM =             {STEAM_BL,          0};
const uvec2 FIRE =              {FIRE_BL,           0};
const uvec2 SMOKE =             {SMOKE_BL,          0};

const uvec4 AIR =               {AIR_BL,            0,    AIR_WL,           0};
const uvec4 NEVER =             {NEVER_BL,          0,    NEVER_WL,         0};

const uvec4 FIRST_FLUID =       {FIRST_FLUID_BL,    0,    AIR_WL,           0};


bool isSolidBlock(uint block_type){ return block_type < FIRST_FLUID_BL; }
bool isFluidBlock(uint block_type){ return block_type >= FIRST_FLUID_BL; }
bool isAirBlock(uint block_type){ return block_type == AIR_BL; }

bool isSolidWall(uint wall_type){ return wall_type < AIR_WL; }
bool isAirWall(uint wall_type){ return wall_type == AIR_WL; }

bvec2 isSolidTile(uvec2 tile_type){ return lessThan(tile_type, FIRST_FLUID.TL_T); }
bvec2 isFluidTile(uvec2 tile_type){ return greaterThanEqual(tile_type, FIRST_FLUID.TL_T); }
bvec2 isAirTile(uvec2 tile_type){ return equal(tile_type, AIR.TL_T); }

#endif // TILE_GLSL