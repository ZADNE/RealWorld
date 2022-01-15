#pragma once

namespace WDS {

	const int LOC_WORLD_TEXTURE = 2;

	const int LOC_POSITION = 3;

	extern const char* tilesDraw_vert;

	extern const char* tilesDraw_frag;

	extern const char* finalLighting_vert;

	extern const char* PT_vert;

	const int LOC_DIAPHRAGM = 100;

	const int LOC_LIGHTING = 101;

	extern const char* combineLighting_frag;

	const int LOC_WORLD_TO_LIGHT_DAYLIGHT = 4;

	const int LOC_WORLD_TO_LIGHT_DIAPHRAGMS = 5;

	extern const char* worldToLight_frag;

	extern const char* addStaticLight_vert;

	extern const char* sumStaticLight_frag;

	extern const char* addStaticLight_frag;

	extern const char* addDynamicLight_vert;

	extern const char* addDynamicLight_frag;

	#ifdef _DEBUG
		//Backpack light simulation function
		void backpack_light();
	#endif // _DEBUG

}