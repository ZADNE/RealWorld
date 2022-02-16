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


const uvec4 AIR = 				{	0,	0,	0,	0};
const uvec4 WATER = 			{	1,	0,	1,	0};
const uvec4 STONE = 			{	2,	0,	2,	0};
const uvec4 DIRT = 				{	3,	0,	3,	0};
const uvec4 GRASS = 			{	4,	0,	4,	0};
const uvec4 COLD_STONE = 		{	5,	0,	5,	0};
const uvec4 SAND = 				{	6,	0,	6,	0};
const uvec4 COLD_DIRT = 		{	7,	0,	7,	0};
const uvec4 COLD_GRASS = 		{	8,	0,	8,	0};
const uvec4 MUD = 				{	9,	0,	9,	0};
const uvec4 MUD_GRASS = 		{	10,	0,	10,	0};
const uvec4 DRY_GRASS = 		{	11,	0,	11,	0};

const uvec4 LAST_FLUID = 		WATER;

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