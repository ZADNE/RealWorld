/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/InventoryUI.hpp>

#include <string>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/rendering/batches/SpriteBatch.hpp>

#include <RealWorld/shaders/common.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>


InventoryUI::InventoryUI(RE::SpriteBatch& spriteBatch, const glm::vec2& windowSize) :
    m_spriteBatch(spriteBatch) {
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
    m_slotsSurf.resize({ newWindowSize }, 1);
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

    //Redraw surface with slots
    m_slotsSurf.setTarget();
    m_slotsSurf.clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 0);

    glm::vec2 pos;

    m_spriteBatch.begin();

    forEachSlotByPosition(PRIMARY, [&](int x, int y) {
        pos = m_invBotLeftPx + slotPivot() + (slotDims() + SLOT_PADDING) * glm::vec2(x, y);
        m_spriteBatch.addSubimage(m_slotTex.get(), pos, 0, glm::vec2(0.0f, 0.0f));
    });

    m_spriteBatch.end(RE::GlyphSortType::TEXTURE);
    m_spriteBatch.draw();

    m_slotsSurf.resetTarget();
}

void InventoryUI::swapUnderCursor(const glm::vec2& cursorPx) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int x = slot->x; int y = slot->y;
        Item& item = (*m_inv[PRIMARY])[x][y];
        if (item.ID == m_heldItem.ID) {//Same items, dropping under corsor to slot
            item.merge(m_heldItem, 1.0f);
        }
        else {
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
        }
        else {//Picking up portion
            m_heldItem.insert((*m_inv[PRIMARY])(i), portion);
        }
        m_heldSprite = ItemSprite(m_heldItem);
        m_inv[PRIMARY]->wasChanged();
    }
}

void InventoryUI::selectSlot(SelectionManner selectionManner, int number) {
    if (m_open) { return; }

    switch (selectionManner) {
    case SelectionManner::ABS:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot = glm::clamp(number, 0, invSize(PRIMARY).x - 1);
        break;
    case SelectionManner::RIGHT:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot += number;
        while (m_chosenSlot > (invSize(PRIMARY).x - 1)) {
            m_chosenSlot -= invSize(PRIMARY).x;
        }
        break;
    case SelectionManner::LEFT:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot -= number;
        while (m_chosenSlot < 0) {
            m_chosenSlot += invSize(PRIMARY).x;
        }
        break;
    case SelectionManner::LAST_SLOT:
        m_chosenSlotPrev = m_chosenSlot;
        m_chosenSlot = invSize(PRIMARY).x - 1;
        break;
    case SelectionManner::PREV:
        std::swap(m_chosenSlot, m_chosenSlotPrev);
        break;
    }

    if (m_itemUser) m_itemUser->selectSlot(m_chosenSlot);
}

void InventoryUI::step() {
    //Increment subimage of all sprites
    forEachConnectedInventory([&](Connection con) {
        forEachSlotByIndex(con, [&](int i) {
            m_invItemSprites[con][i].step();
        });
    });
    m_heldSprite.step();
}

void InventoryUI::draw(const glm::vec2& cursorPx) {
    if (!m_inv[PRIMARY]) { return; }

    glm::vec2 pos;
    glm::vec2 slot0Px = m_invBotLeftPx + slotPivot();

    if (m_open) {//OPEN INVENTORY
        //Slots
        m_spriteBatch.addSurface(m_slotsSurf, glm::vec2(0.0f, 0.0f), 0, 0);
        //Item sprites
        int i = 0;
        forEachSlotByPosition(PRIMARY, [&](int x, int y) {
            Item& item = (*m_inv[PRIMARY])[x][y];
            if (!item.isEmpty()) {
                pos = slot0Px + (slotDims() + SLOT_PADDING) * glm::vec2(x, y);
                m_spriteBatch.addSprite(m_invItemSprites[PRIMARY][i], pos, 1);
            }
            i++;
            });
        if (!m_heldItem.isEmpty()) {
            //Item under cursor
            m_spriteBatch.addSprite(m_heldSprite, cursorPx, 10);
        }
    } else {//CLOSED INVENTORY
        for (int x = 0; x < invSize(PRIMARY).x; x++) {
            Item& item = (*m_inv[PRIMARY])[x][0];
            pos = slot0Px + (slotDims() + SLOT_PADDING) * glm::vec2(x, 0.0f);
            //Slot
            m_spriteBatch.addSubimage(m_slotTex.get(), pos, 0, glm::vec2(0.0f, 0.0f));
            if (!item.isEmpty()) {
                //Item sprite
                m_spriteBatch.addSprite(m_invItemSprites[PRIMARY][x], pos, 1);
            }
        };
        //The selected slot indicator
        m_spriteBatch.addSubimage(m_slotTex.get(), glm::vec2(slot0Px.x + (glm::ivec2(slotDims()).x + SLOT_PADDING.x) * (float)m_chosenSlot, slot0Px.y), 2, glm::vec2(1.0f, 0.0f));
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
