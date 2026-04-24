#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <iostream>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    // Initialisation de la fenetre
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP2 - Triangle", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // TODO 1 : Creer le GraphicsPipeline avec triangle.vert et triangle.frag
        // LavaCake::GraphicsPipeline graphicPipeline  = LavaCake::GraphicsPipeline::Builder(device)
        //                     // Ajoute un vertex shader à partir d'une source
        //                    .addShaderFromFile(root + "shaders/TP2/triangle.vert",vk::ShaderStageFlagBits::eVertex,LavaCake::ShadingLanguage::eGLSL)  
        //                     // Ajoute un fragment shader à partir d'une source
        //                    .addShaderFromFile(root + "shaders/TP2/triangle.frag",vk::ShaderStageFlagBits::eFragment,LavaCake::ShadingLanguage::eGLSL) 
        //                     // Définit le format de l'image dans la quelle on va ecrire
        //                    .addColorAttachmentFormat(device.getSwapchainFormat()) 
        //                     // Affiche toutes les faces, y compris les back faces
        //                    .setCullMode(vk::CullModeFlagBits::eNone) 
        //                    .build();

        // Creation des semaphores (un par image de la swapchain)
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);
        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }
        uint32_t currentFrame = 0;

        // Boucle de rendu
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // TODO 2 : Acquerir l'image de la swapchain, la transitionner vers le layout
            //          d'attachement, et demarrer le DynamicRenderingContext avec une couleur de fond.
            //          Appellez ensuite renderingContext.setDefaultViewportScissor(cmdBuffer).
            //
            //   LavaCake::SwapChainImage& swapchainImage =
            //       device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);
            //
            //   swapchainImage.prepareForAttachementBarrier(cmdBuffer);
            //
            //   LavaCake::DynamicRenderingContext renderingContext =
            //       LavaCake::DynamicRenderingContext::Builder()
            //           .setRenderArea(device.getSwapchainExtent())
            //           .addColorAttachment(swapchainImage, vk::ClearColorValue(...))
            //           .begin(cmdBuffer);
            //
            //   renderingContext.setDefaultViewportScissor(cmdBuffer);


            // TODO 3 : Lier le pipeline et effectuer le draw call (3 sommets)


            // Fin du rendu et transition vers le layout de presentation
            // (a decommenter une fois les TODO 2 et 3 completes)
            // renderingContext.end(cmdBuffer);
            // swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Soumission
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

            // (a decommenter une fois les TODO 2 et 3 completes)
            // device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});

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
