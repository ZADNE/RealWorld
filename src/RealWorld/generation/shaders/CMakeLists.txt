add_subdirectory(tiles)
add_subdirectory(vegetation)

real_target_sources(RealWorld
    PUBLIC
        GenerationPC.glsl           
        generationPll.glsl          
)
