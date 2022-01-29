#pragma once
#include <RealWorld/furniture/FDB.hpp>

#undef NO_ERROR
enum class BuildError {
	NO_ERROR, OCCUPIED, BAD_PLACEMENT, BAD_TYPE, NOT_FOUND
};

//Can be used for accessing buildings in BuildingRegister OR to store the error why the building could not be built/destoryed

//abrv. FurnitureIndex
class FIndex {
	friend class FurnitureManager;
	friend class Planner;
	friend struct QTFurniture;
public:
	FIndex();
	~FIndex();

	bool isError() const;
	BuildError getError() const;

	F_TYPE getType() const;
	size_t getIndex() const;

private:
	FIndex(BuildError error);
	FIndex(F_TYPE type, size_t index);

	F_TYPE m_type;
	size_t m_index;//Index inside furniture register
};