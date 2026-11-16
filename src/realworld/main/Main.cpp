/**
 *  @author    Dubsky Tomas
 */
import realengine;
import :main.Arguments;
import :main.MainMenuRoom;
import :main.Setup.gen;
import :main.WorldRoom;
import :main.settings.GameSettings;

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

    re::MainProgram::initialize(
        re::MainProgramInitInfo{
            re::VulkanInitInfo{.deviceCreateInfoChain = &chain.get<>()},
            &re::setup::k_hotReloadInitInfo
        }
    );

    rw::GameSettings gameSettings{};
    rw::CLIArguments arguments = rw::parseArguments(argc, argv);

    auto* mainMenuRoom = re::MainProgram::addRoom<rw::MainMenuRoom>(gameSettings);
    re::MainProgram::addRoom<rw::WorldRoom>(gameSettings);

    return re::MainProgram::run(
        mainMenuRoom->name(), {rw::MainMenuRoom::TransitionArgs{arguments}}
    );
}
