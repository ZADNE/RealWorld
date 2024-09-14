real_target_sources(RealWorld
    PUBLIC
        BodiesSB.glsl               
        BranchAllocRegSB.glsl       
        BranchAllocReqUB.glsl       
        branchRasterizationPll.glsl 
        BranchSB.glsl               
        duplicateBranches.geom      
        normAngles.glsl             
        rasterizeBranches.vert      rasterizeBranches.frag
        simulateBodies.comp         
        tessellateBranches.tesc     tessellateBranches.tese
        unrasterizeBranches.vert    unrasterizeBranches.frag
        VegDynamicsPC.glsl          
        wind.glsl                   
)
