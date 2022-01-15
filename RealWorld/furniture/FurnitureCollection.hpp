#pragma once
#include <vector>

#include "FX/FStatic.hpp"
#include "FX/FChest.hpp"
#include "FX/FCraftS.hpp"
#include "FX/FFurnace.hpp"
#include "FX/FLightS.hpp"


struct FurnitureCollection {
	std::vector<FStatic> f0;
	std::vector<FChest> f1;
	std::vector<FCraftS> f2;
	std::vector<FFurnace> f3;
	std::vector<FLightS> f4;

	void clear() {
		f0.clear();
		f1.clear();
		f2.clear();
		f3.clear();
		f4.clear();
	}
};