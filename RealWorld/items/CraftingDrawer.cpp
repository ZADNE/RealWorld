#include <RealWorld/items/CraftingDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Font.hpp>
#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/main/Error.hpp>

#include <RealWorld/items/ItemCombinator.hpp>
#include <RealWorld/items/ItemInstruction.hpp>

CraftingDrawer::CraftingDrawer() {

}

CraftingDrawer::~CraftingDrawer() {
	if (m_itemCombinator) {
		m_itemCombinator->connectToCraftingDrawer(nullptr);
	}
}

void CraftingDrawer::init(RE::SpriteBatch* spriteBatch, const glm::vec2& windowSize, const RE::FontSeed& font) {
	m_spriteBatch = spriteBatch;
	m_windowSize = windowSize;
	m_font = font;
	m_rowY = m_windowSize.y / 2.0f;
	m_maxQueueRows = (size_t)(m_rowY / (m_mainSlotDims.y + m_paddingSlots.y) / m_queueScale);
}

void CraftingDrawer::resizeWindow(const glm::vec2& newWindowSize) {
	m_windowSize = newWindowSize;
	m_rowY = m_windowSize.y / 2.0f;
	m_maxQueueRows = (size_t)(m_rowY / (m_mainSlotDims.y + m_paddingSlots.y) / m_queueScale);
}

void CraftingDrawer::connectToItemCombinator(ItemCombinator* itemCombinator) {
	if (m_itemCombinator) {
		m_itemCombinator->connectToCraftingDrawer(nullptr);
	}
	m_itemCombinator = itemCombinator;
	if (m_itemCombinator) {
		m_itemCombinator->connectToCraftingDrawer(this);
		reloadEverything();
	}
}

void CraftingDrawer::reloadEverything() {
	if (!m_itemCombinator) { return; }
	m_itemCombinator->getCraftableInstructions(m_craftableInstructions);
	m_itemSprites.clear();
	for (auto it = m_craftableInstructions.begin(); it != m_craftableInstructions.end(); ++it) {
		for (unsigned int i = 0u; i < it->first->numberOfMainOutputs(); ++i) {
			m_itemSprites.emplace_back((*(*it).first).output(i));
		}
	}
}

void CraftingDrawer::craft(unsigned int times) {
	if (!m_itemCombinator) { return; }
	if (m_description && m_pointingAtSomePS) {
		auto prevBackIt = m_itemCombinator->m_queue.end();
		bool wasEmpty = m_itemCombinator->m_queue.empty();
		if (!wasEmpty) {
			--prevBackIt;
		}
		if (m_itemCombinator->craft(m_description, times, false)) {
			//Instruction successfully added to the queue, now we need to update the m_queueIS
			auto it = m_itemCombinator->m_queue.begin();
			if (!wasEmpty) {
				it = prevBackIt;
				++it;
			}

			while (it != m_itemCombinator->m_queue.end()) {
				if (it->first) {//If it is not border
					m_queueIS.emplace_back(it->first->output(0), it->second);
				} else {//Border
					m_queueIS.emplace_back(I_ID::EMPTY, 0);
				}
				++it;
			}
		}
	}
}

