#include <RealWorld/furniture/FurnitureManager.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/furniture/FDB.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>

#define onVectorByType(type, oper)	switch (type) { \
										case F_TYPE::STATIC:{ \
											auto& v = m_fs.fc.f0; \
											oper; \
											break; }\
										case F_TYPE::CHEST:{ \
											auto& v = m_fs.fc.f1; \
											oper; \
											break; }\
										case F_TYPE::CRAFTING_STATION:{ \
											auto& v = m_fs.fc.f2; \
											oper; \
											break; }\
										case F_TYPE::FURNACE:{ \
											auto& v = m_fs.fc.f3; \
											oper; \
											break; }\
										case F_TYPE::LIGHT_SOURCE:{ \
											auto& v = m_fs.fc.f4; \
											oper; \
											break; }\
										default: \
											throw; \
										} \

const size_t FurnitureManager::FurStore::offset[] = {offsetof(FurStore, fc.f0), offsetof(FurStore, fc.f1), offsetof(FurStore, fc.f2), offsetof(FurStore, fc.f3), offsetof(FurStore, fc.f4)};
const size_t FurnitureManager::FurStore::sizeOfElement[] = {sizeof(FStatic), sizeof(FChest), sizeof(FCraftS), sizeof(FFurnace), sizeof(FLightS)};


FurnitureManager::FurnitureManager(RE::SpriteBatch& spriteBatch, const glm::ivec2& worldDims, World& world, const glm::vec2& viewSizePx) :
	m_spriteBatch(spriteBatch),
	m_world(world) {
	FStatic::initStatics(spriteBatch);
	m_fsQT = Quadtree<QTFurniture>{0u, QTFurniture{ {0, 0}, worldDims }};

	resizeView(viewSizePx);
}

FurnitureManager::~FurnitureManager() {

}

void FurnitureManager::resizeView(const glm::vec2& viewSizePx) {
	m_viewSizeBc = glm::ivec2(1, 1) + glm::ivec2(glm::ceil(viewSizePx / TILE_SIZE));
	m_mustRealoadDrawable = true;
}

BuildError FurnitureManager::accepts(const FStatic& furniture) {
	//First we check if this spot is blocked by another furniture
	auto bl = furniture.getBotLeft();
	auto dims = furniture.getDims();
	QTFurniture b{bl, dims};
	if (!m_fsQT.empty()) {
		std::vector<QTFurniture> possibleCollisions;
		m_fsQT.retrieve(possibleCollisions, b);
		for (auto& bing : possibleCollisions) {
			if (bing.collidesWith(b)) {
				return BuildError::OCCUPIED;
			}
		}
	}
	//Then we check if this spot is blocked by blocks
	if (m_world.getMax(chunk::BLOCK_VALUES::BLOCK, bl, bl + dims - glm::ivec2(1, 1)) > LAST_NONSOLIDBLOCK) {
		return BuildError::OCCUPIED;
	}
	//Now we check placement
	bool ground = (m_world.getMin(chunk::BLOCK_VALUES::BLOCK, glm::ivec2(bl.x, bl.y - 1), glm::ivec2(bl.x + dims.x - 1, bl.y - 1)) > LAST_NONSOLIDBLOCK);
	bool side = ((m_world.getMin(chunk::BLOCK_VALUES::BLOCK, glm::ivec2(bl.x - 1, bl.y), glm::ivec2(bl.x - 1, bl.y + dims.y - 1)) > LAST_NONSOLIDBLOCK)
				|| (m_world.getMin(chunk::BLOCK_VALUES::BLOCK, glm::ivec2(bl.x + dims.x, bl.y), glm::ivec2(bl.x + dims.x, bl.y + dims.y - 1)) > LAST_NONSOLIDBLOCK));
	bool ceiling = (m_world.getMin(chunk::BLOCK_VALUES::BLOCK, glm::ivec2(bl.x, bl.y + dims.y), glm::ivec2(bl.x + dims.x - 1, bl.y + dims.y)) > LAST_NONSOLIDBLOCK);
	uchar indoors = 1;
	if (m_world.getMax(chunk::BLOCK_VALUES::WALL, bl, bl + dims - glm::ivec2(1, 1)) == 0u) {
		indoors = 0;//outdoors
	} else if (m_world.getMin(chunk::BLOCK_VALUES::WALL, bl, bl + dims - glm::ivec2(1, 1)) > 0u) {
		indoors = 2;//indoors
	}
	if (!FDB::getPlacement(furniture.getTotalIndex()).canBePlaced(ground, ceiling, side, indoors)) {
		return BuildError::BAD_PLACEMENT;
	}

	//No problem found
	return BuildError::NO_ERROR;
}

