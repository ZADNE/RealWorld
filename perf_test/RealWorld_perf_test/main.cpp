/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <RealWorld/save/WorldSaveLoader.hpp>

#include <RealWorld_perf_test/TestRoom.hpp>

int main(int argc, char* argv[]) {
    vk::StructureChain chain{
        vk::PhysicalDeviceFeatures2{vk::PhysicalDeviceFeatures{}
                                        .setTessellationShader(true)
                                        .setVertexPipelineStoresAndAtomics(true)
                                        .setFragmentStoresAndAtomics(true)
                                        .setGeometryShader(true)
                                        .setMultiDrawIndirect(true)},
        vk::PhysicalDeviceVulkan12Features{}
            .setShaderInt8(true)
            .setUniformAndStorageBuffer8BitAccess(true)
            .setStorageBuffer8BitAccess(true)
            .setUniformBufferStandardLayout(true)
            .setShaderSampledImageArrayNonUniformIndexing(true)
            .setDescriptorBindingUpdateUnusedWhilePending(true)
            .setDescriptorBindingPartiallyBound(true)
            .setTimelineSemaphore(true),
        vk::PhysicalDeviceVulkan13Features{}.setSynchronization2(true)
    };

    re::MainProgram::initialize(re::VulkanInitInfo{
        .deviceCreateInfoChain = &chain.get<>()
    });

    rw::WorldSaveLoader::deleteWorld("test");
    rw::WorldSaveLoader::createWorld("test", 101);

    rw::GameSettings gameSettings{};

    auto* room = re::MainProgram::addRoom<rw::perf_test::TestRoom>(gameSettings);

    return re::MainProgram::run(room->name(), {std::string{"test"}});
}
