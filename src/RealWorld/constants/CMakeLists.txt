add_subdirectory(tile_properties)

real_target_sources(RealWorld
    PUBLIC
        Body.hpp                    Body.glsl
        Chunk.hpp                   Chunk.glsl
        Generation.hpp              Generation.glsl
        Item.hpp                    
        Light.hpp                   Light.glsl
        ResourceIndex.hpp           
        Tile.hpp                    Tile.glsl
        Vegetation.hpp              Vegetation.glsl
        World.hpp                   World.glsl
)
