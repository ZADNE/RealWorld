R""(
struct PointLight {
	uint pos;//2 half floats - X and Y pos
	uint col;//RGB = color of the light, A = strength of the light (15 is max for single tile!)
};
layout(std430, binding = 0) buffer PointLightsSSIB {
	PointLight pointLights[];
};

)""