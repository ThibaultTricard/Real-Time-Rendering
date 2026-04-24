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
                .addStorageBuffers(3)
                .setMaxSets(1)
                .build();

        // Allocate descriptor set
        vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorSetLayout);

        // Update descriptor set with our buffers
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, ABuffer)
            .bindStorageBuffer(1, BBuffer)
            .bindStorageBuffer(2, CBuffer)
            .update();

        std::cout << "Descriptor set configured" << std::endl;

        // Create graphics pipeline with vertex pulling (no vertex input)
        LavaCake::ComputePipeline pipeline = LavaCake::ComputePipeline::Builder(device)
            .setShaderFromFile(root + "shaders/TP1/addition.comp", LavaCake::ShadingLanguage::eGLSL)
            .addDescriptorSetLayout(descriptorSetLayout)
            .build();

    
        LavaCake::CommandBuffer cmdBuffer(device, true);
        // Begin recording commands
        cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);


        // Bind the pipeline
        pipeline.bind(cmdBuffer);
        pipeline.bindDescriptorSets(cmdBuffer, {descriptorSet});
      
        pipeline.dispatch(cmdBuffer,size);


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


        // Wait for device to finish before cleanup
        device.waitForAllCommands();
    } // GPU objects destroyed here
    
    device.releaseDevice();

    std::cout << "Triangle example closed successfully!" << std::endl;

    return 0;
}
