/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/shaders/VegPrepSB.glsl.hpp>

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

void ChunkGenerator::generateVegetation(const ActionCmdBuf& acb) {
    { // Clear count
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(glsl::VegPrepSB, vegDispatchSize.x),
            sizeof(glsl::VegPrepSB::vegDispatchSize.x), 0
        );
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(glsl::VegPrepSB, branchDispatchSize.x),
            sizeof(glsl::VegPrepSB::branchDispatchSize.x), 0
        );
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(glsl::VegPrepSB, vegOffsetWithinChunk),
            sizeof(glsl::VegPrepSB::vegOffsetWithinChunk), 0
        );
        (*acb)->fillBuffer(
            *m_vegPrepBuf, offsetof(glsl::VegPrepSB, branchOfChunk),
            sizeof(glsl::VegPrepSB::branchOfChunk), 0
        );
        auto barrier = re::bufferMemoryBarrier(
            S::eTransfer,                                   // Src stage mask
            A::eTransferWrite,                              // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            *m_vegPrepBuf
        );
        (*acb)->pipelineBarrier2({{}, {}, barrier, {}});
    }

    // Select species
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVegSpeciesPl);
    (*acb)->dispatch(1u, 1u, m_chunksPlanned);

    { // Add barrier between species selection and their expansion
        auto barrier = re::bufferMemoryBarrier(
            S::eComputeShader,                              // Src stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
            S::eDrawIndirect | S::eComputeShader,           // Dst stage mask
            A::eIndirectCommandRead | A::eShaderStorageRead |
                A::eShaderStorageWrite,                     // Dst access mask
            *m_vegPrepBuf
        );
        (*acb)->pipelineBarrier2({{}, {}, barrier, {}});
    }

    // Expand species instances
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_expandVegInstancesPl);
    (*acb)->dispatchIndirect(*m_vegPrepBuf, offsetof(glsl::VegPrepSB, vegDispatchSize));

    acb.action(
        [&](const re::CommandBuffer& cb) {
            { // Add barrier between expansion and branch allocation
                auto barrier = re::bufferMemoryBarrier(
                    S::eComputeShader,                    // Src stage mask
                    A::eShaderStorageRead |
                        A::eShaderStorageWrite,           // Src access mask
                    S::eDrawIndirect | S::eComputeShader, // Dst stage mask
                    A::eIndirectCommandRead | A::eShaderStorageRead |
                        A::eShaderStorageWrite,           // Dst access mask
                    *m_vegPrepBuf
                );
                cb->pipelineBarrier2({{}, {}, barrier, {}});
            }

            // Allocate branches
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_allocBranchesPl);
            cb->dispatch(1, 1, 1);

            { // Add barrier between branch allocation and output
                auto barrier = re::bufferMemoryBarrier(
                    S::eComputeShader,          // Src stage mask
                    A::eShaderStorageRead |
                        A::eShaderStorageWrite, // Src access mask
                    S::eComputeShader,          // Dst stage mask
                    A::eShaderStorageRead |
                        A::eShaderStorageWrite, // Dst access mask
                    *m_vegPrepBuf
                );
                cb->pipelineBarrier2({{}, {}, barrier, {}});
            }

            // Output branches
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_outputBranchesPl);
            cb->dispatchIndirect(
                *m_vegPrepBuf, offsetof(glsl::VegPrepSB, branchDispatchSize)
            );
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

} // namespace rw
