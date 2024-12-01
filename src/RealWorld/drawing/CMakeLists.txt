add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        MinimapDrawer.hpp           MinimapDrawer.cpp
        MinimapLayout.hpp           
        ShadowDrawer.hpp            ShadowDrawer.cpp
        TileDrawer.hpp              TileDrawer.cpp
        WorldDrawer.hpp             WorldDrawer.cpp
)
