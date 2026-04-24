#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <iostream>
#include <vector>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Cet exercice etend exo1 en enchainant deux passes de calcul sur le GPU :
//   Passe 1 : C[i] = A[i] + B[i]   (addition.comp)
//   Passe 2 : D[i] = A[i] * C[i]   (multiply.comp)
// Les deux dispatches sont enregistres dans le MEME command buffer.
// Une barriere memoire est necessaire entre les deux pour garantir
// que les ecritures de C soient visibles avant les lectures dans la passe suivante.

int main() {

    LavaCake::Device device = LavaCake::createHeadlessDevice(0, 1);

    {
        int size = 1000;

        std::vector<float> A, B;
        for (int i = 0; i < size; i++) {
            A.push_back(i);
            B.push_back(i * 2);
        }

        // Buffers GPU A, B (entree) et C (resultat addition) -- solution exo1
        LavaCake::Buffer ABuffer(device, A, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer BBuffer(device, B, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer CBuffer(device, size * sizeof(float),
            vk::BufferUsageFlagBits::eStorageBuffer,
            vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
            vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

        // TODO 1 : Ajouter un buffer DBuffer pour le resultat de la multiplication
        //          (memes parametres que CBuffer)


        // Layout partage par les deux passes (3 bindings : 0, 1, 2) -- solution exo1
        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eCompute)
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eCompute)
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eCompute)
                .build();

        // TODO 2 : Adapter ce pool pour supporter 2 descriptor sets et 4 storage buffers
        //          (la passe addition utilise A,B,C et la passe multiplication utilise A,C,D)
        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(3)
                .setMaxSets(1)
                .build();

        // Descriptor set de la passe addition -- solution exo1
        vk::DescriptorSet additionDescriptorSet = descriptorPool.allocate(descriptorSetLayout);
        LavaCake::DescriptorSetUpdater(device, additionDescriptorSet)
            .bindStorageBuffer(0, ABuffer)
            .bindStorageBuffer(1, BBuffer)
            .bindStorageBuffer(2, CBuffer)
            .update();

        // TODO 3 : Allouer et mettre a jour le descriptor set de la passe multiplication
        //          ABuffer -> binding 0, CBuffer -> binding 1, DBuffer -> binding 2


        // Pipeline d'addition -- solution exo1
        LavaCake::ComputePipeline additionPipeline =
            LavaCake::ComputePipeline::Builder(device)
                .setShaderFromFile(root + "shaders/TP1/addition.comp",
                                   LavaCake::ShadingLanguage::eGLSL)
                .addDescriptorSetLayout(descriptorSetLayout)
                .build();

        // TODO 4 : Creer le pipeline de multiplication a partir de shaders/TP1/multiply.comp


        LavaCake::CommandBuffer cmdBuffer(device, true);
        cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        // Passe 1 : addition -- solution exo1
        additionPipeline.bind(cmdBuffer);
        additionPipeline.bindDescriptorSets(cmdBuffer, {additionDescriptorSet});
        additionPipeline.dispatch(cmdBuffer, size);

        // TODO 5 : Lier le pipeline de multiplication, son descriptor set et dispatcher


        cmdBuffer.end();

        // Soumission -- solution exo1
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = cmdBuffer;
        device.getComputeQueue(0).submit(submitInfo, cmdBuffer.getFence());
        cmdBuffer.markSubmitted();
        cmdBuffer.waitForCompletion();

        // Readback de C -- solution exo1
        void* data = CBuffer.map();
        std::vector<float> C(size);
        memcpy(C.data(), data, size * sizeof(float));
        CBuffer.unmap();

        // TODO 6 : Lire le resultat de DBuffer vers le vecteur D sur le CPU

        std::vector<float> D(size, 0.0f); // Remplacez par le vrai readback GPU

        // --- Verification de C = A + B (ne pas modifier) ---
        std::cout << "Verification de C = A + B...\n";
        bool success = true;
        for (uint32_t i = 0; i < (uint32_t)size; i++) {
            float expected = A[i] + B[i];
            if (std::abs(C[i] - expected) > 0.0001f) {
                std::cerr << "Erreur C[" << i << "] : attendu " << expected
                          << ", obtenu " << C[i] << "\n";
                success = false;
                break;
            }
        }
        if (success)
            std::cout << "Succes ! C est correct.\n";

        // --- Verification de D = A * C = A * (A + B) (ne pas modifier) ---
        std::cout << "Verification de D = A * C...\n";
        success = true;
        for (uint32_t i = 0; i < (uint32_t)size; i++) {
            float expected = A[i] * (A[i] + B[i]);
            if (std::abs(D[i] - expected) > 0.0001f) {
                std::cerr << "Erreur D[" << i << "] : attendu " << expected
                          << ", obtenu " << D[i] << "\n";
                success = false;
                break;
            }
        }
        if (success)
            std::cout << "Succes ! D est correct.\n";

        device.waitForAllCommands();

    } // Les objets GPU sont detruits ici

    device.releaseDevice();
    return 0;
}
