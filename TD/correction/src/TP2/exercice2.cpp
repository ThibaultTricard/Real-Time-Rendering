#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>
#include <iostream>


std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "02 - Colored Triangle Example", nullptr, nullptr);

    bool framebufferResized = false;
    glfwSetWindowUserPointer(window, &framebufferResized);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int, int) {
        *static_cast<bool*>(glfwGetWindowUserPointer(win)) = true;
    });

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig,1);
    { // Create a context to make sure all GPU objects are destroyed before we release the device

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example 02: Colored Triangle\n";

        
        std::vector<float> position = {
           0.0, -0.5,
           0.5,  0.5,
          -0.5,  0.5,
        };

         std::vector<float> color = {
           1.0, 0.0, 0.0,
           0.0, 1.0, 0.0,
          -0.0, 0.0, 1.0
        };


        // Create storage buffers for vertex pulling
        LavaCake::Buffer positionBuffer(device, position, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer colorBuffer(device, color, vk::BufferUsageFlagBits::eStorageBuffer);


        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)  // A
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)  // B
                .build();

        // Create descriptor pool
        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(2)
                .setMaxSets(1)
                .build();

        // Allocate descriptor set
        vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorSetLayout);

        // Update descriptor set with our buffers
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, colorBuffer)
            .update();

        // Graphic Pipeline creation
        LavaCake::GraphicsPipeline graphicPipeline  = LavaCake::GraphicsPipeline::Builder(device)
                            .addShaderFromFile(root + "shaders/TP2/triangle2.vert",vk::ShaderStageFlagBits::eVertex,LavaCake::ShadingLanguage::eGLSL)
                            .addShaderFromFile(root + "shaders/TP2/triangle.frag",vk::ShaderStageFlagBits::eFragment,LavaCake::ShadingLanguage::eGLSL)
                            .addDescriptorSetLayout(descriptorSetLayout)
                            .addColorAttachmentFormat(device.getSwapchainFormat())
                            .setCullMode(vk::CullModeFlagBits::eNone)
                            .build();

        // Create semaphores per swapchain image to avoid reuse conflicts
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);

        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }
        uint32_t currentFrame = 0;

        // Main render loop
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            // Wait for previous frame to complete
            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            // Acquire next swapchain image using current frame's semaphore
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            // Begin recording commands
            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            // Begin dynamic rendering with cornflower blue clear color
            LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.39f, 0.58f, 0.93f, 1.0f}) // Cornflower blue!
                )
                .begin(cmdBuffer);

            // Set the Viewport and Scissor to for rendering on the whole image
            renderingContext.setDefaultViewportScissor(cmdBuffer);

            // binding the pipeline
            graphicPipeline.bind(cmdBuffer);
            graphicPipeline.bindDescriptorSets(cmdBuffer, {descriptorSet});

            // draw call for three vertices
            graphicPipeline.draw(cmdBuffer,3);

            // End rendering
            renderingContext.end(cmdBuffer);

            // Transition swapchain image to PRESENT layout
            swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Submit command buffer with fence using current frame's semaphores
            vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submitInfo;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
            submitInfo.pWaitDstStageMask = &waitStage;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = cmdBuffer;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

            device.getGraphicQueue(0).submit(submitInfo, cmdBuffer.getFence());
            cmdBuffer.markSubmitted();

            // Present the image
            device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});

            

            // Advance to next frame
            currentFrame = (currentFrame + 1) % swapchainImageCount;
        }

        // Wait for device to finish before cleanup
        device.waitForAllCommands();

        // Clean up all semaphores
        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }
    } // GPU objects destroyed here before device

    device.releaseDevice();
    std::cout << "Example completed successfully!\n";
    

    return 0;
}