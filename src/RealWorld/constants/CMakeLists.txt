add_subdirectory(tile_properties)

real_target_sources(RealWorld
    PUBLIC
        body.hpp                    body.glsl
        chunk.hpp                   chunk.glsl
        generation.hpp              generation.glsl
        item.hpp                    
        light.hpp                   light.glsl
        tile.hpp                    tile.glsl
        vegetation.hpp              vegetation.glsl
        world.hpp                   world.glsl
)
