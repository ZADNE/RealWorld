R""(
struct DynamicLight{
	ivec2 posPx;
	uint col;
	uint padding;
};

layout(std430, binding = 0) readonly restrict buffer DynamicLightsSSIB {
	DynamicLight lights[];
};

)""