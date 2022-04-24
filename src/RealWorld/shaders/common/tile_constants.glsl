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
const uint ACID_BL = 			3;
const uint STONE_BL = 			4;
const uint DIRT_BL = 			5;
const uint GRASS_BL = 			6;
const uint COLD_STONE_BL = 		7;
const uint SAND_BL = 			8;
const uint COLD_DIRT_BL = 		9;
const uint COLD_GRASS_BL = 		10;
const uint MUD_BL = 			11;
const uint MUD_GRASS_BL = 		12;
const uint DRY_GRASS_BL = 		13;

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

const uvec4 AIR = 				{	AIR_BL,			0,	AIR_WL,			0};
const uvec2 WATER = 			{	WATER_BL,		0};
const uvec2 LAVA = 				{	LAVA_BL,		0};
const uvec2 ACID = 				{	ACID_BL,		0};
const uvec4 STONE = 			{	STONE_BL,		0,	STONE_WL,		0};
const uvec4 DIRT = 				{	DIRT_BL,		0,	DIRT_WL,		0};
const uvec4 GRASS = 			{	GRASS_BL,		0,	GRASS_WL,		0};
const uvec4 COLD_STONE = 		{	COLD_STONE_BL,	0,	COLD_STONE_WL,	0};
const uvec4 SAND = 				{	SAND_BL,		0,	SAND_WL,		0};
const uvec4 COLD_DIRT = 		{	COLD_DIRT_BL,	0,	COLD_DIRT_WL,	0};
const uvec4 COLD_GRASS = 		{	COLD_GRASS_BL,	0,	COLD_GRASS_WL,	0};
const uvec4 MUD = 				{	MUD_BL,			0,	MUD_WL,		0};
const uvec4 MUD_GRASS = 		{	MUD_GRASS_BL,	0,	MUD_GRASS_WL,	0};
const uvec4 DRY_GRASS = 		{	DRY_GRASS_BL,	0,	DRY_GRASS_WL,	0};

const uvec4 LAST_FLUID = 		{	ACID_BL,		0,	AIR_WL,			0};

const uvec4 STONE_TILES[] = {
	STONE,
	COLD_STONE
};

struct Biome {
	uvec4 tiles;
	vec2 elevation;//x = fixed, y = random
	vec2 roughness;//x = fixed, y = slope-variable
	vec2 surfaceWidth;//x = fixed, y = slope-variable
};

//							tile			elevation		roughness		surfaceWidth
const Biome MOUNTAINS = 	{COLD_STONE,	{1000., 2000.}, {400., 250.}, 	{800., 200.}};
const Biome TUNDRA = 		{COLD_DIRT,		{0., 1000.}, 	{0., 250.}, 	{50., 250.}};
const Biome TAIGA = 		{COLD_GRASS,	{500., 1200.}, 	{0., 150.}, 	{50., 200.}};

const Biome DIRTLAND = 		{DIRT,			{0., 1000.}, 	{100., 150.},	{50., 200.}};
const Biome GRASSLAND = 	{GRASS,			{0., 800.}, 	{100., 200.}, 	{50., 200.}};
const Biome SWAMP = 		{MUD,			{-1500., 400.}, {0., 100.}, 	{500., 100.}};

const Biome DESERT = 		{SAND,			{0., 500.}, 	{0., 0.}, 		{600., 100.}};
const Biome SAVANNA = 		{DRY_GRASS,		{250., 800.}, 	{100., 100.}, 	{200., 200.}};
const Biome RAINFOREST = 	{MUD_GRASS,		{250., 1000.}, 	{150., 160.}, 	{240., 280.}};

const Biome BIOMES[3][3] = {
//humidity>	|low			|normal			|high			temperature \/
	{		MOUNTAINS,		TUNDRA,			TAIGA		},	//-low
	{		DIRTLAND,		GRASSLAND,		SWAMP		},	//-normal
	{		DESERT,			SAVANNA,		RAINFOREST	}	//-high
};

)""