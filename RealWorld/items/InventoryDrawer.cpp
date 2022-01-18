#include <RealWorld/items/InventoryDrawer.hpp>

#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <RealEngine/ResourceManager.hpp>
#include <RealEngine/SpriteBatch.hpp>
#include <RealEngine/Font.hpp>
#include <RealEngine/utility.hpp> //rmath::clamp()

#include <RealWorld//metadata.hpp>
#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>



InventoryDrawer::InventoryDrawer() {

}

InventoryDrawer::~InventoryDrawer() {
	//Disconnecting all inventories
	for (int i = 0; i < Connection::NUMBER_OF_TYPES; ++i) {
		if (m_inv[i]) {//If an inventory is connected to this slot
			m_inv[i]->connectToDrawer(nullptr, (Connection)i);
		}
	}
}

void InventoryDrawer::init(RE::SpriteBatch* spriteBatch, const glm::vec2& windowSize, const RE::FontSeed& font) {
	m_spriteBatch = spriteBatch;
	m_font = font;
	resizeWindow(windowSize);
}

void InventoryDrawer::resizeWindow(const glm::vec2& newWindowSize) {
	m_windowSize = newWindowSize;
	m_cover.resize({newWindowSize}, 2);
	reloadEverything();
}

void InventoryDrawer::connectToInventory(Inventory* inventory, Connection connection) {
	if (connection == Connection::NUMBER_OF_TYPES) { return; }
	if (m_inv[(size_t)connection]) {//Disconnecting current inventory from this
		m_inv[(size_t)connection]->connectToDrawer(nullptr, connection);
	}
	m_inv[(size_t)connection] = inventory;
	if (m_inv[(size_t)connection]) {//Connecting current inventory to this
		m_invSize[(size_t)connection] = m_inv[(size_t)connection]->getSize();
		m_inv[(size_t)connection]->connectToDrawer(this, connection);
	}
	reloadEverything();
}

void InventoryDrawer::connectToItemUser(ItemUser* itemUser) {
	m_itemUser = itemUser;
}

void InventoryDrawer::switchState() {
	if (m_itemUnderCursor.isEmpty()) {//Not holding anything in hand
		if (!m_opened) {//If going to open
			if (m_itemUser) {
				//Ending all uses
				for (int i = 0; i < ItemUse::NUMBER_OF_USES; i++) {
					m_itemUser->endUse((ItemUse)i);
				}
			}
		}
		m_opened = !m_opened;
	}
}

bool InventoryDrawer::isOpen() {
	return m_opened;
}

void InventoryDrawer::reloadNumbers() {
	updateSurfaceNumbers();
}

void InventoryDrawer::reloadEverything() {
	if (m_inv[Connection::PRIMARY]) {
		m_invSize[Connection::PRIMARY] = m_inv[Connection::PRIMARY]->getSize();
		m_paddingWindow = glm::vec2(m_windowSize.x / 2.0f - m_mainSlotDims.x * (m_invSize[Connection::PRIMARY].x / 2.0f - 0.5f) - m_paddingSlots.x * (m_invSize[Connection::PRIMARY].x / 2.0f - 0.5f), m_mainSlotDims.y / 2.0f + 15.0f);
		//Reloading ItemSprites
		m_invItemSprites[Connection::PRIMARY].resize(m_invSize[Connection::PRIMARY].x);
		for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
			m_invItemSprites[Connection::PRIMARY][x].resize(m_invSize[Connection::PRIMARY].y);
			for (int y = 0; y < m_invSize[Connection::PRIMARY].y; ++y) {
				m_invItemSprites[Connection::PRIMARY][x][y] = ItemSprite((*m_inv)[Connection::PRIMARY][x][y]);
			}
		}
	}
	updateHitboxMesh();
	updateSurfaceSlots();
	updateSurfaceNumbers();
}

void InventoryDrawer::swapUnderCursor() {
	if (m_opened) {
		if (m_inv[Connection::PRIMARY]) {
			if (m_hitboxMainCover.overlaps(m_absCursorPos)) {
				//MAIN INVENTORY
				for (int y = 0; y < m_invSize[Connection::PRIMARY].y; y++) {
					for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
						if (m_hitboxes[x][y].overlaps(m_absCursorPos)) {//Collision found
							Item& item = (*m_inv[Connection::PRIMARY])[x][y];
							if (item.ID == m_itemUnderCursor.ID) {//Same items, dropping under corsor to slot
								item.merge(m_itemUnderCursor, 1.0f);
							} else {
								item.swap(m_itemUnderCursor);
								std::swap(m_invItemSprites[Connection::PRIMARY][x][y], m_underCursorItemSprite);
							}
							m_inv[Connection::PRIMARY]->wasChanged();
							return;
						}
					}
				}
			}
		}
	}
}