void CraftingDrawer::cancel() {
	if (!m_itemCombinator) { return; }
	//First check if the player is pointing at some instruction (is not pointing at padding)
	glm::ivec2 modulo = (glm::ivec2)(glm::ivec2(m_absCursorPos.x - m_paddingSlots.x, m_absCursorPos.y - m_paddingSlots.y + m_paddingSlots.y * m_queueScale)) % (glm::ivec2)(m_mainSlotDims * m_queueScale + m_paddingSlots * m_queueScale);
	if (modulo.x >= 0 && modulo.x < m_mainSlotDims.x * m_queueScale &&
		modulo.y >= 0 && modulo.y < (m_mainSlotDims.y * m_queueScale)) {
		//Now we need to determine which order and instruction of the order (if some) the player is pointing at
		//Order = row
		size_t order = (size_t)(m_windowSize.y - m_absCursorPos.y - m_paddingSlots.y * m_queueScale) / (size_t)(m_mainSlotDims.y * m_queueScale + m_paddingSlots.y * m_queueScale);
		float offset = 0.0f;
		if (order == 0u) {
			offset = std::floor(m_itemCombinator->m_progress / (float)(m_itemCombinator->m_queue.begin()->first->craftTime() * m_itemCombinator->m_queue.begin()->second) * (m_mainSlotDims.x - m_paddingSlots.x - 1.0f));
		}
		size_t instrOfTheOrder = (size_t)(m_absCursorPos.x - m_paddingSlots.x * m_queueScale + offset) / (size_t)(m_mainSlotDims.x * m_queueScale + m_paddingSlots.x * m_queueScale);
		if (m_itemCombinator->cancel(order, instrOfTheOrder) == CancelError::Success) {//Successfully canceled the instruction/s
			//Now we need to remove them from m_queueIS
			size_t currentOrder = 0u;
			auto it = m_queueIS.begin();
			if (order == 0u) {//Cancelling inside first order
				for (size_t i = 0u; i < instrOfTheOrder; ++i) {
					if (!((++it)->second)) {
						return;//Should not happen
					}
				}
				//it finally points to wanted instruction, now we cancel all orders until we find next padding
				do {
					it = m_queueIS.erase(it);
				} while (it->second);
				return;
			}

			while (it != m_queueIS.end()) {
				++it;
				if (!(it->second) && ++currentOrder == order) {//If it is padding before the wanted order
					++it;//Now it is pointing to the first instruction of the wanted order
					for (size_t i = 0u; i < instrOfTheOrder; ++i) {
						if (!((++it)->second)) {
							return;//Should not happen
						}
					}
					//it finally points to wanted instruction, now we cancel all orders until we find next padding
					do {
						it = m_queueIS.erase(it);
					} while (it->second);
					if (instrOfTheOrder == 0u) {
						m_queueIS.erase(it);//Avoiding double padding
					}
					return;//Everything, that should be, is removed
				}
			}
		}
	}
}

void CraftingDrawer::roll(int positions) {
	m_offset += (float)positions * (m_mainSlotDims.x + m_paddingSlots.x);
}

void CraftingDrawer::step(const glm::ivec2& absCursorPos) {
	m_absCursorPos = absCursorPos;
	//Cursor
	if (m_shouldDraw) {
		int index = (int)floor((m_absCursorPos.x - m_offset) / (m_mainSlotDims.x + m_paddingSlots.x));
		if (m_possibleInstrsCursorIndex != index) {//Moved to different instruction icon
			if (index >= 0 && (size_t)index < m_craftableInstructions.size()) {
				reloadInstructionDescription(m_craftableInstructions[index].first);
			} else {
				m_description = nullptr;
			}
			m_possibleInstrsCursorIndex = index;
		}
		//Now to check if pointing exactly at some inctruction
		if (m_absCursorPos.y >= (m_rowY - m_mainSlotDims.y / 2.0f) && m_absCursorPos.y <= (m_rowY + m_mainSlotDims.y / 2.0f)) {
			//Inside the row of instructions
			//Now to check if not pointing between two instructions (at the padding)
			int modulo = (int)(m_absCursorPos.x - m_offset) % (int)(m_mainSlotDims.x + m_paddingSlots.x);
			if (modulo < (int)m_paddingSlots.x) {
				m_pointingAtSomePS = false; ;//Pointing between two instructions
			} else { m_pointingAtSomePS = true; }
		} else { m_pointingAtSomePS = false; }
	}
	//Step for images
	for (auto& i : m_itemSprites) { i.step(); }//Possible instructions
	for (auto& i : m_inputDescrIS) { i.step(); }//Instruction description inputs
	for (auto& i : m_outputDescrIS) { i.step(); }//Instruction description outputs
	for (auto& i : m_queueIS) { if (i.second) { i.first.step(); } }//Queue

	//Rolling
	m_offsetDraw += (m_offset - m_offsetDraw) / 5.0f;
}

