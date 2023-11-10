/*!
 *  @author    Dubsky Tomas
 */
#include <stack>

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/VegPreparationSB.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

namespace {

constexpr size_t k_vegTemplatesBranchCount = 115;

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

struct InterpretationParameters {
    glm::vec2 initSizeTi; // x = radius, y = length
    glm::vec2 sizeChange;
    float     density;
    float     stiffness;
    float     angleChange;
    Wall      wallType;
};

void interpret(
    std::vector<Branch>&            branches,
    std::string_view                sentence,
    const InterpretationParameters& params
) {
    const auto offset    = branches.size();
    auto       addBranch = [&](float lengthTi,
                         float radiusTi,
                         float density,
                         float stiffness,
                         float angleNorm,
                         int   myIndex,
                         int   parentIndex) {
        glm::vec2 absPosTi{0.0};
        float     absAngleNorm;
        float     relRestAngleNorm;
        if (myIndex != parentIndex) { // If it is child branch
            const Branch& parent = branches[parentIndex];
            absAngleNorm = glm::fract(parent.absAngleNorm + angleNorm);
            relRestAngleNorm = glm::fract(angleNorm);
            absPosTi = parent.absPosTi + toCartesian(lengthTi, absAngleNorm);
        } else { // If it is root branch
            absAngleNorm     = glm::fract(angleNorm);
            relRestAngleNorm = 0.0;
        }

        branches.emplace_back(
            absPosTi,
            parentIndex - myIndex,
            static_cast<glm::uint>(params.wallType),
            absAngleNorm,
            relRestAngleNorm,
            0.0f,
            radiusTi,
            lengthTi,
            density,
            stiffness,
            0u
        );
    };

    addBranch(0.0, 0.0, 0.0, 0.1, 0.25, 0, 0);
    struct TurtleState {
        glm::vec2 sizeTi;
        float     density;
        float     stiffness;
        float     angleChange;
        float     angleNorm;
        int       parentIndex;
    };
    std::stack<TurtleState> stack;
    stack.emplace(
        params.initSizeTi, params.density, params.stiffness, params.angleChange, 0.0f, 0
    );
    for (char c : sentence) {
        TurtleState& state = stack.top();
        switch (c) {
        case 'B':
        case 'S':
            addBranch(
                state.sizeTi.y,
                state.sizeTi.x,
                state.density,
                state.stiffness,
                state.angleNorm,
                static_cast<int>(branches.size() - offset),
                state.parentIndex
            );
            state.angleNorm   = 0.0;
            state.parentIndex = branches.size() - offset - 1;
            state.sizeTi *= params.sizeChange;
            state.sizeTi.x = glm::max(state.sizeTi.x, 0.5f);
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
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateVegPl);
    commandBuffer.dispatch(1u, 1u, 1u);

    { // Add barriers between preparation and vector generation
        vk::BufferMemoryBarrier2 preparationBarrier{
            S::eComputeShader,                               // Src stage mask
            A::eShaderStorageWrite,                          // Src access mask
            S::eDrawIndirect | S::eComputeShader,            // Dst stage mask
            A::eIndirectCommandRead | A::eShaderStorageRead, // Dst access mask
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored, // Ownership transition
            *m_vegPreparationBuf,
            0,
            offsetof(VegPreparationSB, b_branchInstances)};
        commandBuffer.pipelineBarrier2({{}, {}, preparationBarrier, {}});
    }

    // Dispatch branch vector generation
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateVectorVegPl);
    commandBuffer.dispatchIndirect(
        *m_vegPreparationBuf, offsetof(VegPreparationSB, b_vegetationDispatchSize)
    );

    { // Add barriers between vector generation and raster generation
        vk::BufferMemoryBarrier2 vectorBarrier{
            S::eComputeShader,      // Src stage mask
            A::eShaderStorageWrite, // Src access mask
            S::eComputeShader,      // Dst stage mask
            A::eShaderStorageRead,  // Dst access mask
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored, // No ownership transition
            *m_vegPreparationBuf,
            offsetof(VegPreparationSB, b_branchInstances),
            vk::WholeSize};
        commandBuffer.pipelineBarrier2({{}, {}, vectorBarrier, {}});
    }

    // Dispatch branch raster generation
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateRasterVegPl);
    commandBuffer.dispatchIndirect(
        *m_vegPreparationBuf, offsetof(VegPreparationSB, b_branchDispatchSize)
    );
}

re::Buffer ChunkGenerator::createVegTemplatesBuffer() {
    std::vector<Branch> branches;
    branches.reserve(k_vegTemplatesBranchCount);

    // Oak
    interpret(
        branches,
        derive("B", std::to_array<RewriteRule>({{'B', "SS-[-B+B+B]+[+B-B-B]"}}), 2),
        InterpretationParameters{
            .initSizeTi  = glm::vec2(2.0, 10.0),
            .sizeChange  = glm::vec2(0.875, 1.0),
            .density     = 4.0,
            .stiffness   = 1.0,
            .angleChange = 0.05,
            .wallType    = Wall::OakWood}
    );
    auto oakEnd = branches.size();

    // Acacia
    interpret(
        branches,
        derive("B", std::to_array<RewriteRule>({{'B', "S[+B][-B]"}}), 5),
        InterpretationParameters{
            .initSizeTi  = glm::vec2(1.75, 30.0),
            .sizeChange  = glm::vec2(0.75, 0.75),
            .density     = 4.0,
            .stiffness   = 0.5,
            .angleChange = 0.08,
            .wallType    = Wall::AcaciaWood}
    );
    auto acaciaSize = branches.size() - oakEnd;

    assert(branches.size() == k_vegTemplatesBranchCount);

    return re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Branch) * branches.size(),
        .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
        .initData    = std::as_bytes(std::span{branches})}};
}

} // namespace rw