void InventoryDrawer::movePortion(float portion) {
	if (m_opened) {
		if (m_inv[Connection::PRIMARY]) {
			//MAIN INVENTORY
			for (int y = 0; y < m_invSize[Connection::PRIMARY].y; y++) {
				for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
					if (m_hitboxes[x][y].overlaps(m_absCursorPos)) {//Collision found
						if (!m_itemUnderCursor.isEmpty()) {//Dropping portion
							(*m_inv[Connection::PRIMARY])[x][y].merge(m_itemUnderCursor, portion);
							(*m_inv[Connection::PRIMARY])[x][y].insert(m_itemUnderCursor, portion);
						} else {//Picking up portion
							m_itemUnderCursor.insert((*m_inv[Connection::PRIMARY])[x][y], portion);
						}
						m_underCursorItemSprite = ItemSprite(m_itemUnderCursor);
						m_inv[Connection::PRIMARY]->wasChanged();
						return;
					}
				}
			}
		}
	}
}

void InventoryDrawer::chooseSlot(Choose choose, int number) {
	if (m_opened) { return; }
	int temp;

	switch (choose) {
	case Choose::ABS:
		m_chosenSlotPrev = m_chosenSlot;
		m_chosenSlot = rmath::clamp(number, 0, m_invSize[Connection::PRIMARY].x - 1);
		break;
	case Choose::RIGHT:
		m_chosenSlotPrev = m_chosenSlot;
		m_chosenSlot += number;
		while (m_chosenSlot > (m_invSize[Connection::PRIMARY].x - 1)) {
			m_chosenSlot -= m_invSize[Connection::PRIMARY].x;
		}
		break;
	case Choose::LEFT:
		m_chosenSlotPrev = m_chosenSlot;
		m_chosenSlot -= number;
		while (m_chosenSlot < 0) {
			m_chosenSlot += m_invSize[Connection::PRIMARY].x;
		}
		break;
	case Choose::LAST_SLOT:
		m_chosenSlotPrev = m_chosenSlot;
		m_chosenSlot = m_invSize[Connection::PRIMARY].x - 1;
		break;
	case Choose::PREV:
		temp = m_chosenSlot;
		m_chosenSlot = m_chosenSlotPrev;
		m_chosenSlotPrev = temp;
		break;
	}

	if (m_itemUser) {
		m_itemUser->chooseSlot(m_chosenSlot);
	}
}

void InventoryDrawer::step(const glm::ivec2& absCursorPos) {
	m_absCursorPos = absCursorPos;
	//ItemSprite step for all items
	for (size_t i = Connection::PRIMARY; i != Connection::NUMBER_OF_TYPES; ++i) {
		if (m_inv[i]) {//If connected to this inventory slot
			m_invSize[Connection::PRIMARY] = m_inv[Connection::PRIMARY]->getSize();
			for (int x = 0; x < m_invSize[Connection::PRIMARY].x; ++x) {
				for (int y = 0; y < m_invSize[Connection::PRIMARY].y; ++y) {
					m_invItemSprites[i][x][y].step();
				}
			}
		}
	}
	//Item under cursor
	m_underCursorItemSprite.step();
}