const FStatic* FurnitureManager::getcFurniture(const glm::ivec2& posSq) const {
	return getcFurniture(getFIndex(posSq));
}

FIndex FurnitureManager::getFIndex(const glm::ivec2& posSq) const {
	std::vector<QTFurniture> found;
	QTFurniture b{posSq, glm::ivec2(1, 1)};
	m_fsQT.retrieve(found, b);
	for (auto& furniture : found) {
		if (b.collidesWith(furniture)) {
			return furniture.fIndex;
		}
	}
	//There is no furniture with such position
	return FIndex{BuildError::NOT_FOUND};
}

FIndex FurnitureManager::build(const FStatic& furniture, bool actuallyBuild) {
	if (furniture.getType() == F_TYPE::NONE) {
		return FIndex(BuildError::NO_ERROR);//Silently ignoring
	}

	//First we need to check whether the furniture can be placed there
	BuildError be = accepts(furniture);
	if (be == BuildError::NO_ERROR) {
		if (actuallyBuild) {//If placing furniture, if not just testing
			return addFurniture(furniture);
		}
		return FIndex(furniture.getType(), 0u);
	} else {
		return FIndex(be);
	}
}

const FStatic* FurnitureManager::getcFurniture(const FIndex fIndex) const {
	return getFurniture(fIndex);
}

std::pair<size_t, glm::ivec2> FurnitureManager::destroy(const glm::ivec2& posSq) {
	return destroy(getFIndex(posSq));
}

std::pair<size_t, glm::ivec2> FurnitureManager::destroy(const FIndex fIndex) {
	FStatic* furniture = getFurniture(fIndex);
	if (!furniture) {
		return std::make_pair(0u, glm::ivec2{0, 0});//getFIndex() returned erroneous FIndex (a furniture with such position probably does not exist)
	}

	destroySimple(fIndex);
	return std::make_pair(furniture->getTotalIndex(), furniture->getBotLeft());
}

void FurnitureManager::clear() {
	m_fsQT.clear();//Quadtree
	callOnEach(&FStatic::destroy);
	m_fs.clear();//FurnitureStore
	for (auto& pqueue : m_fsFI) {//Free indexes
		pqueue = {};
	}
}

void FurnitureManager::step(const glm::vec2& botLeftPosPx) {
	glm::ivec2 newBL = pxToBc((glm::ivec2)botLeftPosPx);
	if (m_botLeftOfViewBc != newBL || m_mustRealoadDrawable) {
		//Updating list of furniture that should be drawn
		m_botLeftOfViewBc = newBL;
		m_QTFurToDraw.clear();
		m_fsQT.retrieve(m_QTFurToDraw, QTFurniture{m_botLeftOfViewBc, m_viewSizeBc});
		m_furToDraw.clear();
		m_furToDraw.reserve(m_QTFurToDraw.size());
		for (size_t i = 0u; i < m_QTFurToDraw.size(); ++i) {
			m_furToDraw.emplace_back(getFurniture(m_QTFurToDraw[i].fIndex));
		}
		m_mustRealoadDrawable = false;
	}

	callOnEach(&FStatic::step);
}

void FurnitureManager::draw() {
	for (auto& f : m_furToDraw) {
		f->draw();
	}
}

void FurnitureManager::adoptFurnitureCollection(const FurnitureCollection& fc) {
	clear();
	for (auto& fur : fc.f0)
		addFurniture(fur);
	for (auto& fur : fc.f1)
		addFurniture(fur);
	for (auto& fur : fc.f2)
		addFurniture(fur);
	for (auto& fur : fc.f3)
		addFurniture(fur);
	for (auto& fur : fc.f4)
		addFurniture(fur);
}

