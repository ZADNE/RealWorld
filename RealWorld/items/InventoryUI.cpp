/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/InventoryUI.hpp>

#include <string>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>

#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>


InventoryUI::InventoryUI(RE::SpriteBatch& sb, const glm::vec2& windowSize) :
    m_sb(sb) {
    windowResized(windowSize);
}

InventoryUI::~InventoryUI() {
    //Disconnecting all inventories
    forEachConnectedInventory([&](Connection con) {
        m_inv[con]->connectToDrawer(nullptr, con);
    });
}

void InventoryUI::windowResized(const glm::vec2& newWindowSize) {
    m_windowSize = newWindowSize;
    reload();
}

void InventoryUI::connectToInventory(Inventory* inventory, Connection connection) {
    if (m_inv[(size_t)connection]) {//Disconnect the current inventory
        m_inv[(size_t)connection]->connectToDrawer(nullptr, connection);
    }
    m_inv[(size_t)connection] = inventory;
    if (m_inv[(size_t)connection]) {//Connecti the new inventory
        m_inv[(size_t)connection]->connectToDrawer(this, connection);
    }
    reload();
}

void InventoryUI::openOrClose() {
    if (m_heldItem.isEmpty()) {//Not holding anything in hand
        m_open = !m_open;
    }
}

void InventoryUI::reload() {
    if (!m_inv[PRIMARY]) return;

    glm::vec2 invSizePx = glm::vec2(invSize(PRIMARY)) * slotDims() + (glm::vec2(invSize(PRIMARY)) - 1.0f) * SLOT_PADDING;

    m_invBotLeftPx = glm::vec2((m_windowSize.x - invSizePx.x) * 0.5f, SLOT_PADDING.y);

    //Reload sprites
    m_invItemSprites[PRIMARY].clear();
    m_invItemSprites[PRIMARY].reserve(invSlotCount(PRIMARY));
    forEachSlotByIndex(PRIMARY, [&](int i) {
        m_invItemSprites[PRIMARY].emplace_back((*m_inv[PRIMARY])(i));
    });
}

void InventoryUI::swapUnderCursor(const glm::vec2& cursorPx) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int x = slot->x; int y = slot->y;
        Item& item = (*m_inv[PRIMARY])[x][y];
        if (item.ID == m_heldItem.ID) {//Same items, dropping under corsor to slot
            item.merge(m_heldItem, 1.0f);
        } else {
            item.swap(m_heldItem);
            std::swap(m_invItemSprites[PRIMARY][m_inv[PRIMARY]->toIndex(x, y)], m_heldSprite);
        }
        m_inv[PRIMARY]->wasChanged();
    }
}

void InventoryUI::movePortion(const glm::vec2& cursorPx, float portion) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int i = m_inv[PRIMARY]->toIndex(slot->x, slot->y);
        if (!m_heldItem.isEmpty()) {//Dropping portion
            (*m_inv[PRIMARY])(i).merge(m_heldItem, portion);
            (*m_inv[PRIMARY])(i).insert(m_heldItem, portion);
        } else {//Picking up portion
            m_heldItem.insert((*m_inv[PRIMARY])(i), portion);
        }
        m_heldSprite = ItemSprite(m_heldItem);
        m_inv[PRIMARY]->wasChanged();
    }
}

void InventoryUI::selectSlot(SlotSelectionManner selectionManner, int number) {
    if (m_open) { return; }

    switch (selectionManner) {
    case SlotSelectionManner::ABS:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot = glm::clamp(number, 0, invSize(PRIMARY).x - 1);
        break;
    case SlotSelectionManner::RIGHT:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot += number;
        while (m_chosenSlot > (invSize(PRIMARY).x - 1)) {
            m_chosenSlot -= invSize(PRIMARY).x;
        }
        break;
    case SlotSelectionManner::LEFT:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot -= number;
        while (m_chosenSlot < 0) {
            m_chosenSlot += invSize(PRIMARY).x;
        }
        break;
    case SlotSelectionManner::LAST_SLOT:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot = invSize(PRIMARY).x - 1;
        break;
    case SlotSelectionManner::PREV:
        std::swap(m_chosenSlot, m_chosenSlotPrev);
        break;
    }

    if (m_itemUser) m_itemUser->selectSlot(m_chosenSlot);
}

void InventoryUI::step() {
    //Increment subimage of all sprites
    forEachConnectedInventory([&](Connection con) {
        forEachSlotByIndex(con, [&](int i) {
            m_invItemSprites[con][i].sprite().step();
            });
    });
    m_heldSprite.sprite().step();
}

void InventoryUI::draw(const glm::vec2& cursorPx) {
    if (!m_inv[PRIMARY]) { return; }

    glm::vec2 pos;
    glm::vec2 slot0Px = m_invBotLeftPx + slotPivot();

    if (m_open) {//OPEN INVENTORY
        //Slots & item sprites
        int i = 0;
        forEachSlotByPosition(PRIMARY, [&](int x, int y) {
            pos = slot0Px + (slotDims() + SLOT_PADDING) * glm::vec2(x, y);
        Item& item = (*m_inv[PRIMARY])[x][y];
        if (!item.isEmpty()) {
            m_sb.addSprite(m_invItemSprites[PRIMARY][i].sprite(), pos, 1);
        }
        m_sb.addSubimage(m_slotTex, pos, 0, glm::vec2(0.0f, 0.0f));
        i++;
            });
        if (!m_heldItem.isEmpty()) {
            //Item under cursor
            m_sb.addSprite(m_heldSprite.sprite(), cursorPx, 10);
        }
    } else {//CLOSED INVENTORY
        for (int x = 0; x < invSize(PRIMARY).x; x++) {
            Item& item = (*m_inv[PRIMARY])[x][0];
            pos = slot0Px + (slotDims() + SLOT_PADDING) * glm::vec2(x, 0.0f);
            //Slot
            m_sb.addSubimage(m_slotTex, pos, 0, glm::vec2(0.0f, 0.0f));
            if (!item.isEmpty()) {
                //Item sprite
                m_sb.addSprite(m_invItemSprites[PRIMARY][x].sprite(), pos, 1);
            }
        };
        //The selected slot indicator
        m_sb.addSubimage(m_slotTex, glm::vec2(slot0Px.x + (glm::ivec2(slotDims()).x + SLOT_PADDING.x) * (float)m_chosenSlot, slot0Px.y), 2, glm::vec2(1.0f, 0.0f));
    }
}

inline glm::ivec2 InventoryUI::invSize(Connection con) const {
    return m_inv[con]->getSize();
}

inline int InventoryUI::invSlotCount(Connection con) const {
    return m_inv[con]->slotCount();
}

std::optional<glm::ivec2> InventoryUI::cursorToSlot(const glm::vec2& cursorPx) const {
    if (m_open && m_inv[PRIMARY]) {
        glm::vec2 posWithinInv = cursorPx - m_invBotLeftPx;
        glm::vec2 slotPos = posWithinInv / (slotDims() + SLOT_PADDING);
        glm::bvec2 validSlot = glm::greaterThanEqual(slotPos, glm::vec2(0.0f, 0.0f)) && glm::lessThan(slotPos, glm::vec2(invSize(PRIMARY)));

        glm::vec2 slotPart = slotDims() / (slotDims() + SLOT_PADDING);
        glm::bvec2 notPadding = glm::lessThan(glm::fract(slotPos), slotPart);

        if (glm::all(validSlot && notPadding)) {
            return slotPos;
        }
    }
    return std::nullopt;
}

