/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>
#include <RealWorld/main/settings/GameSettings.hpp>

int main(int argc, char* argv[]) {
    vk::StructureChain chain{
        vk::PhysicalDeviceFeatures2{
            vk::PhysicalDeviceFeatures{}
                .setTessellationShader(true)
                .setVertexPipelineStoresAndAtomics(true)
                .setFragmentStoresAndAtomics(true)
                .setGeometryShader(true)
                .setMultiDrawIndirect(true)},
        vk::PhysicalDeviceVulkan12Features{}
            .setStorageBuffer8BitAccess(true)
            .setUniformBufferStandardLayout(true)
            .setShaderSampledImageArrayNonUniformIndexing(true)
            .setDescriptorBindingUpdateUnusedWhilePending(true)
            .setDescriptorBindingPartiallyBound(true)
            .setTimelineSemaphore(true),
        vk::PhysicalDeviceVulkan13Features{}.setSynchronization2(true)};
    re::VulkanInitInfo initInfo{.deviceCreateInfoChain = &chain.get<>()};
    re::MainProgram::initialize(initInfo);

    rw::GameSettings gameSettings{};

    auto* mainMenuRoom = re::MainProgram::addRoom<rw::MainMenuRoom>(gameSettings);
    re::MainProgram::addRoom<rw::WorldRoom>(gameSettings);

    return re::MainProgram::run(mainMenuRoom->name(), {});
}