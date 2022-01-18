#pragma once
#include <vector>
#include <queue>
#include <tuple>
#include <fstream>

#include <glm/vec2.hpp>

#include <RealEngine/tupleUp.hpp>

#include <RealWorld/furniture/FIndex.hpp>
#include <RealWorld/furniture/Quadtree.hpp>
#include <RealWorld/furniture/FurnitureCollection.hpp>


#define callOnEachOfType(x, func, args)		auto FI##x = m_fsFI[x]; \
											FI##x.push(m_fs.fc.f##x.size()); \
											for (size_t i = 0u; i < m_fs.fc.f##x.size(); ++i) { \
												if (i != FI##x.top()) { \
													(m_fs.fc.f##x[i].*func)(args); \
												} else { \
													FI##x.pop(); \
												} \
											} \

#define callWithEachOfType(x, func, args)	auto FI##x = m_fsFI[x]; \
											FI##x.push(m_fs.fc.f##x.size()); \
											for (size_t i = 0u; i < m_fs.fc.f##x.size(); ++i) { \
												if (i != FI##x.top()) { \
													(this->*func)(&m_fs.fc.f##x[i], args); \
												} else { \
													FI##x.pop(); \
												} \
											} \


namespace RE {
	class SpriteBatch;
}

class World;

struct QTFurniture {
	QTFurniture() {}
	QTFurniture(const glm::ivec2& botLefT, const glm::ivec2& diM) : botLeft(botLefT), dim(diM) {}
	QTFurniture(const glm::ivec2& botLefT, const glm::ivec2& diM, const FIndex& fIndeX) : botLeft(botLefT), dim(diM), fIndex(fIndeX) {}

	inline bool operator<(const QTFurniture& rhs) const {
		return ((botLeft.x + (botLeft.y << 16)) < (rhs.botLeft.x + (rhs.botLeft.y << 16)));
	}

	inline bool collidesWith(const QTFurniture& oB) const {
		if (botLeft.x < (oB.botLeft.x + oB.dim.x) && (botLeft.x + dim.x) > oB.botLeft.x&&
			botLeft.y < (oB.botLeft.y + oB.dim.y) && (botLeft.y + dim.y) > oB.botLeft.y) {
			return true;
		}
		return false;
	}

	glm::ivec2 botLeft;
	glm::ivec2 dim;//Although signed, x and y should always be > 0
	FIndex fIndex;
};


class FurnitureRegister {
public:
	FurnitureRegister();
	~FurnitureRegister();

	void init(RE::SpriteBatch* spriteBatch, const glm::ivec2& worldDims, World* world, const glm::vec2& viewSizePx);

	void resizeView(const glm::vec2& viewSizePx);

	//For checking if a furniture can be built there
	BuildError accepts(const FStatic& furniture);

	//Returns nullptr if no furniture with such position exists
	//Returned pointer is not to be stored - can be invalidated as soon as a furniture is added or removed (the fIndex is to be stored)
	const FStatic* getcFurniture(const glm::ivec2& posSq) const;

	//Does not check whether the index is within bounds
	//Returns nullptr if fIndex is invalid
	//Returned pointer is not to be stored - can be invalidated as soon as a furniture is added or removed (the fIndex is to be stored)
	const FStatic* getcFurniture(const FIndex fIndex) const;


	//Returns error FIndex if there is not a building with such position
	FIndex getFIndex(const glm::ivec2& posSq) const;


	//actuallyBuild = false -> for testing whether the furniture can be built there
	FIndex build(const FStatic& furniture, bool actuallyBuild = true);

	//Tries to destory the furniture that has THE POSITION of posSq
	//Returns totalIndex of the destroyed furniture and its botleft or 0s if failed
	std::pair<size_t, glm::ivec2> destroy(const glm::ivec2& posSq);
	//Returns totalIndex of the destroyed furniture and its botleft or 0s if failed
	std::pair<size_t, glm::ivec2> destroy(const FIndex fIndex);

	//Destroys all furniture
	void clear();


	//Should be called each physics step
	void step(const glm::vec2& botLeftPosPx);

	void draw();

	void adoptFurnitureCollection(const FurnitureCollection& fc);
	void gatherFurnitureCollection(FurnitureCollection& fc);
private:
	FStatic* getFurniture(const FIndex fIndex) const;

	FIndex addFurniture(const FStatic& furniture);

	//Returns whether such furniture was found and destroyed
	bool destroySimple(const glm::ivec2& posSq);
	//Returns whether such furniture was found and destroyed
	bool destroySimple(const FIndex fIndex);

	template<class ...Args> void callOnEach(void (FStatic::* func)(Args...), Args... args);
	template<class ...Args> void callWithEach(void (FurnitureRegister::* func)(const FStatic*, Args...), Args... args);

	template<class ...Args> void callOnEach(void (FStatic::* func)(Args...) const, Args... args) const;
	template<class ...Args> void callWithEach(void (FurnitureRegister::* func)(const FStatic*, Args...) const, Args... args) const;

	RE::SpriteBatch* m_spriteBatch = nullptr;
	World* m_world = nullptr;

	//Should guarantee that all fIndexes are valid
	Quadtree<QTFurniture> m_fsQT;

	struct FurStore {
		FurnitureCollection fc;

		void clear() {
			fc.clear();
		}

		//Number of vectors in this struct
		static const size_t numberOfMembers = 5u;
		//Offset of each vector inside this struct in bytes
		static const size_t offset[numberOfMembers];
		//Size of element inside each vector in bytes
		static const size_t sizeOfElement[numberOfMembers];

		inline char* getVec(ulong type) const {
			return ((char*)this + offset[type]);
		}

		inline char* getFirstElem(ulong type) const {
			return (char*)reinterpret_cast<const std::vector<FStatic>*>(getVec(type))->data();
		}

		inline char* getElem(ulong type, ulong index) const {
			return (char*)(getFirstElem(type) + (index * sizeOfElement[type]));
		}
	};

	FurStore m_fs;
	std::priority_queue<size_t, std::vector<size_t>, std::greater<size_t>> m_fsFI[FurStore::numberOfMembers];//Free indexes


	//Drawing-related
	glm::ivec2 m_viewSizeBc;
	glm::ivec2 m_botLeftOfViewBc;
	bool m_mustRealoadDrawable = false;
	std::vector<QTFurniture> m_QTFurToDraw;
	std::vector<FStatic*> m_furToDraw;
};

template<class ...Args>
inline void FurnitureRegister::callOnEach(void(FStatic::* func)(Args...), Args... args) {
	callOnEachOfType(0, func, args...);
	callOnEachOfType(1, func, args...);
	callOnEachOfType(2, func, args...);
	callOnEachOfType(3, func, args...);
	callOnEachOfType(4, func, args...);
}

template<class ...Args>
inline void FurnitureRegister::callWithEach(void(FurnitureRegister::* func)(const FStatic*, Args...), Args... args) {
	callWithEachOfType(0, func, args...);
	callWithEachOfType(1, func, args...);
	callWithEachOfType(2, func, args...);
	callWithEachOfType(3, func, args...);
	callWithEachOfType(4, func, args...);
}

template<class ...Args>
inline void FurnitureRegister::callOnEach(void(FStatic::* func)(Args...) const, Args ...args) const {
	callOnEachOfType(0, func, args...);
	callOnEachOfType(1, func, args...);
	callOnEachOfType(2, func, args...);
	callOnEachOfType(3, func, args...);
	callOnEachOfType(4, func, args...);
}

template<class ...Args>
inline void FurnitureRegister::callWithEach(void(FurnitureRegister::* func)(const FStatic*, Args...) const, Args... args) const {
	callWithEachOfType(0, func, args...);
	callWithEachOfType(1, func, args...);
	callWithEachOfType(2, func, args...);
	callWithEachOfType(3, func, args...);
	callWithEachOfType(4, func, args...);
}


#undef callOnEachOfType
#undef callWithEachOfType