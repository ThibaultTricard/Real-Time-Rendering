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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "utils/ObjLoader.hpp"
#include "utils/MipmapHelper.hpp"

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP4 - Textures", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Chargement du modele OBJ ----
        std::vector<float>    positions, normals, texcoords;
        std::vector<uint32_t> positionIndices, normalIndices, texcoordIndices;

        if (!loadModel(root + "models/Michelle.obj",
                       root + "models/",
                       positions, normals, texcoords,
                       positionIndices, normalIndices, texcoordIndices)) {
            std::cerr << "Erreur : impossible de charger le modele." << std::endl;
            return 1;
        }


        // ---- Calcul des tangentes ----
        // Tangent continuity breaks at UV seams (different ti) and sharp edges (different ni),
        // so the deduplication key is (pi, ni, ti) — the full vertex identity.
        std::map<std::tuple<uint32_t,uint32_t,uint32_t>, uint32_t> tangentIndexMap;
        std::vector<glm::vec3> tangentAccum;
        std::vector<uint32_t> tangentIndices(positionIndices.size());

        size_t numTriangles = positionIndices.size() / 3;
        for (size_t f = 0; f < numTriangles; f++) {
            uint32_t pi0 = positionIndices[3*f+0], pi1 = positionIndices[3*f+1], pi2 = positionIndices[3*f+2];
            uint32_t ni0 = normalIndices[3*f+0],   ni1 = normalIndices[3*f+1],   ni2 = normalIndices[3*f+2];
            uint32_t ti0 = texcoordIndices[3*f+0], ti1 = texcoordIndices[3*f+1], ti2 = texcoordIndices[3*f+2];

            glm::vec3 p0 = { positions[3*pi0], positions[3*pi0+1], positions[3*pi0+2] };
            glm::vec3 p1 = { positions[3*pi1], positions[3*pi1+1], positions[3*pi1+2] };
            glm::vec3 p2 = { positions[3*pi2], positions[3*pi2+1], positions[3*pi2+2] };

            glm::vec2 uv0 = { texcoords[2*ti0], texcoords[2*ti0+1] };
            glm::vec2 uv1 = { texcoords[2*ti1], texcoords[2*ti1+1] };
            glm::vec2 uv2 = { texcoords[2*ti2], texcoords[2*ti2+1] };

            glm::vec3 e1 = p1 - p0, e2 = p2 - p0;
            glm::vec2 d1 = uv1 - uv0, d2 = uv2 - uv0;

            float det = d1.x * d2.y - d2.x * d1.y;
            glm::vec3 T(0.0f);
            if (std::abs(det) > 1e-6f)
                T = (d2.y * e1 - d1.y * e2) / det;

            uint32_t pis[3] = {pi0, pi1, pi2};
            uint32_t nis[3] = {ni0, ni1, ni2};
            uint32_t tis[3] = {ti0, ti1, ti2};
            for (int v = 0; v < 3; v++) {
                auto key = std::make_tuple(pis[v], nis[v], tis[v]);
                auto [it, inserted] = tangentIndexMap.emplace(key, (uint32_t)tangentAccum.size());
                if (inserted) tangentAccum.push_back(T);
                else          tangentAccum[it->second] += T;
                tangentIndices[3*f+v] = it->second;
            }
        }

        std::vector<float> tangents;
        tangents.reserve(tangentAccum.size() * 3);
        for (auto& t : tangentAccum) {
            glm::vec3 tn = glm::normalize(t);
            tangents.push_back(tn.x); tangents.push_back(tn.y); tangents.push_back(tn.z);
        }

        // ---- Buffers de stockage ----
        LavaCake::Buffer posBuffer   (device, positions,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmBuffer   (device, normals,         vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texBuffer   (device, texcoords,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer posIdxBuffer(device, positionIndices, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmIdxBuffer(device, normalIndices,   vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texIdxBuffer(device, texcoordIndices, vk::BufferUsageFlagBits::eStorageBuffer);

        // ---- Chargement de la texture diffuse avec mipmaps ----
        std::string texPath = root + "models/Ch03_1001_Diffuse.png";
        int texW, texH, texCh;
        stbi_uc* px = stbi_load(texPath.c_str(), &texW, &texH, &texCh, STBI_rgb_alpha);
        if (!px) {
            std::cerr << "Erreur : impossible de charger la texture : " << texPath << std::endl;
            return 1;
        }
        std::vector<uint8_t> pixels(px, px + texW * texH * 4);
        stbi_image_free(px);

        uint32_t mipLevels = mipLevelCount(static_cast<uint32_t>(texW),
                                           static_cast<uint32_t>(texH));

        // Create image with all mip levels (no data — stays eUndefined)
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

        // Upload pixel data to mip level 0, leaving it in eTransferDstOptimal
        {
            LavaCake::Buffer staging(device, pixels,
                                     vk::BufferUsageFlagBits::eTransferSrc,
                                     vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

            LavaCake::CommandBuffer uploadCmd(device.getDevice(), device.getCommandPool(), false);
            uploadCmd.begin();

            // Transition all mip levels: eUndefined -> eTransferDstOptimal
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
                {},
                {},
                {},
                { barrierUndef }
            );

            // Copy pixels to mip level 0
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

        // Generate mipmaps (transitions all levels to eShaderReadOnlyOptimal)
        generateMipmaps((vk::Image)texture,
                        static_cast<uint32_t>(texW),
                        static_cast<uint32_t>(texH),
                        mipLevels, device);

        LavaCake::ImageView texView(texture);
        LavaCake::Sampler   texSampler(device); // full mipmap support

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
                .addCombinedImageSampler(7, vk::ShaderStageFlagBits::eFragment)
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
                .setCullMode(vk::CullModeFlagBits::eBack)
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
            float camDist = 1.5f + 10.0f * (0.5f + 0.5f * std::sin(time * 0.5f));
            glm::vec3 eye = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)) * camDist;
            glm::mat4 view = glm::lookAt(eye,
                                         glm::vec3(0.0f, 0.0f, 0.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 viewProj = projection * view;

            glm::mat4 model = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 1.0f, 0.0f))
                            * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

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