void FurnitureManager::gatherFurnitureCollection(FurnitureCollection& fc) const {
	auto FI0 = m_fsFI[0];
	FI0.push(m_fs.fc.f0.size());
	for (size_t i = 0u; i < m_fs.fc.f0.size(); ++i) {
		if (i != FI0.top()) {
			fc.f0.push_back(m_fs.fc.f0[i]);
		} else {
			FI0.pop();
		}
	}
	auto FI1 = m_fsFI[1];
	FI1.push(m_fs.fc.f1.size());
	for (size_t i = 0u; i < m_fs.fc.f1.size(); ++i) {
		if (i != FI1.top()) {
			fc.f1.push_back(m_fs.fc.f1[i]);
		} else {
			FI1.pop();
		}
	}
	auto FI2 = m_fsFI[2];
	FI2.push(m_fs.fc.f2.size());
	for (size_t i = 0u; i < m_fs.fc.f2.size(); ++i) {
		if (i != FI2.top()) {
			fc.f2.push_back(m_fs.fc.f2[i]);
		} else {
			FI2.pop();
		}
	}
	auto FI3 = m_fsFI[3];
	FI3.push(m_fs.fc.f3.size());
	for (size_t i = 0u; i < m_fs.fc.f3.size(); ++i) {
		if (i != FI3.top()) {
			fc.f3.push_back(m_fs.fc.f3[i]);
		} else {
			FI3.pop();
		}
	}
	auto FI4 = m_fsFI[4];
	FI4.push(m_fs.fc.f4.size());
	for (size_t i = 0u; i < m_fs.fc.f4.size(); ++i) {
		if (i != FI4.top()) {
			fc.f4.push_back(m_fs.fc.f4[i]);
		} else {
			FI4.pop();
		}
	}
}

FStatic* FurnitureManager::getFurniture(const FIndex fIndex) const {
	unsigned int type = (unsigned int)fIndex.getType();
	if (type >= (unsigned int)F_TYPE::LAST) {
		//The fIndex is invaid
		return nullptr;
	}

	return (FStatic*)m_fs.getElem(type, (unsigned int)fIndex.getIndex());
}

FIndex FurnitureManager::addFurniture(const FStatic& furniture) {
	unsigned int type = (unsigned int)furniture.getType();
	FIndex newFIndex;
	std::vector<FStatic>* vec = (std::vector<FStatic>*)m_fs.getVec(type);
	if (m_fsFI[type].empty()) {
		//No free indexes in the queue
		newFIndex = FIndex(furniture.getType(), vec->size());
		onVectorByType(furniture.getType(), v.push_back(furniture); v.back().build());
	} else {
		//Placing the furniture to the index of a removed furniture
		newFIndex = FIndex(furniture.getType(), m_fsFI[type].top());
		auto i = newFIndex.getIndex();
		m_fsFI[type].pop();
		onVectorByType(furniture.getType(), v[i] = furniture; v[i].build());
	}
	m_fsQT.insert(QTFurniture{furniture.getBotLeft(), furniture.getDims(), newFIndex});
	m_mustRealoadDrawable = true;
	return newFIndex;
}

bool FurnitureManager::destroySimple(const glm::ivec2& posSq) {
	return destroySimple(getFIndex(posSq));
}

bool FurnitureManager::destroySimple(const FIndex fIndex) {
	if (fIndex.isError()) { return false; }//Cannot destroy by an erroneous FIndex
	//Call destroy in destroyed furniture
	F_TYPE type = fIndex.getType();
	size_t index = fIndex.getIndex();
	onVectorByType(type, v[index].destroy());
	//We add the index to free indexes
	m_fsFI[(unsigned int)type].push(index);
	//Removing the fIndex from QT
	auto f = getFurniture(fIndex);
	m_fsQT.erase(QTFurniture{f->getBotLeft(), f->getDims()});
	m_mustRealoadDrawable = true;
	return true;
}

#undef onVectorByType