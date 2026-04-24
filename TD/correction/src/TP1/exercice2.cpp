#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <iostream>
#include <vector>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {
    
    // create a headles device with 0 graphic queue and 1 compute queue
    LavaCake::Device device = LavaCake::createHeadlessDevice(0,1);

    { // Scope for GPU object lifetime management

       
        int size = 1000;

        std::vector<float> A;
        std::vector<float> B;

        for(int i = 0; i < size; i++){
            A.push_back(i);
            B.push_back(i*2);
        }

        LavaCake::Buffer ABuffer(device, A, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer BBuffer(device, B, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer CBuffer(device, size * sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer , vk::AllocationCreateFlagBits::eCreateDedicatedMemory | vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);
        LavaCake::Buffer DBuffer(device, size * sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer , vk::AllocationCreateFlagBits::eCreateDedicatedMemory | vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

        // Create descriptor set layout for our storage buffers
        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eCompute)  // A
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eCompute)  // B
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eCompute)  // C
                .build();

        // Create descriptor pool
        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(4)
                .setMaxSets(2)
                .build();

        // Allocate descriptor set
        vk::DescriptorSet additionDescriptorSet = descriptorPool.allocate(descriptorSetLayout);

        // Update descriptor set with our buffers
        LavaCake::DescriptorSetUpdater(device, additionDescriptorSet)
            .bindStorageBuffer(0, ABuffer)
            .bindStorageBuffer(1, BBuffer)
            .bindStorageBuffer(2, CBuffer)
            .update();


         // Allocate descriptor set
        vk::DescriptorSet multiplicationDescriptorSet = descriptorPool.allocate(descriptorSetLayout);

        // Update descriptor set with our buffers
        LavaCake::DescriptorSetUpdater(device, multiplicationDescriptorSet)
            .bindStorageBuffer(0, ABuffer)
            .bindStorageBuffer(1, CBuffer)
            .bindStorageBuffer(2, DBuffer)
            .update();

        std::cout << "Descriptor set configured" << std::endl;

        // Create graphics pipeline with vertex pulling (no vertex input)
        LavaCake::ComputePipeline addtionPipeline = LavaCake::ComputePipeline::Builder(device)
            .setShaderFromFile(root + "shaders/TP1/addition.comp", LavaCake::ShadingLanguage::eGLSL)
            .addDescriptorSetLayout(descriptorSetLayout)
            .build();

        LavaCake::ComputePipeline multiplyPipeline = LavaCake::ComputePipeline::Builder(device)
            .setShaderFromFile(root + "shaders/TP1/multiply.comp", LavaCake::ShadingLanguage::eGLSL)
            .addDescriptorSetLayout(descriptorSetLayout)
            .build();

    
        LavaCake::CommandBuffer cmdBuffer(device, true);
        // Begin recording commands
        cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);


        // Bind the pipeline
        addtionPipeline.bind(cmdBuffer);
        addtionPipeline.bindDescriptorSets(cmdBuffer, {additionDescriptorSet});
        addtionPipeline.dispatch(cmdBuffer,size);

        // Wait for the addition compute shader to finish writing before the multiply reads
        vk::MemoryBarrier memoryBarrier{
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        };

        cmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            {},             // dependency flags
            memoryBarrier,  // memory barriers
            {},             // buffer memory barriers
            {}              // image memory barriers
        );


        // Bind the pipeline
        multiplyPipeline.bind(cmdBuffer);
        multiplyPipeline.bindDescriptorSets(cmdBuffer, {multiplicationDescriptorSet});
        multiplyPipeline.dispatch(cmdBuffer,size);

        cmdBuffer.end();
        // Submit command buffer
        vk::SubmitInfo submitInfo;
        submitInfo.pCommandBuffers = cmdBuffer;
        submitInfo.commandBufferCount = 1;

        device.getComputeQueue(0).submit(submitInfo, cmdBuffer.getFence());
        cmdBuffer.markSubmitted();


        cmdBuffer.waitForCompletion();

        
        void* data = CBuffer.map();
        std::vector<float> C(size);
        memcpy(C.data(), data, size * sizeof(float));
        CBuffer.unmap();

        data = DBuffer.map();
        std::vector<float> D(size);
        memcpy(D.data(), data, size * sizeof(float));
        DBuffer.unmap();

        // Verify results
        std::cout << "Verifying results...\n";
        bool success = true;
        for (uint32_t i = 0; i < size; i++) {
            float expected = A[i] + B[i];
            if (std::abs(C[i] - expected) > 0.0001f) {
                std::cerr << "Error at index " << i << ": expected " << expected
                            << ", got " << C[i] << "\n";
                success = false;
                break;
            }
        }
        if (success) {
            std::cout << "Success! All " << size << " results are correct.\n";
            std::cout << "C[0]=" << C[0] << " (expected " << (A[0] + B[0]) << ")\n";
            std::cout << "C[" << size-1 << "]=" << C[size-1]
                        << " (expected " << (A[size-1] + B[size-1]) << ")\n";
        }


        // Verify results
        std::cout << "Verifying results...\n";
        success = true;
        for (uint32_t i = 0; i < size; i++) {
            float expected = (A[i] + B[i]) * A[i];
            if (std::abs(D[i] - expected) > 0.0001f) {
                std::cerr << "Error at index " << i << ": expected " << expected
                            << ", got " << D[i] << "\n";
                success = false;
                break;
            }
        }
        if (success) {
            std::cout << "Success! All " << size << " results are correct.\n";
            std::cout << "D[0]=" << D[0] << " (expected " << (A[0] + B[0]) * A[0] << ")\n";
            std::cout << "D[" << size-1 << "]=" << D[size-1]
                        << " (expected " << ((A[size-1] + B[size-1]) * A[size-1]) << ")\n";
        }

        // Wait for device to finish before cleanup
        device.waitForAllCommands();
    } // GPU objects destroyed here
    
    device.releaseDevice();

    std::cout << "Triangle example closed successfully!" << std::endl;

    return 0;
}
