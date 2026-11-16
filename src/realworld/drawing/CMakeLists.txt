add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        MinimapDrawer.cppm          MinimapDrawer.cpp
        MinimapLayout.cppm          
        ShadowDrawer.cppm           ShadowDrawer.cpp
        TileDrawer.cppm             TileDrawer.cpp
        WorldDrawer.cppm            WorldDrawer.cpp
)
