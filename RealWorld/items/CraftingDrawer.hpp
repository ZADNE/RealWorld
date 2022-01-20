#pragma once
#include <vector>
#include <list>

#include <glm/vec2.hpp>

#include <RealEngine/ResourceManager.hpp>
#include <RealEngine/Vertex.hpp>
#include <RealEngine/Surface.hpp>
#include <RealEngine/ShaderProgramCache.hpp>
#include <RealEngine/default_shaders.hpp>

#include <RealWorld/items/ItemSprite.hpp>
#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/metadata.hpp>

class ItemCombinator;
class ItemInstruction;
namespace RE {
class SpriteBatch;
class SpriteFont;
}

class CraftingDrawer {
public:
	CraftingDrawer();
	~CraftingDrawer();

	//All objects must be already initialized
	void init(RE::SpriteBatch* spriteBatch, const glm::vec2& windowSize, const RE::FontSeed& font);
	void resizeWindow(const glm::vec2& newWindowSize);

	//Disconnects from previous and connects to new item combinator
	//Passing nullptr effectively disconnects from any item combinator
	void connectToItemCombinator(ItemCombinator* itemCombinator);

	void reloadEverything();

	//Which instructions depends on the position of cursor
	void craft(unsigned int times);
	//Which instructions depends on the position of cursor
	void cancel();

	//Positive number means shifting instructions to the right
	//Negative number means shifting instructions to the left
	void roll(int positions);

	void step(const glm::ivec2& absCursorPos);

	void draw();

	void switchDraw() { m_shouldDraw = !m_shouldDraw; };

	//To be called from connected ItemCombinator
	void instructionFinished();
private:
	void reloadInstructionDescription(const ItemInstruction* instruction);

	ItemCombinator* m_itemCombinator = nullptr;
	RE::SpriteBatch* m_spriteBatch = nullptr;
	RE::FontSeed m_font;

	glm::vec2 m_absCursorPos = glm::vec2(0.0f, 0.0f);

	float m_rowY = 500.0f;//Y component of the line on which all the instructions are displayed
	float m_offset = 0.0f;
	float m_offsetDraw = 0.0f;
	RE::TexturePtr m_mainSlotTex = RE::RM::getTexture("mainSlot");
	glm::vec2 m_windowSize;
	glm::vec2 m_mainSlotDims = m_mainSlotTex->getSubimageDims();
	glm::vec2 m_paddingSlots = glm::vec2(8.0f, 8.0f);

	int m_possibleInstrsCursorIndex = -100;
	bool m_pointingAtSomePS = false;//PS = possible instructions
	unsigned int m_descriptionWidth;
	std::vector<ItemSprite> m_inputDescrIS;
	std::vector<ItemSprite> m_outputDescrIS;
	float m_queueScale = 0.75f;
	size_t m_maxQueueRows;
	const ItemInstruction* m_description = nullptr;
	RE::Surface m_surfaceDescription{{{RE::TextureFlags::RGBA_NU_NEAR_NEAR_EDGE}}};
	//0 texture: below dynamic sprites
	//1 texture: above dynamic sprites

	std::list<std::pair<ItemSprite, int>> m_queueIS;//For drawing of the queue | int determines how many times it is there (0 => it is a padding)
	bool m_pointingAtSomeQ = false;//Q = queue


	std::vector<std::pair<const ItemInstruction*, int>> m_craftableInstructions;//Set of all instructions that can be crafted at least once, int identifies how many times the instruction can be crafted
	std::vector<ItemSprite> m_itemSprites;
	RE::Colour m_defColour{255u, 255u, 255u, 255u};
	RE::Colour m_amountColour{255u, 0u, 0u, 255u};
	RE::ShaderProgram m_PTSBelow{{.vert = RE::vert_sprite, .frag = RE::frag_sprite}};
	RE::ShaderProgram m_PTSAbove{{.vert = RE::vert_sprite, .frag = shaders::standardOut1_frag}};
	bool m_shouldDraw = false;
};