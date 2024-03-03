/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/VegPrepSB.hpp>

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
        auto barrier = re::bufferMemoryBarrier(
            S::eTransfer,                                   // Src stage mask
            A::eTransferWrite,                              // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            *m_vegPrepBuf
        );
        (*acb)->pipelineBarrier2({{}, {}, barrier, {}});
    }

    // Select vegetation
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVegPl);
    (*acb)->dispatch(1u, 1u, m_chunksPlanned);

    { // Add barrier between vegetation selection and L-system expansion
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

    // Expand L-systems
    (*acb)->bindPipeline(vk::PipelineBindPoint::eCompute, *m_expandLSystemsPl);
    (*acb)->dispatchIndirect(*m_vegPrepBuf, offsetof(VegPrepSB, vegDispatchSize));

    acb.action(
        [&](const re::CommandBuffer& cb) {
            { // Add barrier between L-system expansion and branch allocation
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

} // namespace rw