void CraftingDrawer::draw() {
	auto font = RE::RM::getFont(m_font);
	if (!m_shouldDraw) { return; }
	if (!m_itemCombinator) { return; }
	size_t itemSubimageI = 0u;
	size_t instrI = 0u;
	size_t itemSpriteCounter = 0u;
	for (auto it = m_craftableInstructions.begin(); it != m_craftableInstructions.end(); ++it) {
		glm::vec2 botLeftPx = glm::vec2{m_offsetDraw + (float)(instrI + 1) * m_paddingSlots.x + ((float)instrI + 0.5f) * m_mainSlotDims.x, m_rowY};
		//Draw outputs
		for (unsigned int out = 0u; out < it->first->numberOfMainOutputs(); ++out) {
			const Item& item = (*(*it).first).output(out);
			m_spriteBatch->addTexture(m_mainSlotTex.get(), botLeftPx - glm::vec2(0.0f, (float)out) * (m_mainSlotDims + m_paddingSlots), 0);
			m_spriteBatch->addSprite(m_itemSprites[itemSpriteCounter++], botLeftPx - glm::vec2(0.0f, (float)out) * (m_mainSlotDims + m_paddingSlots), 1);
		}
		++instrI;
	}
	glm::vec2 slotAndPadding = m_mainSlotDims + m_paddingSlots;
	//Description for instruction under cursor
	if (m_description && (m_pointingAtSomePS || m_pointingAtSomeQ)) {
		//Offset to avoid instruction "leaving" screen
		glm::vec2 offset = glm::vec2(0.0f, 0.0f);
		if (m_absCursorPos.x - (m_surfaceDescription.getDims().x / 2.0f) < 0.0f) {
			offset.x = m_surfaceDescription.getDims().x / 2.0f - m_absCursorPos.x;
		} else if (m_absCursorPos.x + (m_surfaceDescription.getDims().x / 2.0f) > m_windowSize.x) {
			offset.x = m_windowSize.x - m_absCursorPos.x - (m_surfaceDescription.getDims().x / 2.0f);
		}
		if (m_absCursorPos.y + m_surfaceDescription.getDims().y > m_windowSize.y) {
			offset.y = m_windowSize.y - m_absCursorPos.y - m_surfaceDescription.getDims().y;
		}
		m_spriteBatch->addSurface(m_surfaceDescription, m_absCursorPos + offset, 2, 0);//Slots
		//Sprites
		unsigned int fontHeight = static_cast<unsigned int>(font->getFontHeight());
		float off = ((float)m_description->numberOfInputs() - 1.0f) * -0.5f * slotAndPadding.x;
		for (unsigned int input = 0u; input < m_description->numberOfInputs(); ++input) {
			m_spriteBatch->addSprite(m_inputDescrIS[input], m_absCursorPos + offset + glm::vec2((float)input * slotAndPadding.x + off, m_mainSlotDims.y - m_mainSlotTex->getPivot().y + fontHeight), 3);
		}
		off = ((float)m_description->numberOfOutputs() - 1.0f) * -0.5f * slotAndPadding.x;
		for (unsigned int output = 0u; output < m_description->numberOfOutputs(); ++output) {
			m_spriteBatch->addSprite(m_outputDescrIS[output], m_absCursorPos + offset + glm::vec2((float)output * slotAndPadding.x + off, m_mainSlotDims.y - m_mainSlotTex->getPivot().y + slotAndPadding.y + fontHeight), 3);
		}
		m_spriteBatch->addSurface(m_surfaceDescription, m_absCursorPos + offset, 4, 1);//Numbers
	}
	//Instruction queue
	if (!m_queueIS.empty()) {//If there is something in the queue
		float offsetX = m_mainSlotDims.x * m_queueScale / 2.0f + m_paddingSlots.x;
		float offsetY = m_mainSlotDims.x * m_queueScale / 2.0f + m_paddingSlots.x;
		size_t rows = 1;

		float progress = std::floor(m_itemCombinator->m_progress / (float)(m_itemCombinator->m_queue.begin()->first->craftTime() * m_itemCombinator->m_queue.begin()->second) * (m_mainSlotDims.x - m_paddingSlots.x - 1.0f));

		for (auto& pair : m_queueIS) {
			if (pair.second) {
				glm::vec2 pos = glm::vec2(offsetX - progress, m_windowSize.y - offsetY);
				m_spriteBatch->addTexture(m_mainSlotTex.get(), pos, 5, m_defColour, glm::vec2(m_queueScale, m_queueScale));
				m_spriteBatch->addSprite(pair.first, pos, 6, m_defColour, glm::vec2(m_queueScale, m_queueScale));
				if (pair.second > 1) {
					font->add(*m_spriteBatch, std::to_string(pair.second), glm::vec2(pos.x, pos.y - 40.0f * m_queueScale), 7, m_amountColour, RE::HAlign::MIDDLE);
				}
				offsetX += slotAndPadding.x * m_queueScale;
			} else {
				if (++rows >= m_maxQueueRows) {//To avoid collision with possible instructions row
					break;
				}
				progress = 0.0f;
				offsetX = m_mainSlotDims.x * m_queueScale / 2.0f + m_paddingSlots.x;
				offsetY += slotAndPadding.y * m_queueScale;
			}
		}
	}
}

