#pragma once
#include <GL/glew.h>

//Physics steps per second
const int PHYSICS_STEPS_PER_SECOND = 50;

//Single block dimensions
#define uvec2_BLOCK_SIZE glm::uvec2(16u, 16u)
#define ivec2_BLOCK_SIZE glm::ivec2(16, 16)
#define vec2_BLOCK_SIZE glm::vec2(16.0f, 16.0f)

//Following must have same values, must not be bigger than 256, should have integer value and overall should not be changed :-D
#define float_NUMBER_OF_BLOCKS 256.0f
#define float_NUMBER_OF_BLOCK_VARIATIONS 256.0f

#define float_NUMBER_OF_WALLS 256.0f
#define float_NUMBER_OF_WALL_VARIATIONS 256.0f

namespace light {
const unsigned int MAX_RANGE = 30u;
}


enum class BLOCK_ID : uint8_t {
	AIR = 0, STONE = 1, DIRT = 2, GRASS = 3,
	BEDROCK = 4, LEAVES = 5, TECHBLOCK = 6, COAL = 7,
	HEMATITE = 8, BAUXITE = 9, COPPER_ORE = 10, CASSITERITE = 11,
	GOLD_ORE = 12, SPODUMENE = 13, QUARTZ_SANDSTONE = 14, BRICK = 15,
	RAW_WOOD = 16, WOODEN_PLANKS = 17, COLD_STONE = 18
};

enum class WALL_ID : uint8_t {
	AIR = 0, STONE = 1, WOOD = 2, DIRT = 3,
	LEAVES = 4, COLD_STONE = 5
};

//OpenGL Texture unit reservation
struct TextureUnitReservation {
	TextureUnitReservation(GLuint index) : m_index(index) {}

	void setActive() const { glActiveTexture(m_index + GL_TEXTURE0); }
	GLint index() const { return m_index; }
private:
	GLint m_index;
};


//0: volatile unit, used by sprite batches, font batches etc.; MUST STAY ACTIVE BY DEFAULT (reset active texture unit back to 0 immediately after you have worked with other units)
const TextureUnitReservation TEX_UNIT_VOLATILE = {0};
//1: block atlas
const TextureUnitReservation TEX_UNIT_BLOCK_ATLAS = {1};
//2: wall atlas
const TextureUnitReservation TEX_UNIT_WALL_ATLAS = {2};
//3: world texture
const TextureUnitReservation TEX_UNIT_WORLD_TEXTURE = {3};

//4: worldDrawer: diaphragm
const TextureUnitReservation TEX_UNIT_DIAPHRAGM = {4};
//5: worldDrawer: lighting
const TextureUnitReservation TEX_UNIT_LIGHTING = {5};
//6: worldDrawer: finished (diaphragm and lighting combined)
const TextureUnitReservation TEX_UNIT_LIGHT_FINISHED = {6};

//7: chunkGenerator: first tiles unit used for pingponging
const TextureUnitReservation TEX_UNIT_CHUNK_TILES0 = {7};
//8: chunkGenerator: second tiles unit used for pingponging
const TextureUnitReservation TEX_UNIT_CHUNK_TILES1 = {8};
//9: chunkGenerator: material
const TextureUnitReservation TEX_UNIT_CHUNK_MATERIAL = {9};