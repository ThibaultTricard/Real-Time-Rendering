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

#include "utils/ObjLoader.hpp"
#include "utils/TextureLoader.hpp"

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "TP5 - Normal Mapping", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig, 1);

    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        // ---- Chargement du modele (fourni) ----
        std::vector<float>    positions, normals, texcoords;
        std::vector<uint32_t> positionIndices, normalIndices, texcoordIndices;

        if (!loadModel(root + "models/sphere_low_poly.obj", root + "models/",
                       positions, normals, texcoords,
                       positionIndices, normalIndices, texcoordIndices)) {
            std::cerr << "Erreur : impossible de charger le modele." << std::endl;
            return 1;
        }

        // -----------------------------------------------------------------------
        // TODO : Calculer les tangentes avec déduplication.
        //
        // La continuité d'une tangente est brisée aux coutures UV (ti différent)
        // ET aux arêtes vives (ni différent). L'identité d'un sommet est donc
        // le triplet (pi, ni, ti).
        //
        // Structures nécessaires :
        //   - std::map<std::tuple<uint32_t,uint32_t,uint32_t>, uint32_t> tangentIndexMap
        //       associe chaque clé (pi, ni, ti) à un indice dans tangentAccum.
        //   - std::vector<glm::vec3> tangentAccum
        //       accumule les tangentes brutes (non normalisées) par entrée unique.
        //   - std::vector<uint32_t> tangentIndices (taille = positionIndices.size())
        //       indice dans tangentAccum pour chaque sommet de chaque triangle.
        //
        // Étapes :
        //   1. Boucler sur les triangles (numTriangles = positionIndices.size() / 3).
        //      Pour chaque triangle f :
        //        - Lire les indices pi0/pi1/pi2, ni0/ni1/ni2, ti0/ti1/ti2.
        //        - Construire p0, p1, p2 et uv0, uv1, uv2.
        //        - Calculer les arêtes : e1 = p1-p0, e2 = p2-p0
        //                                d1 = uv1-uv0, d2 = uv2-uv0
        //        - Calculer la tangente brute T :
        //            float det = d1.x*d2.y - d2.x*d1.y;
        //            glm::vec3 T(0.0f);
        //            if (std::abs(det) > 1e-6f) T = (d2.y*e1 - d1.y*e2) / det;
        //        - Pour chacun des 3 sommets v du triangle :
        //            * Construire la clé key = (pi_v, ni_v, ti_v).
        //            * Insérer la clé dans tangentIndexMap avec tangentIndexMap.emplace().
        //            * Si la clé est nouvelle : ajouter T dans tangentAccum.
        //            * Sinon : accumuler T sur l'entrée existante de tangentAccum.
        //            * Écrire l'indice obtenu dans tangentIndices[3*f+v].
        //
        //   2. Normaliser : créer std::vector<float> tangents à partir de tangentAccum
        //      en normalisant chaque vecteur et en empilant x, y, z.
        // -----------------------------------------------------------------------

        // ---- Buffers GPU (fournis, a activer apres le TODO) ----
        LavaCake::Buffer posBuffer   (device, positions,       vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmBuffer   (device, normals,         vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texBuffer   (device, texcoords,       vk::BufferUsageFlagBits::eStorageBuffer);
        // LavaCake::Buffer tanBuffer(device, tangents,         vk::BufferUsageFlagBits::eStorageBuffer); // a decommenter
        LavaCake::Buffer posIdxBuffer(device, positionIndices, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer nrmIdxBuffer(device, normalIndices,   vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer texIdxBuffer(device, texcoordIndices, vk::BufferUsageFlagBits::eStorageBuffer);
        //LavaCake::Buffer tanIdxBuffer(device, tangentIndices,  vk::BufferUsageFlagBits::eStorageBuffer);
        
        // ---- Textures (fournies) ----
        TextureData normalData = loadTexture(root + "models/Sphere_normal_map.png");
        LavaCake::Image     normalTex(device, normalData.pixels, normalData.width, normalData.height, 1,
                                      vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled);
        LavaCake::ImageView normalView(normalTex);
        LavaCake::Sampler   normalSampler(device, vk::SamplerMipmapMode::eLinear, 0.0f, 0.0f);

        // ---- UBO : viewProj (fourni) ----
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        projection[1][1] *= -1.0f;

        LavaCake::UniformBuffer viewProjUbo(device);
        viewProjUbo.addVariable("viewProj", glm::mat4(1.0f));
        viewProjUbo.end();

        // ---- Descriptor set layout (fourni) ----
        // Bindings: 0-2 pos/nrm/tex  |  3 tangents  |  4-6 indices
        //           7 viewProj UBO   |  8 diffuse   |  9 normal map
        LavaCake::DescriptorSetLayout layout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(3, vk::ShaderStageFlagBits::eVertex)   // tangents
                .addStorageBuffer(4, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(5, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(6, vk::ShaderStageFlagBits::eVertex)
                .addStorageBuffer(7, vk::ShaderStageFlagBits::eVertex)   // tangents
                .addUniformBuffer(8, vk::ShaderStageFlagBits::eVertex)
                .addCombinedImageSampler(9, vk::ShaderStageFlagBits::eFragment)
                .build();

        LavaCake::DescriptorPool pool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(7)
                .addUniformBuffers(1)
                .addCombinedImageSamplers(2)
                .setMaxSets(1)
                .build();

        vk::DescriptorSet descriptorSet = pool.allocate(layout);
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, posBuffer)
            .bindStorageBuffer(1, nrmBuffer)
            .bindStorageBuffer(2, texBuffer)
            // TODO : .bindStorageBuffer(3, tanBuffer)
            .bindStorageBuffer(4, posIdxBuffer)
            .bindStorageBuffer(5, nrmIdxBuffer)
            .bindStorageBuffer(6, texIdxBuffer)
            // TODO :  .bindStorageBuffer(7, tanIdxBuffer)
            .bindUniformBuffer(8, viewProjUbo)
            .bindImage(9, normalView,  normalSampler)
            .update();

        // ---- Depth image (fournie) ----
        vk::Extent2D extent = device.getSwapchainExtent();
        LavaCake::Image depthImage(device, extent.width, extent.height, 1,
            vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        LavaCake::ImageView depthView(depthImage,
            vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth);

        // ---- Pipeline (fourni) ----
        LavaCake::GraphicsPipeline pipeline =
            LavaCake::GraphicsPipeline::Builder(device)
                .addShaderFromFile(root + "shaders/TP5/nrm.vert",
                                   vk::ShaderStageFlagBits::eVertex,
                                   LavaCake::ShadingLanguage::eGLSL)
                .addShaderFromFile(root + "shaders/TP5/nrm.frag",
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

        glm::vec3 camPos = glm::vec3(1.5f, 1.5f, 1.5f);
        uint32_t vertexCount = static_cast<uint32_t>(positionIndices.size());

        // ---- Boucle de rendu (fournie) ----
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            time += 0.01f;

            glm::mat4 view = glm::lookAt(camPos,
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
