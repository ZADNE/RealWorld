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


InventoryUI::InventoryUI(const glm::vec2& windowSize) {
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
    bool changed = false;
    auto& inv = m_inv[(size_t)connection];
    if (inv) {//Disconnect from the current inventory
        inv->connectToDrawer(nullptr, connection);
        changed = true;
    }
    inv = inventory;
    if (inv) {//Connect to the new inventory
        inv->connectToDrawer(this, connection);
        changed = true;
    }
    if (changed) {//Reload the UI
        reload();
    }
}

void InventoryUI::openOrClose() {
    if (m_heldItem.isEmpty()) {//Not holding anything in hand
        m_open = !m_open;
    }
}

void InventoryUI::reload() {
    if (!m_inv[Primary]) return;

    glm::vec2 invSizePx = glm::vec2(invSize(Primary)) * slotDims() + (glm::vec2(invSize(Primary)) - 1.0f) * k_slotPadding;

    m_invBotLeftPx = glm::vec2((m_windowSize.x - invSizePx.x) * 0.5f, k_slotPadding.y);

    //Reload sprites
    m_invItemSprites[Primary].resize(invSlotCount(Primary));
    forEachSlotByIndex(Primary, [&](int i) {
        m_invItemSprites[Primary][i] = ItemSprite{(*m_inv[Primary])(i)};
    });
}

void InventoryUI::swapUnderCursor(const glm::vec2& cursorPx) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int x = slot->x; int y = slot->y;
        Item& item = (*m_inv[Primary])[x][y];
        if (item.id == m_heldItem.id) {//Same items, dropping under corsor to slot
            item.merge(m_heldItem, 1.0f);
        } else {
            item.swap(m_heldItem);
            std::swap(m_invItemSprites[Primary][m_inv[Primary]->toIndex(x, y)], m_heldSprite);
        }
        m_inv[Primary]->wasChanged();
    }
}

void InventoryUI::movePortion(const glm::vec2& cursorPx, float portion) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int i = m_inv[Primary]->toIndex(slot->x, slot->y);
        if (!m_heldItem.isEmpty()) {//Dropping portion
            (*m_inv[Primary])(i).merge(m_heldItem, portion);
            (*m_inv[Primary])(i).insert(m_heldItem, portion);
        } else {//Picking up portion
            m_heldItem.insert((*m_inv[Primary])(i), portion);
        }
        m_heldSprite = ItemSprite(m_heldItem);
        m_inv[Primary]->wasChanged();
    }
}

void InventoryUI::selectSlot(SlotSelectionManner selectionManner, int number) {
    if (m_open) { return; }

    switch (selectionManner) {
    case SlotSelectionManner::AbsolutePos:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot = glm::clamp(number, 0, invSize(Primary).x - 1);
        break;
    case SlotSelectionManner::ScrollRight:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot += number;
        while (m_chosenSlot > (invSize(Primary).x - 1)) {
            m_chosenSlot -= invSize(Primary).x;
        }
        break;
    case SlotSelectionManner::ScrollLeft:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot -= number;
        while (m_chosenSlot < 0) {
            m_chosenSlot += invSize(Primary).x;
        }
        break;
    case SlotSelectionManner::ToLastSlot:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot = invSize(Primary).x - 1;
        break;
    case SlotSelectionManner::ToPrevious:
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

void InventoryUI::draw(RE::SpriteBatch& spriteBatch, const glm::vec2& cursorPx) {
    if (!m_inv[Primary]) { return; }

    glm::vec2 pos;
    glm::vec2 slot0Px = m_invBotLeftPx + slotPivot();

    if (m_open) {//OPEN INVENTORY
        //Slots & item sprites
        int i = 0;
        forEachSlotByPosition(Primary, [&](int x, int y) {
            pos = slot0Px + (slotDims() + k_slotPadding) * glm::vec2(x, y);
        Item& item = (*m_inv[Primary])[x][y];
        spriteBatch.addSubimage(m_slotTex, pos, glm::vec2(0.0f, 0.0f));
        if (!item.isEmpty()) {
            spriteBatch.addSprite(m_invItemSprites[Primary][i].sprite(), pos);
        }
        i++;
        });
        if (!m_heldItem.isEmpty()) {
            //Item under cursor
            spriteBatch.addSprite(m_heldSprite.sprite(), cursorPx);
        }
    } else {//CLOSED INVENTORY
        for (int x = 0; x < invSize(Primary).x; x++) {
            Item& item = (*m_inv[Primary])[x][0];
            pos = slot0Px + (slotDims() + k_slotPadding) * glm::vec2(x, 0.0f);
            //Slot
            spriteBatch.addSubimage(m_slotTex, pos, glm::vec2(0.0f, 0.0f));
            if (!item.isEmpty()) {
                //Item sprite
                spriteBatch.addSprite(m_invItemSprites[Primary][x].sprite(), pos);
            }
        };
        //The selected slot indicator
        spriteBatch.addSubimage(m_slotTex, glm::vec2(slot0Px.x + (glm::ivec2(slotDims()).x + k_slotPadding.x) * (float)m_chosenSlot, slot0Px.y), glm::vec2(1.0f, 0.0f));
    }
}

inline glm::ivec2 InventoryUI::invSize(Connection con) const {
    return m_inv[con]->getSize();
}

inline int InventoryUI::invSlotCount(Connection con) const {
    return m_inv[con]->slotCount();
}

std::optional<glm::ivec2> InventoryUI::cursorToSlot(const glm::vec2& cursorPx) const {
    if (m_open && m_inv[Primary]) {
        glm::vec2 posWithinInv = cursorPx - m_invBotLeftPx;
        glm::vec2 slotPos = posWithinInv / (slotDims() + k_slotPadding);
        glm::bvec2 validSlot = glm::greaterThanEqual(slotPos, glm::vec2(0.0f, 0.0f)) && glm::lessThan(slotPos, glm::vec2(invSize(Primary)));

        glm::vec2 slotPart = slotDims() / (slotDims() + k_slotPadding);
        glm::bvec2 notPadding = glm::lessThan(glm::fract(slotPos), slotPart);

        if (glm::all(validSlot && notPadding)) {
            return slotPos;
        }
    }
    return std::nullopt;
}

