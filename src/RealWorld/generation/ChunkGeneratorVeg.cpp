/*!
 *  @author    Dubsky Tomas
 */
#include <stack>

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

namespace {

constexpr size_t k_vegTemplatesBranchCount = 51;

glm::vec2 toCartesian(float len, float angleNorm) {
    float angle = angleNorm * glm::two_pi<float>();
    return {len * glm::cos(angle), len * glm::sin(angle)};
}

using RewriteRule = std::pair<char, std::string_view>;

std::string derive(
    std::string_view axiom, std::span<const RewriteRule> rewriteRules, size_t nIters
) {
    std::string sentence{axiom};
    std::string next;

    // Do nIters iterations
    for (size_t i = 0; i < nIters; i++) {
        next.clear();
        next.reserve(sentence.size() * 4);
        // Try to rewrite each character
        for (char c : sentence) {
            // Find the rule that can be applied
            auto it = std::find_if(
                rewriteRules.begin(),
                rewriteRules.end(),
                [&](const RewriteRule& rule) { return c == rule.first; }
            );
            // Apply the rule (if found) or just copy the character
            if (it != rewriteRules.end()) {
                next.append(it->second);
            } else {
                next.push_back(c);
            }
        }
        sentence = next;
    }
    return sentence;
}

struct InterpretationInitState {
    float lengthTi;
    float radiusTi;
    float density;
    float stiffness;
    float angleChange;
};

void interpret(
    std::vector<Branch>&           branches,
    std::string_view               sentence,
    const InterpretationInitState& initState
) {
    auto addBranch = [&](float        lengthTi,
                         float        radiusTi,
                         float        density,
                         float        stiffness,
                         float        angleNorm,
                         unsigned int parentIndex) {
        glm::vec2 absPosTi{0.0};
        float     absAngleNorm;
        float     relRestAngleNorm;
        if (parentIndex != ~0u) { // If it is child branch
            const Branch& parent = branches[parentIndex];
            absAngleNorm         = glm::fract(parent.absAngleNorm + angleNorm);
            relRestAngleNorm     = glm::fract(angleNorm);
            absPosTi = parent.absPosTi + toCartesian(lengthTi, absAngleNorm);
        } else { // If it is root branch
            absAngleNorm     = glm::fract(angleNorm);
            relRestAngleNorm = 0.0;
        }

        branches.emplace_back(
            absPosTi,
            parentIndex != ~0 ? parentIndex : (unsigned)branches.size(),
            absAngleNorm,
            relRestAngleNorm,
            0.0f,
            radiusTi,
            lengthTi,
            density,
            stiffness,
            glm::vec2{}
        );
    };

    addBranch(0.0, 0.0, 0.0, 0.1, 0.25, ~0u);
    struct TurtleState {
        float        lengthTi;
        float        radiusTi;
        float        density;
        float        stiffness;
        float        angleChange;
        float        angleNorm;
        unsigned int parentIndex;
    };
    std::stack<TurtleState> stack;
    stack.emplace(
        initState.lengthTi,
        initState.radiusTi,
        initState.density,
        initState.stiffness,
        initState.angleChange,
        0.0f,
        0u
    );
    for (char c : sentence) {
        TurtleState& state = stack.top();
        switch (c) {
        case 'B':
        case 'S':
            addBranch(
                state.lengthTi,
                state.radiusTi,
                state.density,
                state.stiffness,
                state.angleNorm,
                state.parentIndex
            );
            state.angleNorm   = 0.0;
            state.parentIndex = branches.size() - 1;
            state.radiusTi *= 0.875f;
            state.stiffness *= 0.75;
            break;
        case '+': state.angleNorm += state.angleChange; break;
        case '-': state.angleNorm -= state.angleChange; break;
        case '[': stack.emplace(stack.top()); break;
        case ']': stack.pop(); break;
        default: break;
        }
    }
}

} // namespace

void ChunkGenerator::generateVegetation(const vk::CommandBuffer& commandBuffer) {
    // Dispatch preparation
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_prepareVegPl);
    commandBuffer.dispatch(1u, 1u, 1u);

    // Add barriers to generation
    std::array<vk::BufferMemoryBarrier2, 2> vegBarriers = {
        vk::BufferMemoryBarrier2{
            S::eComputeShader,                               // Src stage mask
            A::eShaderStorageWrite,                          // Src access mask
            S::eDrawIndirect | S::eComputeShader,            // Dst stage mask
            A::eIndirectCommandRead | A::eShaderStorageRead, // Dst access mask
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored, // Ownership transition
            *m_vegPreparationBuf,
            0,
            vk::WholeSize},
        vk::BufferMemoryBarrier2{
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
            S::eTransfer,                                   // Dst stage mask
            A::eTransferRead,                               // Dst access mask
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored, // Ownership transition
            **m_branchesBuf.write(),
            offsetof(BranchesSB, header),
            sizeof(BranchesSB::header)}};
    commandBuffer.pipelineBarrier2({{}, {}, vegBarriers, {}});

    // Dispatch generation
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateVegPl);
    commandBuffer.dispatchIndirect(*m_vegPreparationBuf, 0);

    // Copy branch header
    constexpr static vk::BufferCopy2 bufferCopy{
        offsetof(BranchesSB, header),
        offsetof(BranchesSB, header),
        sizeof(BranchesSB::header)};
    commandBuffer.copyBuffer2(vk::CopyBufferInfo2{
        **m_branchesBuf.write(), **m_branchesBuf.read(), bufferCopy});
}

re::Buffer ChunkGenerator::createVegTemplatesBuffer() {
    std::vector<Branch> branches;
    branches.reserve(k_vegTemplatesBranchCount);

    // Oak
    interpret(
        branches,
        derive("B", std::to_array<RewriteRule>({{'B', "SS-[-B+B+B]+[+B-B-B]"}}), 2),
        InterpretationInitState{
            .lengthTi    = 10.0,
            .radiusTi    = 2.0,
            .density     = 4.0,
            .stiffness   = 1.0,
            .angleChange = 0.05}
    );

    assert(branches.size() == k_vegTemplatesBranchCount);

    return re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Branch) * branches.size(),
        .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
        .initData    = std::as_bytes(std::span{branches})}};
}

} // namespace rw
