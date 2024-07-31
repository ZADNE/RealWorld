/*!
 *  @author    Dubsky Tomas
 */
#include <algorithm>
#include <string>

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>

#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/InventoryUI.hpp>
#include <RealWorld/items/ItemUser.hpp>

namespace rw {

InventoryUI::InventoryUI(glm::vec2 windowSize) {
    windowResized(windowSize);
}

InventoryUI::~InventoryUI() {
    // Disconnect all inventories
    forEachConnectedInventory([&](Connection con) {
        m_inv[con]->connectToUI(nullptr, con);
    });
}

void InventoryUI::windowResized(glm::vec2 newWindowSize) {
    m_windowSize = newWindowSize;
    reload();
}

void InventoryUI::connectToInventory(Inventory* inventory, Connection connection) {
    bool changed = false;
    auto& inv    = m_inv[(size_t)connection];
    if (inv) { // Disconnect from the current inventory
        inv->connectToUI(nullptr, connection);
        changed = true;
    }
    inv = inventory;
    if (inv) { // Connect to the new inventory
        inv->connectToUI(this, connection);
        changed = true;
    }
    if (changed) { // Reload the UI
        reload();
    }
}

void InventoryUI::switchOpenClose() {
    if (m_heldItem.isEmpty()) { // Not holding anything in hand
        m_open = !m_open;
    }
}

void InventoryUI::reload() {
    if (!m_inv[Primary])
        return;

    glm::vec2 invSizePx = glm::vec2(invSize(Primary)) * slotDims() +
                          (glm::vec2(invSize(Primary)) - 1.0f) * k_slotPadding;

    m_invBotLeftPx =
        glm::vec2((m_windowSize.x - invSizePx.x) * 0.5f, k_slotPadding.y);

    // Reload sprites
    m_invItemSprites[Primary].resize(invSlotCount(Primary));
    forEachSlotByIndex(Primary, [&](int i) {
        m_invItemSprites[Primary][i] = ItemSprite{(*m_inv[Primary])(i)};
    });
}

void InventoryUI::swapUnderCursor(glm::vec2 cursorPx) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int x            = slot->x;
        int y            = slot->y;
        Item& itemInSlot = (*m_inv[Primary])[x][y];
        if (itemInSlot.id == m_heldItem.id) {
            // Same items, drop item under corsor to slot
            itemInSlot.merge(m_heldItem, 1.0f);
        } else {
            std::swap(itemInSlot, m_heldItem);
            std::swap(
                m_invItemSprites[Primary][m_inv[Primary]->toIndex(x, y)], m_heldSprite
            );
        }
        m_inv[Primary]->wasChanged();
    }
}

void InventoryUI::movePortion(glm::vec2 cursorPx, float portion) {
    auto slot = cursorToSlot(cursorPx);
    if (slot) {
        int i = m_inv[Primary]->toIndex(slot->x, slot->y);
        if (!m_heldItem.isEmpty()) { // Dropping portion
            (*m_inv[Primary])(i).merge(m_heldItem, portion);
            (*m_inv[Primary])(i).insert(m_heldItem, portion);
        } else { // Picking up portion
            m_heldItem.insert((*m_inv[Primary])(i), portion);
        }
        m_heldSprite = ItemSprite(m_heldItem);
        m_inv[Primary]->wasChanged();
    }
}

void InventoryUI::selectSlot(SlotSelectionManner selectionManner, int number) {
    if (m_open) {
        return;
    }

    switch (selectionManner) {
    case SlotSelectionManner::AbsolutePos:
        m_selSlotPrev = m_selSlot;
        m_selSlot     = glm::clamp(number, 0, invSize(Primary).x - 1);
        break;
    case SlotSelectionManner::ScrollRight:
        m_selSlotPrev = m_selSlot;
        m_selSlot += number;
        while (m_selSlot > (invSize(Primary).x - 1)) {
            m_selSlot -= invSize(Primary).x;
        }
        break;
    case SlotSelectionManner::ScrollLeft:
        m_selSlotPrev = m_selSlot;
        m_selSlot -= number;
        while (m_selSlot < 0) { m_selSlot += invSize(Primary).x; }
        break;
    case SlotSelectionManner::ToLastSlot:
        m_selSlotPrev = m_selSlot;
        m_selSlot     = invSize(Primary).x - 1;
        break;
    case SlotSelectionManner::ToPrevious:
        std::swap(m_selSlot, m_selSlotPrev);
        break;
    }
}

