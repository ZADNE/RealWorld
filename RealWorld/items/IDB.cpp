#include "IDB.hpp"

#include <fstream>

#define writeBin(x) write(reinterpret_cast<const char *>(&##x##), sizeof(##x##))
#define readBin(x) read(reinterpret_cast<char *>(&##x##), sizeof(##x##))

std::vector<ItemMetadata> IDB::m_itemMetadata;

const ItemMetadata& IDB::g(I_ID ID){
	return m_itemMetadata[(ulong)ID];
}

void IDB::init() {
	std::ifstream file("items.dat", std::ios::binary | std::ios::in);
	if (file.fail()) {
		return;
	}
	file.seekg(0, std::ios::end);
	auto fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	fileSize -= file.tellg();

	fileSize -= 4u;
	file.seekg(4, std::ios::beg);
	m_itemMetadata.resize((unsigned int)fileSize / ItemMetadata::saveSize());
	for (size_t i = 0u; i < (size_t)fileSize; i += ItemMetadata::saveSize()) {
		file.readBin(m_itemMetadata[i / ItemMetadata::saveSize()].maxStack);
		file.readBin(m_itemMetadata[i / ItemMetadata::saveSize()].textureAtlas);
		file.readBin(m_itemMetadata[i / ItemMetadata::saveSize()].spriteIndex);
		file.readBin(m_itemMetadata[i / ItemMetadata::saveSize()].drawScale);
		file.readBin(m_itemMetadata[i / ItemMetadata::saveSize()].type);
		file.readBin(m_itemMetadata[i / ItemMetadata::saveSize()].typeIndex);
	}
}

#undef writeBin
#undef readBin