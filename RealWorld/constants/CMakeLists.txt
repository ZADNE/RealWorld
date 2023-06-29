add_subdirectory(tile_properties)

target_sources_and_shaders(RealWorld PRIVATE
    biome.glsl                  
    body.hpp                    body.glsl
    generation.hpp              generation.glsl
    chunk.hpp                   
    light.hpp                   light.glsl
    tile.hpp                    tile.glsl
    tree.hpp                    tree.glsl
)
