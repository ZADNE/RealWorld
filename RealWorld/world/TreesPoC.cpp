/*!
 *  @author    Dubsky Tomas
 */

#include <algorithm>
#include <execution>
#include <stack>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/trigonometric.hpp>

#include <RealWorld/world/TreesPoC.hpp>

namespace rw {

constexpr re::Color k_red{255, 0, 0, 255};

glm::vec2 toCartesian(float len, float angleNorm) {
    float angle = angleNorm * glm::two_pi<float>();
    return {len * glm::cos(angle), len * glm::sin(angle)};
}

float angularDifference(float target, float current) {
    float diff = target - current;
    assert(glm::abs(diff) <= 1.0f);
    diff += (diff > 0.5f) ? -1.0f : (diff < -0.5f) ? 1.0f : 0.0f;
    return diff;
}

TreesPoC::TreesPoC() {
    // Derivation
    std::string sentence = "F";
    for (size_t i = 0; i < 2; i++) {
        std::string next;
        next.reserve(sentence.size() * 4);
        for (char c : sentence) {
            if (c == 'F') {
                next.append("FF-[-F+F+F]+[+F-F-F]");
            } else {
                next.push_back(c);
            }
        }
        sentence = next;
    }

    // Interpretation
    addBranch(0.0, 0.0, 0.0, 0.1, 0.25, ~0u, {960.0, 100.0});
    struct TurtleState {
        float        lengthPx;
        float        radiusPx;
        float        density;
        float        stiffness;
        float        angleNorm;
        unsigned int parentIndex;
    };
    std::stack<TurtleState> stack;
    stack.emplace(60.0f, 10.0f, 1.0f, 0.2f, 0.0f, 0u);
    for (char c : sentence) {
        TurtleState& state = stack.top();
        switch (c) {
        case 'F':
            addBranch(
                state.lengthPx,
                state.radiusPx,
                state.density,
                state.stiffness,
                state.angleNorm,
                state.parentIndex
            );
            state.angleNorm   = 0.0;
            state.parentIndex = m_branches[0].size() - 1;
            state.radiusPx *= 0.75f;
            state.stiffness *= 0.75;
            break;
        case '+': state.angleNorm += 0.05; break;
        case '-': state.angleNorm -= 0.05; break;
        case '[': stack.emplace(stack.top()); break;
        case ']': stack.pop(); break;
        default: break;
        }
    }
}

void TreesPoC::step() {
    /*static int dada = 0;
    dada++;
    if (dada % 4 != 0)
        return;*/
    // Swap read index
    m_readIndex = 1 - m_readIndex;
    m_time += 0.0005;

    for (size_t i = 0; i < readBranches().size(); ++i) {
        Branch        b      = readBranches()[i];
        const Branch& parent = readBranches()[b.parentIndex];
        float         wind   = glm::perlin(glm::vec2{m_time, 0.0});
        wind += 0.3 * glm::perlin(glm::vec2{m_time * 10.0, 0.0f});

        float volume = glm::pi<float>() * b.radiusPx * b.radiusPx * b.lengthPx;
        float weight = volume * b.density;

        glm::vec2 force = glm::vec2{wind * b.radiusPx * b.lengthPx, weight * -0.01f};

        float forceAngle = std::atan2(force.y, force.x);
        float forceSize  = glm::length(force);

        float momentOfInertia = glm::third<float>() * weight * b.lengthPx *
                                b.lengthPx;
        momentOfInertia = momentOfInertia == 0.0f ? 1.0 : momentOfInertia;

        float angularAcc =
            b.lengthPx * forceSize *
            glm::sin(forceAngle - b.absAngleNorm * glm::two_pi<float>()) /
            momentOfInertia;

        float angleDiffToRestNorm = angularDifference(
            glm::fract(parent.absAngleNorm + b.relAngleNorm), b.absAngleNorm
        );

        // If bent too much or parent already broke
        if (glm::abs(angleDiffToRestNorm) > 0.45 || !parent.living) {
            // Break the branch
            b.living = false;
        } else {
            // Regular bending
            angularAcc += b.stiffness * angleDiffToRestNorm - b.angleVelNorm * 0.2f;

            b.angleVelNorm += angularAcc;
            b.absAngleNorm += b.angleVelNorm;
            b.absAngleNorm = glm::fract(b.absAngleNorm);

            b.absPosPx = parent.absPosPx +
                         toCartesian(b.lengthPx, parent.absAngleNorm + b.relAngleNorm);
        }

        // Store the modified branch
        writeBranches()[i] = b;
    }
}

void TreesPoC::draw(const vk::CommandBuffer& commandBuffer, const glm::mat4& mvpMat) {
    m_gb.begin();
    std::array<re::VertexPoCo, 8> vertices;
    std::for_each(readBranches().begin(), readBranches().end(), [&](const Branch& b) {
        if (!b.living)
            return;
        const auto& parent = readBranches()[b.parentIndex];
        auto tl = b.absPosPx + toCartesian(b.radiusPx, b.absAngleNorm + 0.25);
        auto tr = b.absPosPx + toCartesian(b.radiusPx, b.absAngleNorm - 0.25);
        auto bl = parent.absPosPx + toCartesian(b.radiusPx, b.absAngleNorm + 0.25);
        auto br = parent.absPosPx + toCartesian(b.radiusPx, b.absAngleNorm - 0.25);

        vertices[0] = re::VertexPoCo{bl, k_red};
        vertices[1] = re::VertexPoCo{br, k_red};
        vertices[2] = re::VertexPoCo{br, k_red};
        vertices[3] = re::VertexPoCo{tr, k_red};
        vertices[4] = re::VertexPoCo{tr, k_red};
        vertices[5] = re::VertexPoCo{tl, k_red};
        vertices[6] = re::VertexPoCo{tl, k_red};
        vertices[7] = re::VertexPoCo{bl, k_red};
        m_gb.addVertices(vertices);
    });
    m_gb.end();
    m_gb.draw(commandBuffer, mvpMat);
}

void TreesPoC::addBranch(
    float        lengthPx,
    float        radiusPx,
    float        density,
    float        stiffness,
    float        angleNorm,
    unsigned int parentIndex,
    glm::vec2    absPosPx /*= {}*/
) {
    float absAngleNorm{};
    float relAngleNorm{};
    if (parentIndex != ~0u) { // If it is child branch
        const Branch& parent = m_branches[0][parentIndex];
        absAngleNorm         = glm::fract(parent.absAngleNorm + angleNorm);
        relAngleNorm         = glm::fract(angleNorm);
        absPosPx = parent.absPosPx + toCartesian(lengthPx, absAngleNorm);
    } else { // If it root branch
        absAngleNorm = glm::fract(angleNorm);
        relAngleNorm = 0.0;
    }

    Branch b{
        absPosPx,
        lengthPx,
        radiusPx,
        density,
        stiffness,
        absAngleNorm,
        relAngleNorm,
        0.0,
        parentIndex != ~0 ? parentIndex : (unsigned)m_branches[0].size(),
        true};

    m_branches[0].push_back(b);
    m_branches[1].push_back(b);
}

} // namespace rw