void InventoryUI::step() {
    // Increment subimage of all sprites
    forEachConnectedInventory([&](Connection con) {
        forEachSlotByIndex(con, [&](int i) {
            m_invItemSprites[con][i].sprite().step();
        });
    });
    m_heldSprite.sprite().step();
}

void InventoryUI::draw(re::SpriteBatch& spriteBatch, glm::vec2 cursorPx) {
    if (!m_inv[Primary]) {
        return;
    }

    const glm::vec2 slotOffsetPx = slotDims() + k_slotPadding;
    glm::vec2 slot0Px            = m_invBotLeftPx + slotPivot();

    if (m_open) {                                          // OPEN INVENTORY
        int i = 0;
        forEachSlotByPosition(Primary, [&](int x, int y) { // All inv. slots
            glm::vec2 pos = slot0Px + slotOffsetPx * glm::vec2{x, y};
            drawSlot(spriteBatch, pos, i);
            i++;
        });
        if (!m_heldItem.isEmpty()) { // Item under cursor
            spriteBatch.addSprite(m_heldSprite.sprite(), cursorPx);
            drawItemCount(spriteBatch, cursorPx, m_heldItem.count);
        }
    } else {                                           // CLOSED INVENTORY
        for (int x = 0; x < invSize(Primary).x; x++) { // Hotbar
            glm::vec2 pos = slot0Px + slotOffsetPx * glm::vec2{x, 0.0f};
            drawSlot(spriteBatch, pos, x);
        };
        spriteBatch.addSubimage( // The selected slot indicator
            m_slotTex,
            glm::vec2(slot0Px.x + slotOffsetPx.x * m_selSlot, slot0Px.y),
            glm::vec2(1.0f, 0.0f)
        );
    }
}
glm::ivec2 InventoryUI::invSize(Connection con) const {
    return m_inv[con]->dims();
}

int InventoryUI::invSlotCount(Connection con) const {
    return m_inv[con]->slotCount();
}

std::optional<glm::ivec2> InventoryUI::cursorToSlot(glm::vec2 cursorPx) const {
    if (m_open && m_inv[Primary]) {
        glm::vec2 posWithinInv = cursorPx - m_invBotLeftPx;
        glm::vec2 slotPos      = posWithinInv / (slotDims() + k_slotPadding);
        glm::bvec2 validSlot =
            glm::greaterThanEqual(slotPos, glm::vec2(0.0f, 0.0f)) &&
            glm::lessThan(slotPos, glm::vec2(invSize(Primary)));

        glm::vec2 slotPart    = slotDims() / (slotDims() + k_slotPadding);
        glm::bvec2 notPadding = glm::lessThan(glm::fract(slotPos), slotPart);

        if (glm::all(validSlot && notPadding)) {
            return slotPos;
        }
    }
    return std::nullopt;
}

void InventoryUI::drawSlot(
    re::SpriteBatch& spriteBatch, glm::vec2 pivotPx, int slotIndex
) const {
    spriteBatch.addSubimage(m_slotTex, pivotPx, glm::vec2(0.0f, 0.0f));
    const Item& item = (*m_inv[Primary])(slotIndex);
    if (!item.isEmpty()) {
        spriteBatch.addSprite(m_invItemSprites[Primary][slotIndex].sprite(), pivotPx);
        drawItemCount(spriteBatch, pivotPx, item.count);
    }
}

void InventoryUI::drawItemCount(
    re::SpriteBatch& spriteBatch, glm::vec2 pivotPx, int itemCount
) const {
    if (itemCount > 1) {
        auto str = std::to_string(itemCount);
        std::u8string_view sv{
            reinterpret_cast<const char8_t*>(str.c_str()), str.size()
        };
        pivotPx += glm::vec2{slotPivot().x - 3.0f, -slotPivot().y + 6.0f};
        m_countFont.add(spriteBatch, sv, pivotPx, re::HorAlign::Right);
    }
}

} // namespace rw
