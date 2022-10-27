add_subdirectory(shaders)

target_sources(RealWorld PRIVATE
    ExternalLight.hpp           
    MinimapDrawer.hpp           MinimapDrawer.cpp
    ShadowDrawer.hpp            ShadowDrawer.cpp
    TileDrawer.hpp              TileDrawer.cpp
    WorldDrawer.hpp             WorldDrawer.cpp
)