void CraftingDrawer::instructionFinished() {
	m_queueIS.pop_front();
	if (!m_queueIS.front().second) {//If next is border
		m_queueIS.pop_front();
	}
}

void CraftingDrawer::reloadInstructionDescription(const ItemInstruction* instruction) {
	auto font = RE::RM::getFont(m_font);
	m_description = instruction;
	if (instruction) {
		m_descriptionWidth = std::max(instruction->numberOfInputs(), instruction->numberOfOutputs());
		glm::vec2 slotAndPadding = m_mainSlotDims + m_paddingSlots;
		unsigned int fontHeight = static_cast<unsigned int>(font->getFontHeight());
		//Resize the surface
		m_surfaceDescription.resize(glm::uvec2(m_descriptionWidth * (unsigned int)(slotAndPadding.x) + m_paddingSlots.x, (slotAndPadding.y) * 2u + fontHeight), 2);
		//Set origin to bottom-middle
		m_surfaceDescription.setPivot(glm::vec2(((float)m_descriptionWidth * slotAndPadding.x + m_paddingSlots.x) / 2.0f, 0.0f));

		m_surfaceDescription.setTarget();
		//SLOTS
		m_spriteBatch->begin();
		//Inputs
		float offset = (float)(m_descriptionWidth - instruction->numberOfInputs() + 1u) / 2.0f * slotAndPadding.x + m_paddingSlots.x / 2.0f;
		for (unsigned int input = 0u; input < instruction->numberOfInputs(); ++input) {
			m_spriteBatch->addTexture(m_mainSlotTex.get(), glm::vec2((float)input * slotAndPadding.x + offset, m_mainSlotDims.y - m_mainSlotTex->getPivot().y + fontHeight), 0);
		}
		//Outputs
		offset = (float)(m_descriptionWidth - instruction->numberOfOutputs() + 1u) / 2.0f * slotAndPadding.x + m_paddingSlots.x / 2.0f;
		for (unsigned int output = 0u; output < instruction->numberOfOutputs(); ++output) {
			m_spriteBatch->addTexture(m_mainSlotTex.get(), glm::vec2((float)output * slotAndPadding.x + offset, m_mainSlotDims.y - m_mainSlotTex->getPivot().y + slotAndPadding.y + fontHeight), 0);
		}
		m_spriteBatch->end();
		m_spriteBatch->draw(m_PTSBelow);

		//NUMBERS
		m_surfaceDescription.clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 1);
		m_spriteBatch->begin();
		//Inputs
		offset = (float)(m_descriptionWidth - instruction->numberOfInputs() + 1u) / 2.0f * slotAndPadding.x + m_paddingSlots.x / 2.0f;
		for (unsigned int input = 0u; input < instruction->numberOfInputs(); ++input) {
			if (instruction->input(input).amount > 1) {
				font->add(*m_spriteBatch, std::to_string(instruction->input(input).amount), glm::vec2((float)input * slotAndPadding.x + offset, m_mainSlotDims.y - m_mainSlotTex->getPivot().y - 40.0f + fontHeight), 0, m_amountColour, RE::HAlign::MIDDLE);
			}
		}
		//Outputs
		offset = (float)(m_descriptionWidth - instruction->numberOfOutputs() + 1u) / 2.0f * slotAndPadding.x + m_paddingSlots.x / 2.0f;
		for (unsigned int output = 0u; output < instruction->numberOfOutputs(); ++output) {
			if (instruction->output(output).amount > 1) {
				font->add(*m_spriteBatch, std::to_string(instruction->output(output).amount), glm::vec2((float)output * slotAndPadding.x + offset, m_mainSlotDims.y - m_mainSlotTex->getPivot().y + slotAndPadding.y - 40.0f + fontHeight), 0, m_amountColour, RE::HAlign::MIDDLE);
			}
		}
		m_spriteBatch->end();
		m_spriteBatch->draw(m_PTSAbove);

		m_surfaceDescription.resetTarget();

		//ITEM SPRITES
		m_inputDescrIS.resize(instruction->numberOfInputs());
		for (unsigned int input = 0u; input < instruction->numberOfInputs(); ++input) {
			m_inputDescrIS[input] = ItemSprite(instruction->input(input));
		}
		m_outputDescrIS.resize(instruction->numberOfOutputs());
		for (unsigned int output = 0u; output < instruction->numberOfOutputs(); ++output) {
			m_outputDescrIS[output] = ItemSprite(instruction->output(output));
		}
	}
}