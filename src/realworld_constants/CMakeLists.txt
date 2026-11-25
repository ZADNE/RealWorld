add_subdirectory(tile_properties)

re_target_sources(RealWorld_constants
    PUBLIC
        Body.cppm                   Body.glsl
        Chunk.cppm                  Chunk.glsl
        Generation.cppm             Generation.glsl
        Item.cppm                   
                                    Light.glsl
                                    Sky.glsl
        Tile.cppm                   Tile.glsl
        Vegetation.cppm             Vegetation.glsl
        World.cppm                  World.glsl
)
