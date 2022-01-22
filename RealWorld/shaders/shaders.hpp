#pragma once

namespace shaders {

	const int LOC_PROJECTION_MATRIX = 0;

	const int LOC_BASE_TEXTURE = 1;

	extern const char* standard_vert;

	extern const char* standardOut1_frag;

	extern const char* gauss32_frag;

	extern const char* data_vert;

	extern const char* worldDebug_frag;

	const int LOC_AIR_ID = 10;
	const int LOC_TIME = 6;

	extern const char* setWithVarUpdate_vert;

	extern const char* setWithVarUpdate_frag;

}

namespace WGS {

	const int LOC_SET = 5;

	const int LOC_WORLD_TEXTURE = 2;

	//Chunk generator \|/
	const int LOC_TILES_SELECTOR = 1;
	const int LOC_TILES0_TEXTURE = 2;
	const int LOC_TILES1_TEXTURE = 3;
	const int LOC_MATERIAL_TEXTURE = 4;

	extern const char* worldPT_vert;

	extern const char* chunkGen_vert;

	extern const char* set_frag;

	extern const char* basicTerrain_frag;

	extern const char* var_frag;

	const int LOC_CELL_AUTO_LOW = 33;
	const int LOC_CELL_AUTO_HIGH = 34;

	extern const char* cellularAutomaton_frag;

}