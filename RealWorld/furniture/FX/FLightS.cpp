#include <RealWorld/furniture/FX/FLightS.hpp>

void FLightS::build(){
	p_sl = p_lightManipulator.staticLight_add(StaticLight{ getMiddlePos() + glm::ivec2(1, 0), RE::Colour{31u, 0u, 0u, 31u}, 0.25f, 0.25f });
}

void FLightS::destroy(){
	p_lightManipulator.staticLight_remove(p_sl);
}
