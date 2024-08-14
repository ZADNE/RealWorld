add_subdirectory(shaders)

target_sources(RealWorld PRIVATE
    DroppedTilesDrawer.hpp      DroppedTilesDrawer.cpp
    ExternalLight.hpp           
    MinimapDrawer.hpp           MinimapDrawer.cpp
    MinimapLayout.hpp           
    ShadowDrawer.hpp            ShadowDrawer.cpp
    TileDrawer.hpp              TileDrawer.cpp
    WorldDrawer.hpp             WorldDrawer.cpp
    WorldDrawingPC.hpp          
)
