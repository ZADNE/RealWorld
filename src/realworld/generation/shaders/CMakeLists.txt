add_subdirectory(tiles)
add_subdirectory(vegetation)

re_target_sources(RealWorld
    PUBLIC
        GenerationPC.glsl           
        generationPll.glsl          
)
