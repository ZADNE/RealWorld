#include <RealWorld/items/IDB.hpp>

#include <fstream>

template<class T> 
void writeBinary(std::ofstream& file, const T& x){
	file.write(reinterpret_cast<const char *>(&x), sizeof(x));
}

template<class T>
void readBinary(std::ifstream& file, T& x){
	file.read(reinterpret_cast<char *>(&x), sizeof(x));
}

std::vector<ItemMetadata> IDB::m_itemMetadata;

const ItemMetadata& IDB::g(I_ID ID){
	return m_itemMetadata[static_cast<unsigned int>(ID)];
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
		readBinary(file, m_itemMetadata[i / ItemMetadata::saveSize()].maxStack);
		readBinary(file, m_itemMetadata[i / ItemMetadata::saveSize()].textureAtlas);
		readBinary(file, m_itemMetadata[i / ItemMetadata::saveSize()].spriteIndex);
		readBinary(file, m_itemMetadata[i / ItemMetadata::saveSize()].drawScale);
		readBinary(file, m_itemMetadata[i / ItemMetadata::saveSize()].type);
		readBinary(file, m_itemMetadata[i / ItemMetadata::saveSize()].typeIndex);
	}
}