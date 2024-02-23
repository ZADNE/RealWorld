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

    vk::AttachmentDescription2 attachmentDescription{
        {},
        re::k_surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eDontCare,  // Color
        vk::AttachmentStoreOp::eStore,    // Color
        vk::AttachmentLoadOp::eDontCare,  // Stencil
        vk::AttachmentStoreOp::eDontCare, // Stencil
        vk::ImageLayout::eUndefined,      // Initial
        vk::ImageLayout::ePresentSrcKHR   // Final
    };
    vk::AttachmentReference2 attachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};
    vk::SubpassDescription2 subpassDescription{
        {},
        vk::PipelineBindPoint::eGraphics,
        0u,
        {},           // Input attachments
        attachmentRef // Color attachments
    };
    vk::SubpassDependency2 subpassDependency{
        vk::SubpassExternal,                                 // Src subpass
        0u,                                                  // Dst subpass
        {vk::PipelineStageFlagBits::eColorAttachmentOutput}, // Src stage mask
        {vk::PipelineStageFlagBits::eColorAttachmentOutput}, // Dst stage mask
        vk::AccessFlags{},                                   // Src access mask
        {vk::AccessFlagBits::eColorAttachmentWrite}          // Dst access mask
    };
    vk::RenderPassCreateInfo2 rpci{
        {}, attachmentDescription, subpassDescription, subpassDependency};

    re::MainProgram::initialize(re::VulkanInitInfo{
        .deviceCreateInfoChain = &chain.get<>(), .mainRenderPass = &rpci});

    rw::GameSettings gameSettings{};

    auto* mainMenuRoom = re::MainProgram::addRoom<rw::MainMenuRoom>(gameSettings);
    re::MainProgram::addRoom<rw::WorldRoom>(gameSettings);

    return re::MainProgram::run(mainMenuRoom->name(), {});
}