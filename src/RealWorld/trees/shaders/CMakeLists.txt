target_shaders(RealWorld PRIVATE
    BodiesSB.glsl               
    Branch.glsl                 
    BranchesSB.glsl             
    normAngles.glsl             
    simulateBodies.comp         
    rasterizeBranches.vert      rasterizeBranches.frag
    tessellateBranches.tesc     tessellateBranches.tese
    TreeDynamicsPC.glsl         
    unrasterizeBranches.vert    unrasterizeBranches.frag
)
