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

// Afficher le modele LTE.obj eclaire avec les normales lues directement
// depuis le fichier OBJ. Les normales OBJ ont des indices independants
// des indices de position : un meme sommet peut avoir des normales
// differentes selon la face a laquelle il appartient.

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP3 - Normales OBJ", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Chargement du modele OBJ (fourni) ----
        // Cette version de loadModel charge les positions ET les normales du fichier OBJ,
        // avec des indices separes pour chacun.
        std::vector<float>    positions;
        std::vector<float>    normals;
        std::vector<uint32_t> positionIndices;
        std::vector<uint32_t> normalIndices;

        if (!loadModel(root + "models/LTE.obj", root + "models/",
                       positions, normals, positionIndices, normalIndices)) {
            std::cerr << "Erreur : impossible de charger le modele." << std::endl;
            return 1;
        }

        // TODO 1 : Creer quatre storage buffers GPU :
        //   - posBuffer    depuis positions
        //   - nrmBuffer    depuis normals
        //   - posIdxBuffer depuis positionIndices
        //   - nrmIdxBuffer depuis normalIndices
        //
        // Les indices de normales sont separes des indices de positions :
        // pour le sommet i, la position est positions[positionIndices[i]]
        // et la normale est  normals[normalIndices[i]].

        // TODO 2 : Creer la matrice MVP et un UniformBuffer
        //   (identique aux exercices precedents)

        // TODO 3 : Creer le DescriptorSetLayout (4 storage + 1 uniform),
        //          le pool, allouer et mettre a jour le descriptor set
        //   binding 0 : posBuffer
        //   binding 1 : nrmBuffer
        //   binding 2 : posIdxBuffer
        //   binding 3 : nrmIdxBuffer
        //   binding 4 : mvpUbo

        // TODO 4 : Creer l'image de profondeur et le GraphicsPipeline
        //   Shaders : shaders/TP3/objNormal.vert et shaders/TP3/obj.frag
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
