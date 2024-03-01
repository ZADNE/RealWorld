/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/VegPrepSB.hpp>
#include <RealWorld/generation/VegTemplatesUB.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

void ChunkGenerator::generateVegetation(const ActionCmdBuf& acb) {
    { // Clear count
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(VegPrepSB, vegDispatchSize.x),
            sizeof(VegPrepSB::vegDispatchSize.x), 0
        );
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(VegPrepSB, branchDispatchSize.x),
            sizeof(VegPrepSB::branchDispatchSize.x), 0
        );
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(VegPrepSB, vegOffsetWithinChunk),
            sizeof(VegPrepSB::vegOffsetWithinChunk), 0
        );
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(VegPrepSB, branchOfChunk),
            sizeof(VegPrepSB::branchOfChunk), 0
        );
        auto clearCountsBarrier = re::bufferMemoryBarrier(
            S::eTransfer,                                   // Src stage mask
            A::eTransferWrite,                              // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            *m_vegPrepBuf
        );
        (*acb)->pipelineBarrier2({{}, {}, clearCountsBarrier, {}});
    }

    // Select vegetation
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVegPl);
    (*acb)->dispatch(1u, 1u, m_chunksPlanned);

    { // Add barrier between vegetation selection and L-system expansion
        auto preparationBarrier = re::bufferMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eDrawIndirect | S::eComputeShader,           // Dst stage mask
            A::eIndirectCommandRead | A::eShaderStorageRead |
                A::eShaderStorageWrite,                     // Dst access mask
            *m_vegPrepBuf
        );
        (*acb)->pipelineBarrier2({{}, {}, preparationBarrier, {}});
    }

    // Expand L-systems
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_expandLSystemsPl);
    (*acb)->dispatchIndirect(*m_vegPrepBuf, offsetof(VegPrepSB, vegDispatchSize));

    acb.action(
        [&](const re::CommandBuffer& cb) {
            { // Add barrier between L-system expansion and branch allocation
                auto vectorBarrier = re::bufferMemoryBarrier(
                    S::eComputeShader,                    // Src stage mask
                    A::eShaderStorageRead |
                        A::eShaderStorageWrite,           // Src access mask
                    S::eDrawIndirect | S::eComputeShader, // Dst stage mask
                    A::eIndirectCommandRead | A::eShaderStorageRead |
                        A::eShaderStorageWrite,           // Dst access mask
                    *m_vegPrepBuf
                );
                cb->pipelineBarrier2({{}, {}, vectorBarrier, {}});
            }

            // Allocate branches
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_allocBranchesPl);
            cb->dispatch(1, 1, 1);

            { // Add barrier between branch allocation and output
                auto vectorBarrier = re::bufferMemoryBarrier(
                    S::eComputeShader,          // Src stage mask
                    A::eShaderStorageRead |
                        A::eShaderStorageWrite, // Src access mask
                    S::eComputeShader,          // Dst stage mask
                    A::eShaderStorageRead |
                        A::eShaderStorageWrite, // Dst access mask
                    *m_vegPrepBuf
                );
                cb->pipelineBarrier2({{}, {}, vectorBarrier, {}});
            }

            // Output branches
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_outputBranchesPl);
            cb->dispatchIndirect(*m_vegPrepBuf, offsetof(VegPrepSB, branchDispatchSize));
        },
        BufferAccess{
            .name   = BufferTrackName::Branch,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        },
        BufferAccess{
            .name   = BufferTrackName::AllocReg,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        }
    );
}

constexpr VegTemplatesUB composeVegTemplates() {
    using namespace std::string_view_literals;
    std::string symbols;
    std::vector<float> probs;
    std::vector<String> bodies;
    std::vector<VegTemplate> tmplts;

    auto addString = [&symbols](std::string_view sv) -> String {
        glm::uint offset = symbols.size();
        symbols.append(sv);
        return String{offset, static_cast<glm::uint>(sv.size())};
    };

    auto addProbRuleBodies =
        [&probs, &bodies](std::initializer_list<std::pair<float, String>> probBodies
        ) -> ProbabilisticRewriteRuleBodies {
        glm::uint first = probs.size();
        for (const auto& pair : probBodies) {
            probs.emplace_back(pair.first);
            bodies.emplace_back(pair.second);
        }
        return ProbabilisticRewriteRuleBodies{
            .firstIndex = first,
            .bodyCount  = static_cast<glm::uint>(probBodies.size())
        };
    };

    auto addTemplate =
        [addString, &tmplts](
            std::string_view axiom,
            const std::array<ProbabilisticRewriteRuleBodies, k_rewriteableSymbolCount>&
                rewriteBodies,
            float alpha, glm::uint iterCount
        ) {
            tmplts.emplace_back(addString(axiom), rewriteBodies, alpha, iterCount);
        };

    auto oakRuleStr    = addString("SS-[-B+B+B]+[+B-B-B]"sv);
    auto acaciaRuleStr = addString("S[+B][-B]"sv);
    addTemplate(
        "B"sv, {addProbRuleBodies({std::make_pair(1.0f, oakRuleStr)})}, 1.0f, 2
    );
    addTemplate(
        "B"sv, {addProbRuleBodies({std::make_pair(1.0f, acaciaRuleStr)})}, 1.0f, 5
    );

    addTemplate(
        "B"sv, {addProbRuleBodies({std::make_pair(1.0f, oakRuleStr)})}, 1.0f, 2
    );

    return VegTemplatesUB{
        std::string_view{symbols}, std::span{probs}, std::span{bodies},
        std::span{tmplts}
    };
}

constexpr VegTemplatesUB k_templatesUB{composeVegTemplates()};

re::Buffer ChunkGenerator::createVegTemplatesBuffer() {
    return re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(k_templatesUB),
        .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
        .initData    = re::objectToByteSpan(k_templatesUB),
        .debugName   = "rw::ChunkGeneratorVeg::vegTemplates"
    }};
}

} // namespace rw