void InventoryDrawer::draw() {
	if (!m_shouldDraw) { return; }

	char amount[10];
	glm::vec2 pos;

	if (m_opened) {//OPEN INVENTORY
		//MAIN INVENTORY
		if (m_inv[Connection::PRIMARY]) {
			//Slots
			m_spriteBatch->addSurface(m_cover, glm::vec2(0.0f, 0.0f), 0, 0);
			//Item sprites
			for (int y = 0; y < m_invSize[Connection::PRIMARY].y; y++) {
				for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
					Item& item = m_inv[Connection::PRIMARY]->m_data.items[x][y];
					if (item.isEmpty()) {
						continue;//Not drawing empty item
					}
					pos = m_paddingWindow + (m_mainSlotDims + m_paddingSlots) * glm::vec2(x, y);
					m_spriteBatch->addSprite(m_invItemSprites[Connection::PRIMARY][x][y], pos, 1);
				}
			}
			if (!m_itemUnderCursor.isEmpty()) {
				//Item under cursor
				m_spriteBatch->addSprite(m_underCursorItemSprite, m_absCursorPos, 10);
				snprintf(amount, 10, "%i", m_itemUnderCursor.amount);
				if (m_itemUnderCursor.amount > 1) {
					RE::RM::getFont(m_font)->add(*m_spriteBatch, amount, glm::vec2(m_absCursorPos.x, m_absCursorPos.y - 40.0f), glm::vec2(1.0f, 1.0f), 11, m_amountColour, RE::HAlign::MIDDLE);
				}
			}
			//Amounts
			m_spriteBatch->addSurface(m_cover, glm::vec2(0.0f, 0.0f), 2, 1);
		}
	} else {//CLOSED INVENTORY
	   //MAIN INVENTORY
		if (m_inv[Connection::PRIMARY]) {
			for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
				Item& item = m_inv[Connection::PRIMARY]->m_data.items[x][0];
				pos = m_paddingWindow + (m_mainSlotDims + m_paddingSlots) * glm::vec2(x, 0);
				//Slot
				m_spriteBatch->addTexture(m_mainSlotTex.get(), pos, 0);
				if (item.isEmpty()) {
					continue;//Not drawing empty item
				}
				//Item sprite
				m_spriteBatch->addSprite(m_invItemSprites[Connection::PRIMARY][x][0], pos, 1);
				//Amount
				if (item.amount > 1) {
					snprintf(amount, 10, "%i", item.amount);
					RE::RM::getFont(m_font)->add(*m_spriteBatch, amount, glm::vec2(pos.x, pos.y - 40.0f), glm::vec2(1.0f, 1.0f), 3, m_amountColour, RE::HAlign::MIDDLE);
				}
			}
			//Chosen slot indicator
			m_spriteBatch->addTexture(m_slotIndicatorTex.get(), glm::vec2(m_paddingWindow.x + (m_mainSlotDims.x + m_paddingSlots.x) * (float)m_chosenSlot, m_paddingWindow.y), 2);
		}
	}
}

void InventoryDrawer::updateHitboxMesh() {
	m_hitboxes.clear();//Clearing previous mesh
	if (m_inv[Connection::PRIMARY]) {//Creating new mesh (if connected)
		m_hitboxMainCover.setDims(glm::ivec2(m_mainSlotDims + m_paddingSlots) * m_invSize[Connection::PRIMARY]);
		m_hitboxMainCover.setPosition(m_paddingWindow);
		std::vector<Hitbox> row;
		for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
			row.clear();
			for (int y = 0; y < m_invSize[Connection::PRIMARY].y; y++) {//Creating row
				row.emplace_back(m_paddingWindow - m_mainSlotDims / 2.0f + (m_mainSlotDims + m_paddingSlots) * glm::vec2(x, y), m_mainSlotDims, m_mainSlotDims / 2.0f);
			}
			m_hitboxes.emplace_back(row);//Inserting row
		}
	}
}

void InventoryDrawer::updateSurfaceSlots() {
	m_cover.setTarget();
	m_cover.clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 0);

	//MAIN INVENTORY
	if (m_inv[Connection::PRIMARY]) {
		glm::vec2 pos;

		m_spriteBatch->begin();

		for (int y = 0; y < m_invSize[Connection::PRIMARY].y; y++) {
			for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
				pos = m_paddingWindow + (m_mainSlotDims + m_paddingSlots) * glm::vec2(x, y);
				m_spriteBatch->addTexture(m_mainSlotTex.get(), pos, 0);
			}
		}

		m_spriteBatch->end();
		m_spriteBatch->draw();
	}

	m_cover.resetTarget();
}

void InventoryDrawer::updateSurfaceNumbers() {
	m_cover.setTarget();
	m_cover.clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 1);

	//MAIN INVENTORY
	if (m_inv[Connection::PRIMARY]) {
		glm::vec2 pos;
		char amount[10];

		m_spriteBatch->begin();

		for (int y = 0; y < m_invSize[Connection::PRIMARY].y; y++) {
			for (int x = 0; x < m_invSize[Connection::PRIMARY].x; x++) {
				pos = m_paddingWindow + (m_mainSlotDims + m_paddingSlots) * glm::vec2(x, y);
				if (m_inv[Connection::PRIMARY]->m_data.items[x][y].amount > 1) {
					snprintf(amount, 10, "%i", m_inv[Connection::PRIMARY]->m_data.items[x][y].amount);
					RE::RM::getFont(m_font)->add(*m_spriteBatch, amount, glm::vec2(pos.x, pos.y - 40.0f), glm::vec2(1.0f, 1.0f), 2, m_amountColour, RE::HAlign::MIDDLE);
				}
			}
		}
		m_spriteBatch->end();
		m_spriteBatch->draw(m_PTSAbove);
	}

	m_cover.resetTarget();
}