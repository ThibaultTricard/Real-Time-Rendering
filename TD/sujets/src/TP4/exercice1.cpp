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
        // TODO 1 : Charger la texture diffuse avec stb_image et creer les
        //          objets LavaCake correspondants (Image, ImageView, Sampler).
        //
        //   - Charger "models/Ch03_1001_Diffuse.png" avec stbi_load (STBI_rgb_alpha)
        //   - Copier les pixels dans un std::vector<uint8_t>, liberer avec stbi_image_free
        //   - Creer un LavaCake::Image au format eR8G8B8A8Srgb avec usage eSampled
        //     (le constructeur uploade et transite vers eShaderReadOnlyOptimal)
        //   - Creer un LavaCake::ImageView et un LavaCake::Sampler par defaut
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 2 : Creer le DescriptorSetLayout avec 8 bindings :
        //   bindings 0-5 : storage buffers (vertex) — positions, normals, texcoords,
        //                  posIndices, nrmIndices, texIndices
        //   binding 6    : uniform buffer (vertex) — viewProj
        //   binding 7    : combined image sampler (fragment) — texture diffuse
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 3 : Creer l'UniformBuffer viewProj.
        //
        //   Calculez view (camera fixe) et proj (perspective 45 deg, ratio 800/600,
        //   near=0.1, far=100 ; n'oubliez pas de corriger proj[1][1] pour Vulkan).
        //   Creez un LavaCake::UniformBuffer, ajoutez la variable "viewProj", appelez end().
        // -----------------------------------------------------------------------

        // -----------------------------------------------------------------------
        // TODO 4 : Creer le DescriptorPool, allouer et mettre a jour le descriptor set.
        //
        //   Pool : 6 storage buffers, 1 uniform buffer, 1 combined image sampler.
        //   Updater : bindez les 6 buffers (0-5), l'UBO (6) et la texture (7).
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
            //   - Calculer la matrice model animee (rotation autour de Y avec time)
            //   - Uploader l'UBO : viewProjUbo.update(cmdBuffer)
            //   - Preparer le swapchain, creer le DynamicRenderingContext
            //     (color attachment + depth attachment, clear gris 0.1)
            //   - Binder le pipeline et le descriptor set, pousser model en push constant
            //   - Dessiner vertexCount sommets
            //   - Terminer le contexte, preparer le swapchain pour la presentation,
            //     terminer le command buffer
            // -------------------------------------------------------------------

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
