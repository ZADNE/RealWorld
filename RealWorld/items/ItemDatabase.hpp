/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>
#include <array>

enum class ItemId : uint16_t {
    Empty,
    //Fluids
    FWater,
    FLava,
    FSteam,
    FFire,
    FSmoke,
    FAcid,
    //Blocks
    BStone,
    BDirt,
    BGrass,
    BColdStone,
    BSand,
    BColdDirt,
    BColdGrass,
    BMud,
    BMudGrass,
    BDryGrass,
    BHallowStone,
    BHallowDirt,
    BHallowGrass,
    //Walls
    WStone,
    WDirt,
    WGrass,
    WColdStone,
    WSand,
    WColdDirt,
    WColdGrass,
    WMud,
    WMudGrass,
    WDryGrass,
    //Pickaxes
    CreativePickaxe,
    //Hammers
    CreativeHammer,

    Count
};

enum class ItemType : uint32_t {
    Empty,
    Material,
    Block,
    Wall,
    Pickaxe,//Mines blocks
    Hammer,//Mines walls
    CursorLight,
    OperatorLight
};

struct ItemMetadata {

    ItemMetadata(int maxStack, char textureAtlas, float spriteIndex, float drawScale, ItemType type, int typeIndex, const std::string& name) :
        name(name),
        maxStack(maxStack),
        textureAtlas(textureAtlas),
        spriteIndex(spriteIndex),
        drawScale(drawScale),
        type(type),
        typeIndex(typeIndex) {

    };

    std::string name;
    int maxStack;

    char textureAtlas;//Determines which texture atlas should be used to draw this item
    float spriteIndex;//Determines which sprite in the texture atlas should be used
    float drawScale;//Both X and Y scaling used when drawing the item inside slot

    ItemType type;
    int typeIndex;//ID of the block with block types
};

/**
 * @brief Is a readonly database of item metadata
*/
class ItemDatabase {
public:

    /**
     * @brief Fetches metadata of an item
    */
    const static ItemMetadata& md(ItemId id);

private:

    const static std::array<ItemMetadata, static_cast<size_t>(ItemId::Count)> m_itemMetadata;
};