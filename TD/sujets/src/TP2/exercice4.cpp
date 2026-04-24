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

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP2 - RGB Cube (Push Constants)", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Geometrie du cube RGB (identique a l'exercice 3) ----
        std::vector<float> positions = {
            -0.5f, -0.5f,  0.5f,  // 0
             0.5f, -0.5f,  0.5f,  // 1
             0.5f,  0.5f,  0.5f,  // 2
            -0.5f,  0.5f,  0.5f,  // 3
            -0.5f, -0.5f, -0.5f,  // 4
             0.5f, -0.5f, -0.5f,  // 5
             0.5f,  0.5f, -0.5f,  // 6
            -0.5f,  0.5f, -0.5f,  // 7
        };
        std::vector<float> colors = {
            0.0f, 0.0f, 1.0f,  // 0 bleu
            1.0f, 0.0f, 1.0f,  // 1 magenta
            1.0f, 1.0f, 1.0f,  // 2 blanc
            0.0f, 1.0f, 1.0f,  // 3 cyan
            0.0f, 0.0f, 0.0f,  // 4 noir
            1.0f, 0.0f, 0.0f,  // 5 rouge
            1.0f, 1.0f, 0.0f,  // 6 jaune
            0.0f, 1.0f, 0.0f,  // 7 vert
        };
        std::vector<uint32_t> indices = {
            0,1,2, 0,2,3,  // face avant
            1,5,6, 1,6,2,  // face droite
            5,4,7, 5,7,6,  // face arriere
            4,0,3, 4,3,7,  // face gauche
            3,2,6, 3,6,7,  // face haute
            4,5,1, 4,1,0,  // face basse
        };

        LavaCake::Buffer posBuffer(device, positions, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer colBuffer(device, colors,    vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer idxBuffer(device, indices,   vk::BufferUsageFlagBits::eStorageBuffer);

        // TODO 1 : Separer la matrice MVP en deux parties :
        //   - viewProj = projection * view  (statique, dans un uniform buffer, binding 3)
        //   - model    = matrice identite pour l'instant (sera animee plus tard)
        //
        //   Creer un LavaCake::UniformBuffer pour viewProj :
        //     LavaCake::UniformBuffer viewProjUbo(device);
        //     viewProjUbo.addVariable("viewProj", viewProj);
        //     viewProjUbo.end();
        //   Le descriptor set layout et le pool restent identiques a l'exercice 3.
        //   L'upload se fait dans la boucle via viewProjUbo.update(cmdBuffer)

        // TODO 2 : Modifier le pipeline pour utiliser cube2.vert
        //   Ajouter un push constant range pour la matrice model (mat4, stage vertex, offset 0)
        //   Methode : .addPushConstantRange(stage, offset, size)

        // TODO 3 : Dans la boucle de rendu, calculer a chaque frame une matrice model animee
        //   Utiliser glm::rotate avec un angle qui s'incremente a chaque frame
        //   Envoyer la matrice model au pipeline via pipeline.pushConstants(...)
        //   avant l'appel a pipeline.draw(...)

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

        // ---- Boucle de rendu (identique a l'exercice 3, a completer) ----
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            time += 0.01f;

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            LavaCake::SwapChainImage& swapchainImage =
                device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

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
            // TODO : envoyer la matrice model via pushConstants avant de dessiner
            pipeline.draw(cmdBuffer, 36);

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
