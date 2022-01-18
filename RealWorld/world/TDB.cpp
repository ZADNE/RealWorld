#include <RealWorld/world/TDB.hpp>

#include <fstream>

template<class T> 
void writeBinary(std::ofstream& file, const T& x){
	file.write(reinterpret_cast<const char *>(&x), sizeof(x));
}

template<class T>
void readBinary(std::ifstream& file, T& x){
	file.read(reinterpret_cast<char *>(&x), sizeof(x));
}

std::vector<BlockMetadata> TDB::m_blockMetadata;
std::vector<WallMetadata> TDB::m_wallMetadata;

const BlockMetadata& TDB::gb(BLOCK_ID ID) {
	return m_blockMetadata[(ulong)ID];
}

const WallMetadata& TDB::gw(WALL_ID ID) {
	return m_wallMetadata[(ulong)ID];
}

void TDB::init() {
	std::ifstream file("blocks.dat", std::ios::binary | std::ios::in);
	if (file.fail()) { return; }
	file.seekg(0, std::ios::end);
	auto fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	fileSize -= file.tellg();

	fileSize -= 4u;
	file.seekg(4, std::ios::beg);
	m_blockMetadata.resize(std::numeric_limits<std::underlying_type_t<BLOCK_ID>>::max());
	for (size_t i = 0u; i < (size_t)fileSize; i += BlockMetadata::saveSize()) {
		readBinary(file, m_blockMetadata[i / BlockMetadata::saveSize()].hardness);
		readBinary(file, m_blockMetadata[i / BlockMetadata::saveSize()].toughness);
		readBinary(file, m_blockMetadata[i / BlockMetadata::saveSize()].itemID);
	}
	file.close();

	file.open("walls.dat", std::ios::binary | std::ios::in);
	if (file.fail()) { return; }
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	fileSize -= file.tellg();

	fileSize -= 4u;
	file.seekg(4, std::ios::beg);
	m_wallMetadata.resize(std::numeric_limits<std::underlying_type_t<WALL_ID>>::max());
	for (size_t i = 0u; i < (size_t)fileSize; i += WallMetadata::saveSize()) {
		readBinary(file, m_wallMetadata[i / WallMetadata::saveSize()].hardness);
		readBinary(file, m_wallMetadata[i / WallMetadata::saveSize()].toughness);
		readBinary(file, m_wallMetadata[i / WallMetadata::saveSize()].itemID);
	}
}