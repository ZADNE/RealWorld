target_shaders(RealWorld PRIVATE
    addDynamicLights.comp       
    AnalysisPC.glsl             
    analyzeTiles.comp           
    calculateShadows.comp       
    drawDroppedTiles.vert       drawDroppedTiles.geom       drawDroppedTiles.frag
    drawFullscreen.vert         
    drawMinimap.vert            drawMinimap.frag
    drawShadows.frag            
    drawTiles.frag              
    DroppedTilesDrawingPC.glsl  
    DynamicLightsSB.glsl        
    WorldDrawingPC.glsl         
)
