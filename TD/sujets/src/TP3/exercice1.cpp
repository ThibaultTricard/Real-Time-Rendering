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

#include "utils/ObjLoader.hpp"

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP3 - Normales par face", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Chargement du modele OBJ (fourni) ----
        // Seules les positions et les indices de position sont charges.
        // Les normales seront calculees a la main a partir de la geometrie.
        std::vector<float>    positions;
        std::vector<uint32_t> positionIndices;

        if (!loadModel(root + "models/LTE.obj", root + "models/",
                       positions, positionIndices)) {
            std::cerr << "Erreur : impossible de charger le modele." << std::endl;
            return 1;
        }

        // TODO 1 : Calculer les normales par face (une par triangle).
        //
        // Chaque triangle est defini par trois indices consecutifs dans positionIndices.
        // Pour le triangle t (sommets d'indice t, t+1, t+2 dans positionIndices) :
        //   - Recuperer les trois positions v0, v1, v2 depuis le tableau positions
        //     (chaque position occupe 3 floats : positions[i*3], [i*3+1], [i*3+2])
        //   - Calculer la normale : n = normalize(cross(v1 - v0, v2 - v1))
        //   - Stocker n dans perFaceNormals[t/3]
        //
        //   std::vector<glm::vec3> perFaceNormals(positionIndices.size() / 3);
        //   for (uint32_t t = 0; t < positionIndices.size(); t += 3) { ... }

        // TODO 2 : Creer trois storage buffers GPU :
        //   - posBuffer    depuis positions
        //   - nrmBuffer    depuis perFaceNormals
        //   - posIdxBuffer depuis positionIndices

        // TODO 3 : Creer la matrice MVP et un UniformBuffer
        //   view       = glm::lookAt(vec3(0.5, 0.5, 0.5), origine, up)
        //   projection = glm::perspective(45 degres, 800/600, 0.1, 100) + correction Y
        //   mvp        = projection * view * identite
        //
        //   LavaCake::UniformBuffer mvpUbo(device);
        //   mvpUbo.addVariable("mvp", mvp);
        //   mvpUbo.end();

        // TODO 4 : Creer le DescriptorSetLayout (3 storage + 1 uniform), le pool,
        //          allouer et mettre a jour le descriptor set

        // TODO 5 : Creer l'image de profondeur et le GraphicsPipeline
        //   Shaders : shaders/TP3/perFaceNormal.vert et shaders/TP3/obj.frag
        //   Activer depth test et back-face culling

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

        // ---- Boucle de rendu (fournie) ----
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            LavaCake::SwapChainImage& swapchainImage =
                device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // TODO : mettre a jour la MVP avec glm::rotate(time++), uploader l'UBO,
            //        configurer le rendu, binder pipeline + descriptor set, dessiner vertexCount

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
