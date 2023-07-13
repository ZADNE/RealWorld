/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/batches/GeometryBatch.hpp>

namespace rw {

class TreesPoC {
public:
    TreesPoC();

    void step();

    void draw(const vk::CommandBuffer& commandBuffer, const glm::mat4& mvpMat);

private:
    struct Branch {
        float        stiffness;
        float        weight;
        glm::vec2    absPosPx; // Absolute
        float        lengthPx;
        float        absAngleNorm; // Absolute
        float        relAngleNorm; // Relative to parent
        float        angleVelNorm;
        unsigned int parentIndex;
    };

    std::array<std::vector<Branch>, 2> m_branches;
    unsigned int                       m_readIndex = 1;
    float                              m_time      = 0.0;
    re::GeometryBatch m_gb{vk::PrimitiveTopology::eLineList, 1024, 1.0};

    const std::vector<Branch>& readBranches() const {
        return m_branches[m_readIndex];
    }

    std::vector<Branch>& writeBranches() { return m_branches[1 - m_readIndex]; }

    void addBranch(
        float        lengthPx,
        float        radiusPx,
        float        angleNorm,
        unsigned int parentIndex,
        glm::vec2    absPosPx = {}
    );
};

} // namespace rw