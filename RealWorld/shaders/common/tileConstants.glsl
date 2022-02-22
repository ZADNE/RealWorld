R""(
const float TILEPx = 			4.0;

const ivec2 CHUNK_SIZE = 		{128, 128};

const float LIGHT_MAX_RANGEUn = 30;
const float LIGHT_DOWNSAMPLE = 	4;

#define TILE xyzw
#define TILE_TYPE xz
#define BLOCK xy
#define BLOCK_TYPE x
#define BLOCK_VAR y
#define WALL zw
#define WALL_TYPE z
#define WALL_VAR w

#define TL TILE
#define TL_T TILE_TYPE
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

const uint LAST_FLUID = 		ACID_BL;

const uvec4 STONE_TILES[] = {
	STONE,
	COLD_STONE
};

const uvec4 SURFACE_TILES[3][3] = {
//humidity>	|low				|normal				|high				temperature \/
	{		COLD_STONE,			COLD_DIRT,			COLD_GRASS	},		//-low
	{		DIRT,				GRASS,				MUD			},		//-normal
	{		SAND,				DRY_GRASS,			MUD_GRASS	}		//-high
};

//x = elevation, y = roughness, z = derivation-variable surface layer width, w = base surface layer width
const vec4 BIOME_PARAMS[3][3] = {
//humidity>	|low						|normal						|high						temperature \/
	{		{2000., 600., 40., 200.},	{800., 300., 120., 120.},	{700., 250., 80., 80.}	},	//-low
	{		{640., 96., 160., 120.},	{640., 96., 200., 160.},	{640., 96., 200., 200.}	},	//-normal
	{		{500., 0., 60., 400.},		{640., 96., 100., 160.},	{640., 120., 240., 280.}}	//-high
};

)""