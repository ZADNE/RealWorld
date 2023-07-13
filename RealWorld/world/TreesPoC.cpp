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
    addBranch(0.0, 0.0, 0.25, ~0u, {960.0, 100.0});
    struct TurtleState {
        float        angleNorm;
        float        radiusPx;
        unsigned int parentIndex;
    };
    std::stack<TurtleState> stack;
    stack.emplace(0.0f, 10.0f, 0u);
    for (char c : sentence) {
        switch (c) {
        case 'F':
            addBranch(
                60.0, stack.top().radiusPx, stack.top().angleNorm, stack.top().parentIndex
            );
            stack.top().angleNorm   = 0.0;
            stack.top().parentIndex = m_branches[0].size() - 1;
            stack.top().radiusPx *= 0.75f;
            break;
        case '+': stack.top().angleNorm += 0.05; break;
        case '-': stack.top().angleNorm -= 0.05; break;
        case '[': stack.emplace(stack.top()); break;
        case ']': stack.pop(); break;
        default: break;
        }
    }
}

void TreesPoC::step() {
    // Swap read index
    m_readIndex = 1 - m_readIndex;
    m_time += 0.001;

    std::transform(
        readBranches().begin(),
        readBranches().end(),
        writeBranches().begin(),
        [&](const Branch& in) {
            Branch        out    = in;
            const Branch& parent = readBranches()[out.parentIndex];
            float         wind   = glm::perlin(glm::vec2{m_time, 0.0});
            wind += 0.1 * glm::perlin(glm::vec2{m_time * 20.0, 0.0f});

            glm::vec2 force = glm::vec2{wind * 10.0f, out.weight * -0.0001f};

            float forceAngle = std::atan2(force.y, force.x);
            float forceSize  = glm::length(force);

            float momentOfInertia = glm::third<float>() * out.weight *
                                    out.lengthPx * out.lengthPx;
            momentOfInertia = momentOfInertia == 0.0f ? 1.0 : momentOfInertia;

            float angularAcc =
                out.lengthPx * forceSize *
                glm::sin(forceAngle - out.absAngleNorm * glm::two_pi<float>()) /
                momentOfInertia;
            angularAcc += out.stiffness *
                              angularDifference(
                                  parent.absAngleNorm + out.relAngleNorm, out.absAngleNorm
                              ) -
                          out.angleVelNorm * 0.2;

            out.angleVelNorm += angularAcc;
            out.absAngleNorm += out.angleVelNorm;
            out.absAngleNorm = glm::fract(out.absAngleNorm);

            out.absPosPx =
                parent.absPosPx +
                toCartesian(out.lengthPx, parent.absAngleNorm + out.relAngleNorm);

            return out;
        }
    );
}

void TreesPoC::draw(const vk::CommandBuffer& commandBuffer, const glm::mat4& mvpMat) {
    m_gb.begin();
    std::array<re::VertexPoCo, 8> vertices;
    std::for_each(readBranches().begin(), readBranches().end(), [&](const Branch& b) {
        const auto& parent = readBranches()[b.parentIndex];
        float radiusPx = glm::sqrt(b.weight / (glm::pi<float>() * b.lengthPx));
        auto  bl = b.absPosPx + toCartesian(radiusPx, b.absAngleNorm + 0.25);
        auto  br = b.absPosPx + toCartesian(radiusPx, b.absAngleNorm - 0.25);
        auto tl = parent.absPosPx + toCartesian(radiusPx, b.absAngleNorm + 0.25);
        auto tr = parent.absPosPx + toCartesian(radiusPx, b.absAngleNorm - 0.25);

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
    float        angleNorm,
    unsigned int parentIndex,
    glm::vec2    absPosPx /* = {}*/
) {
    float weight    = glm::pi<float>() * radiusPx * radiusPx * lengthPx;
    float stiffness = weight * 0.00005;

    float absAngleNorm{};
    float relAngleNorm{};
    if (parentIndex != ~0u) { // If it is child branch
        const Branch& parent = m_branches[0][parentIndex];
        absAngleNorm         = glm::fract(parent.absAngleNorm + angleNorm);
        relAngleNorm         = glm::fract(angleNorm);
        absPosPx = parent.absPosPx + toCartesian(lengthPx, absAngleNorm);
    } else { // If it root branch
        absAngleNorm = glm::fract(angleNorm);
        relAngleNorm = glm::fract(angleNorm);
    }

    Branch b{
        stiffness,
        weight,
        absPosPx,
        lengthPx,
        absAngleNorm,
        relAngleNorm,
        0.0f,
        parentIndex == ~0 ? (unsigned)m_branches[0].size() : parentIndex};

    m_branches[0].push_back(b);
    m_branches[1].push_back(b);
}

} // namespace rw
