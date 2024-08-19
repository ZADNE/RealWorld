add_subdirectory(tile_properties)

target_sources_and_shaders(RealWorld PRIVATE
    body.hpp                    body.glsl
    chunk.hpp                   chunk.glsl
    generation.hpp              generation.glsl
    item.hpp                    
    light.hpp                   light.glsl
    tile.hpp                    tile.glsl
    vegetation.hpp              vegetation.glsl
    world.hpp                   world.glsl
)
