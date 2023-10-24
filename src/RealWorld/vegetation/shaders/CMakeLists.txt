target_shaders(RealWorld PRIVATE
    BodiesSB.glsl               
    Branch.glsl                 
    BranchRasterSB.glsl         
    BranchVectorSB.glsl         
    normAngles.glsl             
    simulateBodies.comp         
    rasterizeBranches.vert      rasterizeBranches.frag
    tessellateBranches.tesc     tessellateBranches.tese
    unrasterizeBranches.vert    unrasterizeBranches.frag
    VegDynamicsPC.glsl          
)
