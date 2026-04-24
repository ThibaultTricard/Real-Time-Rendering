#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <iostream>
#include <vector>
#include <cmath>

std::string root = PROJECT_ROOT;


void plotSignal(std::vector<float> V){
    const int W = 80, H = 15;
    float vmin = *std::min_element(V.begin(), V.end());
    float vmax = *std::max_element(V.begin(), V.end());
    if (vmax == vmin) vmax = vmin + 1.0f;
    std::cout << "\n--- Signal (ASCII art) ---\n";
    for (int row = H - 1; row >= 0; row--) {
        std::cout << ' ';
        for (int col = 0; col < W; col++) {
            int idx = col;
            int valueRow = (int)std::round((V[idx] - vmin) / (vmax - vmin) * (H - 1));
            if(row < valueRow){
                std::cout << '|';
            }else if(row == valueRow){
                std::cout << '*';
            }else {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
    std::cout << '+';
    for (int col = 0; col < W; col++) std::cout << '-';
    std::cout << '\n';
}

using namespace LavaCake;

int main() {

    LavaCake::Device device = LavaCake::createHeadlessDevice(0, 1);

    {
        int size = 1000;

        std::vector<float> A(size);
        for (int i = 0; i < size; i++)
            A[i] = (i / 8) % 2==0 ? 1.0f : 0.0f;


        LavaCake::Buffer ABuffer(device, A, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer BBuffer(device, size * sizeof(float),
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
            vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);
        LavaCake::Buffer CBuffer(device, size * sizeof(float),
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
            vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);
        LavaCake::Buffer DBuffer(device, size * sizeof(float),
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
            vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eCompute)
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eCompute)
                .build();

        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(6)
                .setMaxSets(3)
                .build();

        // Passe 1 : A -> B
        vk::DescriptorSet set0 = descriptorPool.allocate(descriptorSetLayout);
        LavaCake::DescriptorSetUpdater(device, set0)
            .bindStorageBuffer(0, ABuffer)
            .bindStorageBuffer(1, BBuffer)
            .update();

        // Passe 2 : B -> C
        vk::DescriptorSet set1 = descriptorPool.allocate(descriptorSetLayout);
        LavaCake::DescriptorSetUpdater(device, set1)
            .bindStorageBuffer(0, BBuffer)
            .bindStorageBuffer(1, CBuffer)
            .update();

        // Passe 3 : C -> D
        vk::DescriptorSet set2 = descriptorPool.allocate(descriptorSetLayout);
        LavaCake::DescriptorSetUpdater(device, set2)
            .bindStorageBuffer(0, CBuffer)
            .bindStorageBuffer(1, DBuffer)
            .update();

        LavaCake::ComputePipeline filterPipeline =
            LavaCake::ComputePipeline::Builder(device)
                .setShaderFromFile(root + "shaders/TP1/boxfilter.comp",
                                   LavaCake::ShadingLanguage::eGLSL)
                .addDescriptorSetLayout(descriptorSetLayout)
                .build();

        vk::MemoryBarrier memoryBarrier{
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        };

        LavaCake::CommandBuffer cmdBuffer(device, true);
        cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        // Passe 1 : A -> B
        filterPipeline.bind(cmdBuffer);
        filterPipeline.bindDescriptorSets(cmdBuffer, {set0});
        filterPipeline.dispatch(cmdBuffer, size);

        
        cmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            {}, memoryBarrier, {}, {}
        );

        // Passe 2 : B -> C
        filterPipeline.bind(cmdBuffer);
        filterPipeline.bindDescriptorSets(cmdBuffer, {set1});
        filterPipeline.dispatch(cmdBuffer, size);

        cmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            {}, memoryBarrier, {}, {}
        );

        // Passe 3 : C -> D
        filterPipeline.bind(cmdBuffer);
        filterPipeline.bindDescriptorSets(cmdBuffer, {set2});
        filterPipeline.dispatch(cmdBuffer, size);
        

        cmdBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = cmdBuffer;
        device.getComputeQueue(0).submit(submitInfo, cmdBuffer.getFence());
        cmdBuffer.markSubmitted();
        cmdBuffer.waitForCompletion();

        void* data = DBuffer.map();
        std::vector<float> D(size);
        memcpy(D.data(), data, size * sizeof(float));
        BBuffer.unmap();

        
        plotSignal(D);


        

        device.waitForAllCommands();

    } // Les objets GPU sont detruits ici

    device.releaseDevice();
    return 0;
}
