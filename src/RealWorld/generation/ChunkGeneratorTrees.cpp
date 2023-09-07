/*!
 *  @author    Dubsky Tomas
 */
#include <stack>

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <RealWorld/generation/ChunkGenerator.hpp>

namespace rw {

namespace {

glm::vec2 toCartesian(float len, float angleNorm) {
    float angle = angleNorm * glm::two_pi<float>();
    return {len * glm::cos(angle), len * glm::sin(angle)};
}

std::string derive(std::string_view axiom, std::string_view fRewrite, size_t nIters) {
    std::string sentence{axiom};
    std::string next;
    for (size_t i = 0; i < nIters; i++) {
        next.clear();
        next.reserve(sentence.size() * 4);
        for (char c : sentence) {
            if (c == 'F') {
                next.append(fRewrite);
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
};

std::vector<Branch> interpret(
    std::string_view sentence, const InterpretationInitState& initState
) {
    std::vector<Branch> branches;

    auto addBranch = [&](float        lengthTi,
                         float        radiusTi,
                         float        density,
                         float        stiffness,
                         float        angleNorm,
                         unsigned int parentIndex) {
        glm::vec2      absPosTi{0.0};
        Branch::Angles angles;
        if (parentIndex != ~0u) { // If it is child branch
            const Branch&  parent       = branches[parentIndex];
            Branch::Angles parentAngles = parent.angles.unpack();
            angles.absAngleNorm = glm::fract(parentAngles.absAngleNorm + angleNorm);
            angles.relRestAngleNorm = glm::fract(angleNorm);
            absPosTi = parent.absPosTi + toCartesian(lengthTi, angles.absAngleNorm);
        } else { // If it is root branch
            angles.absAngleNorm     = glm::fract(angleNorm);
            angles.relRestAngleNorm = 0.0;
        }

        branches.emplace_back(
            absPosTi,
            parentIndex != ~0 ? parentIndex : (unsigned)branches.size(),
            angles,
            radiusTi,
            lengthTi,
            density,
            stiffness
        );
    };

    addBranch(0.0, 0.0, 0.0, 0.1, 0.25, ~0u);
    struct TurtleState {
        float        lengthTi;
        float        radiusTi;
        float        density;
        float        stiffness;
        float        angleNorm;
        unsigned int parentIndex;
    };
    std::stack<TurtleState> stack;
    stack.emplace(
        initState.lengthTi,
        initState.radiusTi,
        initState.density,
        initState.stiffness,
        0.0f,
        0u
    );
    for (char c : sentence) {
        TurtleState& state = stack.top();
        switch (c) {
        case 'F':
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
            state.radiusTi *= 0.75f;
            state.stiffness *= 0.75;
            break;
        case '+': state.angleNorm += 0.05; break;
        case '-': state.angleNorm -= 0.05; break;
        case '[': stack.emplace(stack.top()); break;
        case ']': stack.pop(); break;
        default: break;
        }
    }

    return branches;
}

} // namespace

void ChunkGenerator::generateTrees(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateTreesPl);
    commandBuffer.dispatch(1u, 1u, 1u);
}

re::Buffer ChunkGenerator::createTreeTemplatesBuffer() {
    std::vector<Branch> oak = interpret(
        derive("F", "FF-[-F+F+F]+[+F-F-F]", 2),
        InterpretationInitState{
            .lengthTi = 15.0, .radiusTi = 2.0, .density = 1.0, .stiffness = 0.2}
    );

    return re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Branch) * oak.size(),
        .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
        .initData    = std::as_bytes(std::span{oak})}};
}

} // namespace rw
