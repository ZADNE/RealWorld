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
        glm::vec2    absPosPx; // Absolute
        float        lengthPx;
        float        radiusPx;
        float        density;
        float        stiffness;
        float        absAngleNorm; // Absolute
        float        relRestAngleNorm; // Relative to parent
        float        angleVelNorm; // Absolute
        unsigned int parentIndex;
        bool         living;
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
        float        density,
        float        stiffness,
        float        angleNorm,
        unsigned int parentIndex,
        glm::vec2    absPosPx = {}
    );
};

} // namespace rw