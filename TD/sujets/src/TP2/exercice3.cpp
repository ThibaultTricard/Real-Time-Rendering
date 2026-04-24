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
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP2 - RGB Cube", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Geometrie du cube RGB (fournie) ----
        // 8 sommets aux coins du cube, couleurs style RGB
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

        // TODO 1 : Creer trois storage buffers GPU :
        //   - posBuffer  depuis positions
        //   - colBuffer  depuis colors
        //   - idxBuffer  depuis indices

        // TODO 2 : Calculer la matrice MVP avec GLM
        //   model      = identite
        //   view       = glm::lookAt(vec3(2,2,2), origine, up)
        //   projection = glm::perspective(45 degres, ratio 800/600, 0.1, 100)
        //   Attention : corriger l'axe Y Vulkan (projection[1][1] *= -1)
        //   mvp        = projection * view * model
        //
        //   Creer un LavaCake::UniformBuffer et y enregistrer la matrice mvp :
        //     LavaCake::UniformBuffer mvpUbo(device);
        //     mvpUbo.addVariable("mvp", mvp);
        //     mvpUbo.end();
        //   L'upload GPU se fait plus tard dans la boucle via mvpUbo.update(cmdBuffer)

        // TODO 3 : Creer un DescriptorSetLayout avec :
        //   - binding 0 : storage buffer (vertex)
        //   - binding 1 : storage buffer (vertex)
        //   - binding 2 : storage buffer (vertex)
        //   - binding 3 : uniform buffer (vertex)
        //   Creer un DescriptorPool adapte (3 storage + 1 uniform, 1 set max)
        //   Allouer et mettre a jour le descriptor set

        // TODO 4 : Creer une image de profondeur et sa vue (depth buffer)
        //   LavaCake::Image depthImage(device, width, height, 1, eD32Sfloat, eDepthStencilAttachment)
        //   LavaCake::ImageView depthView(depthImage, e2D, eDepth)

        // TODO 5 : Creer le GraphicsPipeline avec :
        //   - shaders/TP2/cube.vert  et  shaders/TP2/cube.frag
        //   - le descriptor set layout
        //   - le format couleur du swapchain
        //   - le format de profondeur eD32Sfloat
        //   - depth test active (.setDepthTest(true), depth write est actif par defaut)
        //   - back-face culling active

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

        // ---- Boucle de rendu ----
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            LavaCake::SwapChainImage& swapchainImage =
                device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            // TODO 6 : Uploader l'UBO puis effectuer le rendu :
            //   mvpUbo.update(cmdBuffer)
            //   Creer le DynamicRenderingContext avec :
            //   - la zone de rendu (swapchain extent)
            //   - la color attachment (swapchainImage) avec un fond gris fonce
            //   - la depth attachment (depthView) avec clearDepth = 1.0
            //   Appeler setDefaultViewportScissor
            //   Binder le pipeline et le descriptor set
            //   Dessiner 36 sommets (6 faces x 2 triangles x 3 sommets)

            // TODO (fin du TODO 6) : terminer le contexte de rendu
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
