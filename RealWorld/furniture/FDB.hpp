#pragma once
#include <vector>
#include <array>

#include <glm/vec2.hpp>

#include <RealEngine/ResourceManager.hpp>

#include "../metadata.hpp"
#include "../items/IDB.hpp"

namespace RE {
class Texture;
}

#undef ERROR
enum class F_TYPE : uchar {
	STATIC,
	CHEST,
	CRAFTING_STATION,
	FURNACE,
	LIGHT_SOURCE,

	LAST,

	NONE = 255u,
	ERROR = 255u,
};

/*
enum class PLACEMENT: uchar {
	FFFF0 = 0u,
	TFFF0,
	FTFF0,
	TTFF0,
	FFTF0,
	TFTF0,
	FTTF0,
	TTTF0,
	FFF_0,
	TFF_0,
	FTF_0,
	TTF_0,
	FFT_0,
	TFT_0,
	FTT_0,
	TTT_0,
	FFFT0,
	TFFT0,
	FTFT0,
	TTFT0,
	FFTT0,
	TFTT0,
	FTTT0,
	TTTT0,
	FFFF1 = 32u,
	TFFF1,
	FTFF1,
	TTFF1,
	FFTF1,
	TFTF1,
	FTTF1,
	TTTF1,
	FFF_1,
	TFF_1,
	FTF_1,
	TTF_1,
	FFT_1,
	TFT_1,
	FTT_1,
	TTT_1,
	FFFT1,
	TFFT1,
	FTFT1,
	TTFT1,
	FFTT1,
	TFTT1,
	FTTT1,
	TTTT1,
	FFFF2 = 64u,
	TFFF2,
	FTFF2,
	TTFF2,
	FFTF2,
	TFTF2,
	FTTF2,
	TTTF2,
	FFF_2,
	TFF_2,
	FTF_2,
	TTF_2,
	FFT_2,
	TFT_2,
	FTT_2,
	TTT_2,
	FFFT2,
	TFFT2,
	FTFT2,
	TTFT2,
	FFTT2,
	TFTT2,
	FTTT2,
	TTTT2,
	FFFF3 = 96u,
	TFFF3,
	FTFF3,
	TTFF3,
	FFTF3,
	TFTF3,
	FTTF3,
	TTTF3,
	FFF_3,
	TFF_3,
	FTF_3,
	TTF_3,
	FFT_3,
	TFT_3,
	FTT_3,
	TTT_3,
	FFFT3,
	TFFT3,
	FTFT3,
	TTFT3,
	FFTT3,
	TFTT3,
	FTTT3,
	TTTT3
};
*/
struct Placement {
	//indoors: 0 = no walls behind | 1 = some walls behind | 2 = every wall behind
	bool canBePlaced(bool grounD, bool ceilinG, bool sidE, uchar indoorS) const {
		if (indoorS != indoors && indoors != 1) {//Bad background
			return false;
		}
		uchar cmet = 0u;
		if (grounD && ground) { cmet++; }
		if (ceilinG && ceiling) { cmet++; }
		if (sidE && side) { cmet++; }
		if (cmet >= conditionsToBeMet) {
			return true;
		}
		return false;
	};

	uchar ground : 1,//1 = must touch ground | 0 = does not have to touch ground
		ceiling : 1,//1 = must touch ceiling | 0 = does not have to touch ceiling
		side : 1,//1 = must touch from sides | 0 = does not have to touch from sides
		indoors : 2,//0 = must not be inside | 1 = do not care | 2 = must be inside (in front of walls)
		conditionsToBeMet : 2;//0 = none of the conditions {1, 2, 3} have to be met | 1 = one have to be met | 2 = two have to | 3 = three have to
};

//FurnitureDatabase
class FDB {
public:
	static void init();

	static F_TYPE getType(size_t totalIndex);//Quick
	static uchar getSpecificIndex(size_t totalIndex);//Quick
	static size_t getTotalIndex(F_TYPE type, size_t specificIndex);//Slow (linear search; 0 returned if not found)
	static glm::ivec2 getDims(size_t totalIndex);
	static RE::TexturePtr getTexture(size_t totalIndex);
	static float getSprite(size_t totalIndex);
	static float getImageSpeed(size_t totalIndex);
	static I_ID getItemID(size_t totalIndex);
	static Placement getPlacement(size_t totalIndex);

private:
	struct FurEntry {
		constexpr static size_t saveSize() { return (sizeof(F_TYPE) + 2u * sizeof(char) + 2u * sizeof(uchar) + sizeof(float) + sizeof(I_ID) + sizeof(Placement)); }
		F_TYPE type = F_TYPE::NONE;
		uchar index = 0u;
		char dimx = 1;
		char dimy = 1;
		uchar sprite = 0u;
		float imageSpeed = 1.0f;
		I_ID itemID = I_ID::EMPTY;
		Placement placement;
	};

	struct CommonInfo {
		F_TYPE type;
		uchar index;
		glm::ivec2 dim;//dimensions in Bc
		float sprite;
		float imageSpeed;
		I_ID itemID;//ID of the item that is dropped when this is dismantled
		Placement placement;//Where the furniture may be placed
	};

	static std::vector<CommonInfo> m_commonInfo;
	static std::array<RE::TexturePtr, 3> m_textures;
};

