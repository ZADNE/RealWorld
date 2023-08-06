target_shaders(RealWorld PRIVATE
    ActiveChunksSB.glsl         
    analyzeContinuity.comp      
    BodiesSB.glsl               
    BranchesSB.glsl             
    LSystemSB.glsl             
    modifyTiles.comp            
    simulateBodies.comp         
    simulateFluids.comp         
    simulAndRasterizeTrees.vert simulAndRasterizeTrees.tesc
    simulAndRasterizeTrees.tese simulAndRasterizeTrees.frag
    simulationPL.comp           
    tileLoad.glsl               tileLoadStore.glsl
    TilePropertiesUB.glsl       
    transformTiles.comp         
    TreeDynamicsPC.glsl         
    WorldDynamicsPC.glsl        
)
