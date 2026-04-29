#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/UniformBuffer.hpp>
#include <LavaCake/Image.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

#include "utils/MipmapHelper.hpp"

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP4 - Normal Mapping", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Plane mesh ----
        std::vector<float> positions = {
            -1.0f, 0.0f, -1.0f,
             1.0f, 0.0f, -1.0f,
             1.0f, 0.0f,  1.0f,
            -1.0f, 0.0f,  1.0f,
        };
        std::vector<float> normals = {
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
        };
        std::vector<float> texcoords = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };
        std::vector<uint32_t> positionIndices = { 0, 1, 2, 0, 2, 3 };
        std::vector<uint32_t> normalIndices   = { 0, 1, 2, 0, 2, 3 };
        std::vector<uint32_t> texcoordIndices = { 0, 1, 2, 0, 2, 3 };

        // ---- Buffers de stockage ----
        LavaCake::Buffer posBuffer   (device, positions,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmBuffer   (device, normals,         vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texBuffer   (device, texcoords,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer posIdxBuffer(device, positionIndices, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmIdxBuffer(device, normalIndices,   vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texIdxBuffer(device, texcoordIndices, vk::BufferUsageFlagBits::eStorageBuffer);

        // ---- Checkerboard texture ----
        int texW = 2048, texH = 2048;
        std::vector<uint8_t> pixels(texW * texH * 4);
        for (int y = 0; y < texH; ++y) {
            for (int x = 0; x < texW; ++x) {
                int idx = (y * texW + x) * 4;
                bool white = ((x / 4) + (y / 4)) % 2 == 0;
                uint8_t c = white ? 255 : 0;
                pixels[idx+0] = c;
                pixels[idx+1] = c;
                pixels[idx+2] = c;
                pixels[idx+3] = 255;
            }
        }

        uint32_t mipLevels = mipLevelCount(static_cast<uint32_t>(texW),
                                           static_cast<uint32_t>(texH));

        LavaCake::Image texture(device,
                                static_cast<uint32_t>(texW),
                                static_cast<uint32_t>(texH),
                                1,
                                vk::Format::eR8G8B8A8Srgb,
                                vk::ImageUsageFlagBits::eSampled |
                                vk::ImageUsageFlagBits::eTransferDst |
                                vk::ImageUsageFlagBits::eTransferSrc,
                                vk::AllocationCreateFlagBits::eCreateDedicatedMemory,
                                mipLevels);

        {
            LavaCake::Buffer staging(device, pixels,
                                     vk::BufferUsageFlagBits::eTransferSrc,
                                     vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

            LavaCake::CommandBuffer uploadCmd(device.getDevice(), device.getCommandPool(), false);
            uploadCmd.begin();

            vk::ImageMemoryBarrier barrierUndef{};
            barrierUndef.image                           = texture;
            barrierUndef.srcAccessMask                   = vk::AccessFlagBits::eNone;
            barrierUndef.dstAccessMask                   = vk::AccessFlagBits::eTransferWrite;
            barrierUndef.oldLayout                       = vk::ImageLayout::eUndefined;
            barrierUndef.newLayout                       = vk::ImageLayout::eTransferDstOptimal;
            barrierUndef.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrierUndef.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrierUndef.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
            barrierUndef.subresourceRange.baseMipLevel   = 0;
            barrierUndef.subresourceRange.levelCount     = mipLevels;
            barrierUndef.subresourceRange.baseArrayLayer = 0;
            barrierUndef.subresourceRange.layerCount     = 1;

            uploadCmd.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                {}, {}, {},
                { barrierUndef }
            );

            vk::CommandBuffer vkUpload = uploadCmd;
            vk::BufferImageCopy region{};
            region.bufferOffset                    = 0;
            region.bufferRowLength                 = 0;
            region.bufferImageHeight               = 0;
            region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel       = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount     = 1;
            region.imageOffset                     = vk::Offset3D{0, 0, 0};
            region.imageExtent                     = vk::Extent3D{static_cast<uint32_t>(texW),
                                                                  static_cast<uint32_t>(texH), 1};
            vkUpload.copyBufferToImage(staging, (vk::Image)texture,
                                       vk::ImageLayout::eTransferDstOptimal, { region });

            uploadCmd.end();

            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = uploadCmd;
            device.getAnyQueue().submit(submitInfo, {});
            device.getAnyQueue().waitIdle();
        }

        generateMipmaps((vk::Image)texture,
                        static_cast<uint32_t>(texW),
                        static_cast<uint32_t>(texH),
                        mipLevels, device);

        LavaCake::ImageView texView(texture);
        LavaCake::Sampler   texSampler(device);

        // ---- Uniform buffer : viewProj (mis a jour chaque frame) ----
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        projection[1][1] *= -1.0f;

        LavaCake::UniformBuffer viewProjUbo(device);
        glm::mat4 initialViewProj = glm::mat4(1.0f);
        viewProjUbo.addVariable("viewProj", initialViewProj);
        viewProjUbo.end();

        // ---- Descriptor set layout ----
        LavaCake::DescriptorSetLayout layout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(3, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(4, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(5, vk::ShaderStageFlagBits::eVertex)
                .addUniformBuffer(6, vk::ShaderStageFlagBits::eVertex)
                .addCombinedImageSampler(7, vk::ShaderStageFlagBits::eFragment) // diffuseTexture
                .build();

        LavaCake::DescriptorPool pool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(6)
                .addUniformBuffers(1)
                .addCombinedImageSamplers(1)
                .setMaxSets(1)
                .build();

        vk::DescriptorSet descriptorSet = pool.allocate(layout);
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, posBuffer)
            .bindStorageBuffer(1, nrmBuffer)
            .bindStorageBuffer(2, texBuffer)
            .bindStorageBuffer(3, posIdxBuffer)
            .bindStorageBuffer(4, nrmIdxBuffer)
            .bindStorageBuffer(5, texIdxBuffer)
            .bindUniformBuffer(6, viewProjUbo)
            .bindImage(7, texView, texSampler)
            .update();

        // ---- Depth image ----
        vk::Extent2D extent = device.getSwapchainExtent();
        LavaCake::Image depthImage(device, extent.width, extent.height, 1,
            vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        LavaCake::ImageView depthView(depthImage, vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eDepth);

        // ---- Pipeline ----
        LavaCake::GraphicsPipeline pipeline =
            LavaCake::GraphicsPipeline::Builder(device)
                .addShaderFromFile(root + "shaders/TP4/obj_mip.vert",
                                   vk::ShaderStageFlagBits::eVertex,
                                   LavaCake::ShadingLanguage::eGLSL)
                .addShaderFromFile(root + "shaders/TP4/obj_mip.frag",
                                   vk::ShaderStageFlagBits::eFragment,
                                   LavaCake::ShadingLanguage::eGLSL)
                .addDescriptorSetLayout(layout)
                .addPushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4))
                .addColorAttachmentFormat(device.getSwapchainFormat())
                .setDepthAttachmentFormat(vk::Format::eD32Sfloat)
                .setDepthTest(true)
                .setCullMode(vk::CullModeFlagBits::eNone)
                .build();

        // ---- Semaphores ----
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);
        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }
        uint32_t currentFrame = 0;

        uint32_t vertexCount = static_cast<uint32_t>(positionIndices.size());
        float time = 0.0f;

        // ---- Boucle de rendu ----
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            time += 0.01f;

            // Camera oscillante
            float camDist = 1.5f + 20.0f * (0.5f + 0.5f * std::sin(time * 0.5f));
            camDist = 10.0f;
            glm::vec3 eye = glm::normalize(glm::vec3(1.0f, 0.1f, 1.0f)) * camDist;
            glm::mat4 view = glm::lookAt(eye,
                                         glm::vec3(0.0f, 0.0f, 0.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 viewProj = projection * view;

            glm::mat4 model = // glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 1.0f, 0.0f)) *
                              // glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(30.0f));

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            LavaCake::SwapChainImage& swapchainImage =
                device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            viewProjUbo.setVariable("viewProj", viewProj);
            viewProjUbo.update(cmdBuffer);

            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            LavaCake::DynamicRenderingContext renderingContext =
                LavaCake::DynamicRenderingContext::Builder()
                    .setRenderArea(device.getSwapchainExtent())
                    .addColorAttachment(swapchainImage,
                        vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}))
                    .setDepthAttachment(depthView, 1.0f)
                    .begin(cmdBuffer);

            renderingContext.setDefaultViewportScissor(cmdBuffer);

            pipeline.bind(cmdBuffer);
            pipeline.bindDescriptorSets(cmdBuffer, {descriptorSet});
            pipeline.pushConstants(cmdBuffer, vk::ShaderStageFlagBits::eVertex, 0, model);
            pipeline.draw(cmdBuffer, vertexCount);

            renderingContext.end(cmdBuffer);
            swapchainImage.prepareForPresentBarrier(cmdBuffer);
            cmdBuffer.end();

            vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submitInfo;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &imageAvailableSemaphores[currentFrame];
            submitInfo.pWaitDstStageMask    = &waitStage;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = cmdBuffer;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &renderFinishedSemaphores[currentFrame];

            device.getGraphicQueue(0).submit(submitInfo, cmdBuffer.getFence());
            cmdBuffer.markSubmitted();
            device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});

            currentFrame = (currentFrame + 1) % swapchainImageCount;
        }

        device.waitForAllCommands();

        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }

    } // Les objets GPU sont detruits ici

    device.releaseDevice();
    return 0;
}
