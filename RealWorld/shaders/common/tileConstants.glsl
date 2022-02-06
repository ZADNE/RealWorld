R""(
const float TILEPx = 		4.0;
const float METERPx = 		16.0;
const float tiToMt = 		TILEPx / METERPx;
const float mtToTi = 		METERPx / TILEPx;

const float LIGHT_MAX_RANGEUn = 30;
const float LIGHT_DOWNSAMPLE = 4;

const uvec2 AIR = 			uvec2(0, 0);
const uvec2 STONE = 		uvec2(1, 1);
const uvec2 DIRT = 			uvec2(2, 2);
const uvec2 GRASS = 		uvec2(3, 3);
const uvec2 COLD_STONE = 	uvec2(4, 4);
const uvec2 SAND = 			uvec2(5, 5);
const uvec2 COLD_DIRT = 	uvec2(6, 6);
const uvec2 COLD_GRASS = 	uvec2(7, 7);
const uvec2 MUD = 			uvec2(8, 8);
const uvec2 MUD_GRASS = 	uvec2(9, 9);

const uvec2 LAST_FLUID = 	uvec2(1, 1);

const uvec2 STONE_TILES[] = uvec2[](
	STONE,
	COLD_STONE
);

const uvec2 SURFACE_TILES[][] = uvec2[][](
//temperature>	|low			|normal			|high		humidity \/
	uvec2[](	COLD_STONE,		DIRT,			SAND),			//-low
	uvec2[](	COLD_DIRT,		GRASS,			GRASS),			//-normal
	uvec2[](	COLD_GRASS,		MUD,			MUD_GRASS)		//-high
);

)""