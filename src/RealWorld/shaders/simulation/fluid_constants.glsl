R""(
/*
	1: WATER, 	2: LAVA, 3: STEAM,
	4: FIRE,	5: SMOKE
*/

//0-1024 = probability to move horizontally
uint FLUIDITY[] = {
	1024,	16,		200,
	24,		64
};

//-1 = moves down (liquids), 1 = moves up (gases)
int PRIM_MOVE_DIR[] = {
	-1,		-1,		1,
	1,		1
};

//0-1024 probability to move vertically
uint PRIM_MOVE_PROB[] = {
	920,	1024,	200,
	256,	64
};

//Fluids with higher density move below fluid lower density
uint DENSITY[] = {
	1000,	3000,	50,
	10,		40
};

//x = 0-1024 probability to convert the fluid
//y = The block that this fluid is converted to
//z = The block that triggers conversion of this fluid when they touch
uvec3 TRIGGER_CONV[] = {
	{1024, STEAM_BL, LAVA_BL},	{512, STONE_BL, WATER_BL},	{0, AIR_BL, NEVER_BL},
	{1024, AIR_BL, WATER_BL},	{1024, AIR_BL, WATER_BL}
};

//x = 0-65535 probability to randomly convert the fluid
//y = The block that this fluid is converted to
//z = 0-65535 probability to randomly convert the fluid if the previous test failed
//w = The block that this fluid is converted to if the previous test failed
uvec4 RANDOM_CONV[] = {
	{1, STEAM_BL, 0, AIR_BL},	{1, STONE_BL, 1, SMOKE_BL},	{64, WATER_BL, 64, AIR_BL},
	{800, SMOKE_BL, 0, AIR_BL},	{384, AIR_BL, 0, AIR_BL}
};

)""