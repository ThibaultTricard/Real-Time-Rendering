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
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP3 - Normales par sommet", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Chargement du modele OBJ (fourni) ----
        std::vector<float>    positions;
        std::vector<uint32_t> positionIndices;

        if (!loadModel(root + "models/LTE.obj", root + "models/",
                       positions, positionIndices)) {
            std::cerr << "Erreur : impossible de charger le modele." << std::endl;
            return 1;
        }

        // TODO 1 : Calculer les normales par sommet par accumulation.
        //
        // Contrairement a l'exercice 1 (une normale par triangle), ici on calcule
        // une normale par sommet unique en accumulant les normales de toutes les faces
        // qui partagent ce sommet, puis en normalisant.
        //
        // Etape A : Creer un vecteur de normales de taille positions.size()/3
        //           (un vec3 par sommet unique) initialise a zero :
        //   std::vector<glm::vec3> perVertexNormals(positions.size() / 3);
        //
        // Etape B : Pour chaque triangle t, calculer la normale de face n
        //           (produit croix, comme en exercice 1) et l'ajouter aux trois sommets :
        //   perVertexNormals[i0] += n;
        //   perVertexNormals[i1] += n;
        //   perVertexNormals[i2] += n;
        //
        // Etape C : Normaliser toutes les normales accumulees :
        //   for (auto& n : perVertexNormals) { n = glm::normalize(n); }

        // TODO 2 : Creer les trois storage buffers GPU
        //   (identique a l'exercice 1 mais avec perVertexNormals au lieu de perFaceNormals)

        // TODO 3-5 : Meme structure qu'a l'exercice 1 pour l'UBO, le descriptor set,
        //            le depth buffer et le pipeline.
        //   Shader vertex : shaders/TP3/perVertexNormal.vert  (au lieu de perFaceNormal.vert)
        //   Shader fragment : shaders/TP3/obj.frag

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

            // TODO : meme boucle de rendu qu'en exercice 1

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
