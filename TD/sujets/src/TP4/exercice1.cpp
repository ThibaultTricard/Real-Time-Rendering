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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "utils/ObjLoader.hpp"

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP4 - Texture", nullptr, nullptr);

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

        // -----------------------------------------------------------------------
        // TODO 1 : Charger la texture diffuse avec stb_image
        //          et creer les objets LavaCake correspondants.
        //
        //   Etape A — Charger le fichier image :
        //     int texWidth, texHeight, texChannels;
        //     stbi_uc* pixels = stbi_load(
        //         (root + "models/Ch03_1001_Diffuse.png").c_str(),
        //         &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        //     if (!pixels) {
        //         std::cerr << "Erreur : impossible de charger la texture." << std::endl;
        //         return 1;
        //     }
        //
        //   Etape B — Copier les pixels dans un std::vector<uint8_t> :
        //     std::vector<uint8_t> pixelData(pixels,
        //         pixels + texWidth * texHeight * 4);
        //     stbi_image_free(pixels);
        //
        //   Etape C — Creer l'Image LavaCake :
        //     LavaCake::Image diffuseTexture(device, pixelData,
        //         texWidth, texHeight, 1,
        //         vk::Format::eR8G8B8A8Srgb,
        //         vk::ImageUsageFlagBits::eSampled);
        //     // Le constructeur uploade automatiquement les pixels sur le GPU
        //     // et effectue la transition vers eShaderReadOnlyOptimal.
        //
        //   Etape D — Creer l'ImageView et le Sampler :
        //     LavaCake::ImageView diffuseView(diffuseTexture);
        //     LavaCake::Sampler   diffuseSampler(device);
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 2 : Creer le DescriptorSetLayout.
        //
        //   LavaCake::DescriptorSetLayout layout =
        //       LavaCake::DescriptorSetLayout::Builder(device)
        //           .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)   // positions
        //           .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)   // normals
        //           .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)   // texcoords
        //           .addStorageBuffer(3, vk::ShaderStageFlagBits::eVertex)   // posIndices
        //           .addStorageBuffer(4, vk::ShaderStageFlagBits::eVertex)   // nrmIndices
        //           .addStorageBuffer(5, vk::ShaderStageFlagBits::eVertex)   // texIndices
        //           .addUniformBuffer(6, vk::ShaderStageFlagBits::eVertex)   // viewProj
        //           .addCombinedImageSampler(7, vk::ShaderStageFlagBits::eFragment) // texture diffuse
        //           .build();
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 3 : Creer l'UniformBuffer pour viewProj (view * projection).
        //
        //   glm::mat4 view = glm::lookAt(
        //       glm::vec3(0.0f, 2.0f, 4.0f),
        //       glm::vec3(0.0f, 0.0f, 0.0f),
        //       glm::vec3(0.0f, 1.0f, 0.0f));
        //   glm::mat4 proj = glm::perspective(
        //       glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        //   proj[1][1] *= -1.0f;  // correction axe Y Vulkan
        //   glm::mat4 viewProj = proj * view;
        //
        //   LavaCake::UniformBuffer viewProjUbo(device);
        //   viewProjUbo.addVariable("viewProj", viewProj);
        //   viewProjUbo.end();
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 4 : Creer le DescriptorPool, allouer et mettre a jour le descriptor set.
        //
        //   LavaCake::DescriptorPool pool =
        //       LavaCake::DescriptorPool::Builder(device.getDevice())
        //           .addStorageBuffers(6)           // 6 storage buffers (bindings 0-5)
        //           .addUniformBuffers(1)           // 1 uniform buffer  (binding 6)
        //           .addCombinedImageSamplers(1)    // 1 sampler         (binding 7)
        //           .setMaxSets(1)
        //           .build();
        //
        //   vk::DescriptorSet descriptorSet = pool.allocate(layout);
        //
        //   LavaCake::DescriptorSetUpdater(device, descriptorSet)
        //       .bindStorageBuffer(0, posBuffer)
        //       .bindStorageBuffer(1, nrmBuffer)
        //       .bindStorageBuffer(2, texBuffer)
        //       .bindStorageBuffer(3, posIdxBuffer)
        //       .bindStorageBuffer(4, nrmIdxBuffer)
        //       .bindStorageBuffer(5, texIdxBuffer)
        //       .bindUniformBuffer(6, viewProjUbo)
        //       .bindImage(7, diffuseView, diffuseSampler)
        //       .update();
        // -----------------------------------------------------------------------

        // ---- Image de profondeur (fournie) ----
        vk::Extent2D extent = device.getSwapchainExtent();

        LavaCake::Image depthImage(device, extent.width, extent.height, 1,
            vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        LavaCake::ImageView depthView(depthImage,
            vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eDepth);

        // ---- Pipeline (fourni) ----
        // Utilise les shaders shaders/TP4/obj_tex.vert et shaders/TP4/obj_tex.frag
        // Le pipeline attend : binding 7 = combined image sampler, push constant = mat4 model
        LavaCake::GraphicsPipeline pipeline =
            LavaCake::GraphicsPipeline::Builder(device)
                .addShaderFromFile(root + "shaders/TP4/obj_tex.vert",
                                   vk::ShaderStageFlagBits::eVertex,
                                   LavaCake::ShadingLanguage::eGLSL)
                .addShaderFromFile(root + "shaders/TP4/obj_tex.frag",
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

        // ---- Boucle de rendu ----
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            time += 0.01f;

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            LavaCake::SwapChainImage& swapchainImage =
                device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // -------------------------------------------------------------------
            // TODO 5 : Completer la boucle de rendu.
            //
            //   Etape A — Calculer la matrice model animee :
            //     glm::mat4 model = glm::rotate(glm::mat4(1.0f), time,
            //                                   glm::vec3(0.0f, 1.0f, 0.0f));
            //
            //   Etape B — Uploader l'UBO (viewProj ne change pas, mais doit etre
            //              envoye au GPU au moins une fois) :
            //     viewProjUbo.update(cmdBuffer);
            //
            //   Etape C — Preparer le swapchain et creer le contexte de rendu :
            //     swapchainImage.prepareForAttachementBarrier(cmdBuffer);
            //     LavaCake::DynamicRenderingContext renderingContext =
            //         LavaCake::DynamicRenderingContext::Builder()
            //             .setRenderArea(device.getSwapchainExtent())
            //             .addColorAttachment(swapchainImage,
            //                 vk::ClearColorValue(std::array<float,4>{0.1f,0.1f,0.1f,1.0f}))
            //             .setDepthAttachment(depthView, 1.0f)
            //             .begin(cmdBuffer);
            //
            //   Etape D — Binder et dessiner :
            //     renderingContext.setDefaultViewportScissor(cmdBuffer);
            //     pipeline.bind(cmdBuffer);
            //     pipeline.bindDescriptorSets(cmdBuffer, {descriptorSet});
            //     pipeline.pushConstants(cmdBuffer,
            //         vk::ShaderStageFlagBits::eVertex, 0, model);
            //     pipeline.draw(cmdBuffer, vertexCount);
            //
            //   Etape E — Terminer le contexte et le command buffer :
            //     renderingContext.end(cmdBuffer);
            //     swapchainImage.prepareForPresentBarrier(cmdBuffer);
            //     cmdBuffer.end();
            // -------------------------------------------------------------------

            // swapchainImage.prepareForPresentBarrier(cmdBuffer);
            // cmdBuffer.end();

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
