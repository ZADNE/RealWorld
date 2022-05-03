R""(
const float TILEPx = 			4.0;

const ivec2 CHUNK_SIZE = 		{128, 128};

const int ACTIVE_CHUNKS_AREA_X = 16;
const int ACTIVE_CHUNKS_AREA_Y = ACTIVE_CHUNKS_AREA_X;
const int ACTIVE_CHUNKS_MAX_UPDATES = (ACTIVE_CHUNKS_AREA_X - 1) * (ACTIVE_CHUNKS_AREA_Y - 1);
const ivec2 ACTIVE_CHUNKS_AREA = ivec2(ACTIVE_CHUNKS_AREA_X, ACTIVE_CHUNKS_AREA_Y);
const ivec2 ACTIVE_CHUNKS_AREA_MASK = ivec2(ACTIVE_CHUNKS_AREA_X, ACTIVE_CHUNKS_AREA_Y) - 1;

const float LIGHT_MAX_RANGEUn = 30;
const float LIGHT_DOWNSAMPLE = 	4;

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

const uint AIR_BL = 			0;

const uint WATER_BL = 			1;
const uint LAVA_BL = 			2;
const uint STEAM_BL = 			3;
const uint FIRE_BL = 			4;
const uint SMOKE_BL = 			5;

const uint ACID_BL = 			15;
const uint LAST_FLUID_BL = 		15;

const uint STONE_BL = 			16;
const uint DIRT_BL = 			17;
const uint GRASS_BL = 			18;
const uint COLD_STONE_BL = 		19;
const uint SAND_BL = 			20;
const uint COLD_DIRT_BL = 		21;
const uint COLD_GRASS_BL = 		22;
const uint MUD_BL = 			23;
const uint MUD_GRASS_BL = 		24;
const uint DRY_GRASS_BL = 		25;

const uint NEVER_BL = 			256;

const uint AIR_WL = 			0;
const uint STONE_WL = 			1;
const uint DIRT_WL = 			2;
const uint GRASS_WL = 			3;
const uint COLD_STONE_WL = 		4;
const uint SAND_WL = 			5;
const uint COLD_DIRT_WL = 		6;
const uint COLD_GRASS_WL = 		7;
const uint MUD_WL = 			8;
const uint MUD_GRASS_WL = 		9;
const uint DRY_GRASS_WL = 		10;

const uint NEVER_WL = 			256;

const uvec4 AIR = 				{	AIR_BL,			0,	AIR_WL,			0};

const uvec2 WATER = 			{	WATER_BL,		0};
const uvec2 LAVA = 				{	LAVA_BL,		0};
const uvec2 ACID = 				{	ACID_BL,		0};
const uvec2 STEAM = 			{	STEAM_BL,		0};
const uvec2 FIRE = 				{	FIRE_BL,		0};
const uvec2 SMOKE = 			{	SMOKE_BL,		0};

const uvec4 STONE = 			{	STONE_BL,		0,	STONE_WL,		0};
const uvec4 DIRT = 				{	DIRT_BL,		0,	DIRT_WL,		0};
const uvec4 GRASS = 			{	GRASS_BL,		0,	GRASS_WL,		0};
const uvec4 COLD_STONE = 		{	COLD_STONE_BL,	0,	COLD_STONE_WL,	0};
const uvec4 SAND = 				{	SAND_BL,		0,	SAND_WL,		0};
const uvec4 COLD_DIRT = 		{	COLD_DIRT_BL,	0,	COLD_DIRT_WL,	0};
const uvec4 COLD_GRASS = 		{	COLD_GRASS_BL,	0,	COLD_GRASS_WL,	0};
const uvec4 MUD = 				{	MUD_BL,			0,	MUD_WL,			0};
const uvec4 MUD_GRASS = 		{	MUD_GRASS_BL,	0,	MUD_GRASS_WL,	0};
const uvec4 DRY_GRASS = 		{	DRY_GRASS_BL,	0,	DRY_GRASS_WL,	0};

const uvec4 NEVER = 			{	NEVER_BL,		0,	NEVER_WL,		0};

const uvec4 LAST_FLUID = 		{	LAST_FLUID_BL,	0,	AIR_WL,			0};

)""