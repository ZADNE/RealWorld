target_shaders(RealWorld PRIVATE
    BodiesSB.glsl               
    Branch.glsl                 
    BranchSB.glsl               BranchSBWithAllocation.glsl
    duplicateBranches.geom      
    normAngles.glsl             
    rasterizeBranches.vert      rasterizeBranches.frag
    simulateBodies.comp         
    tessellateBranches.tesc     tessellateBranches.tese
    unrasterizeBranches.vert    unrasterizeBranches.frag
    VegDynamicsPC.glsl          
)
