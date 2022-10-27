add_subdirectory(tile_properties)

target_shaders(RealWorld PRIVATE
    ActiveChunksSSIB.glsl       
    analyzeContinuity.comp      
    modifyTiles.comp            
    simulateFluids.comp         
    tileLoad.glsl               tileLoadStore.glsl
    transformTiles.comp         
    WorldDynamicsUIB.glsl       
)
