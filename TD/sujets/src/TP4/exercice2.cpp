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

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// ---------------------------------------------------------------------------
// TODO 1 : Implementer la generation de mipmaps par blits GPU successifs.
//
// Prototype :
//   void generateMipmaps(vk::Image image,
//                        uint32_t width, uint32_t height,
//                        uint32_t mipLevels,
//                        LavaCake::Device& device);
//
// Algorithme :
//   Creer un LavaCake::CommandBuffer en mode one-shot (false = non signale).
//   Appeler cmd.begin().
//
//   Pour chaque niveau i de 1 a mipLevels-1 :
//     1. Transition du niveau (i-1) : eTransferDstOptimal -> eTransferSrcOptimal
//        (vk::ImageMemoryBarrier sur baseMipLevel = i-1, levelCount = 1)
//        srcAccess = eTransferWrite, dstAccess = eTransferRead
//        Soumettre via cmd.pipelineBarrier(eTransfer, eTransfer, ...)
//
//     2. vkCmdBlitImage du niveau (i-1) vers le niveau (i) :
//          srcSubresource.mipLevel = i - 1
//          srcOffsets[1] = { max(width  >> (i-1), 1u),
//                            max(height >> (i-1), 1u), 1 }
//          dstSubresource.mipLevel = i
//          dstOffsets[1] = { max(width  >> i, 1u),
//                            max(height >> i, 1u), 1 }
//          filter = vk::Filter::eLinear
//        (cast cmd en vk::CommandBuffer pour appeler blitImage)
//
//     3. Transition du niveau (i-1) : eTransferSrcOptimal -> eShaderReadOnlyOptimal
//        srcAccess = eTransferRead, dstAccess = eShaderRead
//        Soumettre via cmd.pipelineBarrier(eTransfer, eFragmentShader, ...)
//
//   Apres la boucle :
//     Transition du dernier niveau (mipLevels-1) :
//       eTransferDstOptimal -> eShaderReadOnlyOptimal
//       srcAccess = eTransferWrite, dstAccess = eShaderRead
//       Soumettre via cmd.pipelineBarrier(eTransfer, eFragmentShader, ...)
//
//   Appeler cmd.end(), puis soumettre via device.getAnyQueue().submit(...)
//   et attendre avec device.getAnyQueue().waitIdle().
// ---------------------------------------------------------------------------

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP4 - Mipmaps", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Chargement du modele OBJ (fourni) ----
        std::vector<float>    positions, normals, texcoords;
        std::vector<uint32_t> positionIndices, normalIndices, texcoordIndices;

        if (!loadModel(root + "models/Michelle.obj", root + "models/",
                       positions, normals, texcoords,
                       positionIndices, normalIndices, texcoordIndices)) {
            std::cerr << "Erreur : impossible de charger le modele OBJ." << std::endl;
            return 1;
        }

        // ---- Buffers GPU (fournis) ----
        LavaCake::Buffer posBuffer   (device, positions,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmBuffer   (device, normals,         vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texBuffer   (device, texcoords,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer posIdxBuffer(device, positionIndices, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmIdxBuffer(device, normalIndices,   vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texIdxBuffer(device, texcoordIndices, vk::BufferUsageFlagBits::eStorageBuffer);

        // ---- Chargement de la texture (fourni, repris de exo1) ----
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(
            (root + "models/Ch03_1001_Diffuse.png").c_str(),
            &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            std::cerr << "Erreur : impossible de charger la texture." << std::endl;
            return 1;
        }
        std::vector<uint8_t> pixelData(pixels, pixels + texWidth * texHeight * 4);
        stbi_image_free(pixels);

        // -----------------------------------------------------------------------
        // TODO 2 : Calculer le nombre de niveaux de mipmap, creer l'image et
        //          uploader les donnees du niveau 0.
        //
        //   a) Calcul du nombre de niveaux :
        //     uint32_t mipLevels =
        //         static_cast<uint32_t>(
        //             std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        //
        //   b) Creer l'image SANS donnees (constructeur sans pixels) avec les
        //      usages eSampled | eTransferDst | eTransferSrc et mipLevels niveaux :
        //     LavaCake::Image diffuseTexture(device,
        //         static_cast<uint32_t>(texWidth),
        //         static_cast<uint32_t>(texHeight),
        //         1,
        //         vk::Format::eR8G8B8A8Srgb,
        //         vk::ImageUsageFlagBits::eSampled |
        //         vk::ImageUsageFlagBits::eTransferDst |
        //         vk::ImageUsageFlagBits::eTransferSrc,
        //         vk::AllocationCreateFlagBits::eCreateDedicatedMemory,
        //         mipLevels);
        //
        //   c) Uploader les pixels dans le niveau 0 via un staging buffer :
        //     - Creer un LavaCake::Buffer de staging avec les donnees pixelData
        //       et les flags eTransferSrc + eCreateHostAccessSequentialWrite
        //     - Creer un LavaCake::CommandBuffer uploadCmd(device.getDevice(),
        //         device.getCommandPool(), false) et appeler uploadCmd.begin()
        //     - Transition de TOUS les niveaux : eUndefined -> eTransferDstOptimal
        //       (vk::ImageMemoryBarrier avec baseMipLevel=0, levelCount=mipLevels)
        //       srcAccess = eNone, dstAccess = eTransferWrite
        //       Soumettre via uploadCmd.pipelineBarrier(eTopOfPipe, eTransfer, ...)
        //     - Copier les pixels vers le niveau 0 avec vkCmdCopyBufferToImage :
        //       region.imageSubresource.mipLevel = 0
        //       region.imageExtent = { texWidth, texHeight, 1 }
        //       layout = eTransferDstOptimal
        //     - uploadCmd.end(), soumettre et attendre (getAnyQueue + waitIdle)
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 3 : Appeler generateMipmaps apres avoir uploade le niveau 0.
        //
        //   generateMipmaps((vk::Image)diffuseTexture,
        //                   static_cast<uint32_t>(texWidth),
        //                   static_cast<uint32_t>(texHeight),
        //                   mipLevels, device);
        // -----------------------------------------------------------------------

        // LavaCake::ImageView diffuseView(diffuseTexture);
        // LavaCake::Sampler   diffuseSampler(device);

        // ---- Descriptor set layout (fourni, identique a exo1) ----
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

        // ---- viewProj UBO (fourni) ----
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        projection[1][1] *= -1.0f;

        LavaCake::UniformBuffer viewProjUbo(device);
        glm::mat4 initialViewProj = glm::mat4(1.0f);
        viewProjUbo.addVariable("viewProj", initialViewProj);
        viewProjUbo.end();

        // ---- Pool + descriptor set (fournis) ----
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
            // .bindImage(7, diffuseView, diffuseSampler)  // a decommenter apres TODO 1, 2 & 3
            .update();

        // ---- Image de profondeur (fournie) ----
        vk::Extent2D extent = device.getSwapchainExtent();

        LavaCake::Image depthImage(device, extent.width, extent.height, 1,
            vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        LavaCake::ImageView depthView(depthImage,
            vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eDepth);

        // ---- Pipeline (fourni) ----
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

        // ---- Semaphores (fournis) ----
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);
        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }
        uint32_t currentFrame = 0;
        float time = 0.0f;

        uint32_t vertexCount = static_cast<uint32_t>(positionIndices.size());

        // ---- Boucle de rendu avec camera oscillante (fournie) ----
        // La camera se rapproche et s'eloigne cycliquement, mettant en evidence
        // l'aliasing sans mipmaps (scintillement de la texture).
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            time += 0.01f;

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
