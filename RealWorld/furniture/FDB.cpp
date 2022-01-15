#include "FDB.hpp"

#include <fstream>

#include <RealEngine/ResourceManager.hpp>

#define writeBin(x) write(reinterpret_cast<const char *>(&##x##), sizeof(##x##))
#define readBin(x) read(reinterpret_cast<char *>(&##x##), sizeof(##x##))

std::vector<FDB::CommonInfo> FDB::m_commonInfo;

std::array<RE::TexturePtr, 3> FDB::m_textures;

void FDB::init() {
	std::ifstream file("furniture.dat", std::ios::binary | std::ios::in);
	if (file.fail()) {
		return;
	}
	file.seekg(0, std::ios::end);
	auto fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	fileSize -= file.tellg();

	fileSize -= 4u;
	file.seekg(4, std::ios::beg);
	FurEntry fe;
	m_commonInfo.resize((unsigned int)fileSize / FurEntry::saveSize());
	for (size_t i = 0u; i < (size_t)fileSize; i += FurEntry::saveSize()) {
		file.readBin(m_commonInfo[i / FurEntry::saveSize()].type);
		file.readBin(m_commonInfo[i / FurEntry::saveSize()].index);
		file.readBin(fe.dimx);
		file.readBin(fe.dimy);
		file.readBin(fe.sprite);
		file.readBin(m_commonInfo[i / FurEntry::saveSize()].imageSpeed);

		m_commonInfo[i / FurEntry::saveSize()].dim = glm::ivec2((int)fe.dimx, (int)fe.dimy);
		m_commonInfo[i / FurEntry::saveSize()].sprite = (float)fe.sprite;

		file.readBin(m_commonInfo[i / FurEntry::saveSize()].itemID);
		file.readBin(m_commonInfo[i / FurEntry::saveSize()].placement);
	}

	m_textures = {
		RE::RM::getTexture("ruler"),
		RE::RM::getTexture("placedChest"),
		RE::RM::getTexture("placedLight")
	};
}

F_TYPE FDB::getType(size_t totalIndex) {
	return m_commonInfo[totalIndex].type;
}

uchar FDB::getSpecificIndex(size_t totalIndex) {
	return m_commonInfo[totalIndex].index;
}

size_t FDB::getTotalIndex(F_TYPE type, size_t specificIndex) {
	for (size_t i = 0u; i < m_commonInfo.size(); ++i) {
		if (m_commonInfo[i].index == specificIndex && m_commonInfo[i].type == type) {
			return i;
		}
	}
	//Not found :-/
	return 0u;
}

glm::ivec2 FDB::getDims(size_t totalIndex) {
	return m_commonInfo[totalIndex].dim;
}

RE::TexturePtr FDB::getTexture(size_t totalIndex) {
	switch (m_commonInfo[totalIndex].type) {
	case F_TYPE::NONE:
		return m_textures[0];
	case F_TYPE::CHEST:
		return m_textures[1];
	case F_TYPE::LIGHT_SOURCE:
		return m_textures[2];
	default:
		return m_textures[0];
	}
}

float FDB::getSprite(size_t totalIndex) {
	return m_commonInfo[totalIndex].sprite;
}

float FDB::getImageSpeed(size_t totalIndex) {
	return m_commonInfo[totalIndex].imageSpeed;
}

I_ID FDB::getItemID(size_t totalIndex) {
	return m_commonInfo[totalIndex].itemID;
}

Placement FDB::getPlacement(size_t totalIndex) {
	return m_commonInfo[totalIndex].placement;
}

#undef writeBin
#undef readBin