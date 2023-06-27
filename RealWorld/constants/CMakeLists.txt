add_subdirectory(tile_properties)

target_sources_and_shaders(RealWorld PRIVATE
    biomes.glsl                 
    bodies.hpp                  bodies.glsl
    generation.hpp              generation.glsl
    chunk.hpp                   
    light.hpp                   light.glsl
    tile.hpp                    tile.glsl
)
