R""(
const float TILEPx = 		4.0;

const float LIGHT_MAX_RANGEUn = 30;
const float LIGHT_DOWNSAMPLE = 4;

const uvec2 AIR = 			{0, 0};
const uvec2 STONE = 		{1, 1};
const uvec2 DIRT = 			{2, 2};
const uvec2 GRASS = 		{3, 3};
const uvec2 COLD_STONE = 	{4, 4};
const uvec2 SAND = 			{5, 5};
const uvec2 COLD_DIRT = 	{6, 6};
const uvec2 COLD_GRASS = 	{7, 7};
const uvec2 MUD = 			{8, 8};
const uvec2 MUD_GRASS = 	{9, 9};
const uvec2 DRY_GRASS = 	{10, 10};

const uvec2 LAST_FLUID = 	{1, 1};

const uvec2 STONE_TILES[] = {
	STONE,
	COLD_STONE
};

const uvec2 SURFACE_TILES[3][3] = {
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