#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <iostream>
#include <vector>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    // Creation d'un device headless : 0 queue graphique, 1 queue compute
    LavaCake::Device device = LavaCake::createHeadlessDevice(0, 1);

    { // Scope pour la gestion de la duree de vie des objets GPU

        int size = 1000;

        // Initialisation des vecteurs d'entree sur le CPU
        std::vector<float> A, B;
        for (int i = 0; i < size; i++) {
            A.push_back(i);
            B.push_back(i * 2);
        }

        // TODO 1 : Creer les buffers GPU pour A, B (entree) et C (sortie)
        //
        // Pour A et B, un simple StorageBuffer suffit :
        //   LavaCake::Buffer monBuffer(device, monVecteur, vk::BufferUsageFlagBits::eStorageBuffer);
        //
        // Pour C, le buffer doit etre lisible depuis le CPU. Utilisez les flags suivants :
        //   LavaCake::Buffer CBuffer(device, size * sizeof(float),
        //       vk::BufferUsageFlagBits::eStorageBuffer,
        //       vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
        //       vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);


        // TODO 2 : Creer un DescriptorSetLayout avec 3 bindings de type StorageBuffer
        //          (bindings 0, 1, 2) accessibles depuis le stage eCompute
        //
        // Utilisez le builder :
        //   LavaCake::DescriptorSetLayout layout =
        //       LavaCake::DescriptorSetLayout::Builder(device)
        //           .addStorageBuffer(0, vk::ShaderStageFlagBits::eCompute)
        //           .addStorageBuffer(1, vk::ShaderStageFlagBits::eCompute)
        //           .addStorageBuffer(2, vk::ShaderStageFlagBits::eCompute)
        //           .build();


        // TODO 3 : Creer un DescriptorPool pouvant contenir 3 StorageBuffers et 1 set max
        //
        //   LavaCake::DescriptorPool pool =
        //       LavaCake::DescriptorPool::Builder(device)
        //           .addStorageBuffers(3)
        //           .setMaxSets(1)
        //           .build();


        // TODO 4 : Allouer un descriptor set depuis le pool avec le layout cree
        //
        //   vk::DescriptorSet descriptorSet = pool.allocate(layout);


        // TODO 5 : Mettre a jour le descriptor set pour lier les buffers aux bindings
        //          ABuffer -> binding 0, BBuffer -> binding 1, CBuffer -> binding 2
        //
        //   LavaCake::DescriptorSetUpdater(device, descriptorSet)
        //       .bindStorageBuffer(0, ABuffer)
        //       .bindStorageBuffer(1, BBuffer)
        //       .bindStorageBuffer(2, CBuffer)
        //       .update();


        // TODO 6 : Creer un ComputePipeline a partir du shader shaders/TP1/addition.comp
        //
        //   LavaCake::ComputePipeline pipeline =
        //       LavaCake::ComputePipeline::Builder(device)
        //           .setShaderFromFile(root + "shaders/TP1/addition.comp",
        //                             LavaCake::ShadingLanguage::eGLSL)
        //           .addDescriptorSetLayout(layout)
        //           .build();


        // TODO 7 : Creer un CommandBuffer (avec fence = true)
        //
        //   LavaCake::CommandBuffer cmdBuffer(device, true);


        // TODO 8 : Enregistrer les commandes dans le command buffer
        //   - Commencer l'enregistrement : cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
        //   - Lier le pipeline         : pipeline.bind(cmdBuffer)
        //   - Lier le descriptor set   : pipeline.bindDescriptorSets(cmdBuffer, {descriptorSet})
        //   - Dispatcher               : pipeline.dispatch(cmdBuffer, size)
        //   - Terminer                 : cmdBuffer.end()


        // TODO 9 : Soumettre le command buffer a la queue compute et attendre la fin
        //
        //   vk::SubmitInfo submitInfo;
        //   submitInfo.commandBufferCount = 1;
        //   submitInfo.pCommandBuffers    = cmdBuffer;
        //   device.getComputeQueue(0).submit(submitInfo, cmdBuffer.getFence());
        //   cmdBuffer.markSubmitted();
        //   cmdBuffer.waitForCompletion();


        // TODO 10 : Lire le resultat depuis CBuffer vers le vecteur C sur le CPU
        //
        //   void* data = CBuffer.map();
        //   std::vector<float> C(size);
        //   memcpy(C.data(), data, size * sizeof(float));
        //   CBuffer.unmap();

        std::vector<float> C(size, 0.0f); // Remplacez cette ligne par le vrai readback GPU

        // --- Verification (ne pas modifier) ---
        std::cout << "Verification des resultats...\n";
        bool success = true;
        for (uint32_t i = 0; i < (uint32_t)size; i++) {
            float expected = A[i] + B[i];
            if (std::abs(C[i] - expected) > 0.0001f) {
                std::cerr << "Erreur a l'index " << i << " : attendu " << expected
                          << ", obtenu " << C[i] << "\n";
                success = false;
                break;
            }
        }
        if (success) {
            std::cout << "Succes ! Les " << size << " resultats sont corrects.\n";
            std::cout << "C[0]=" << C[0] << " (attendu " << (A[0] + B[0]) << ")\n";
            std::cout << "C[" << size - 1 << "]=" << C[size - 1]
                      << " (attendu " << (A[size - 1] + B[size - 1]) << ")\n";
        }

        device.waitForAllCommands();

    } // Les objets GPU sont detruits ici

    device.releaseDevice();
    return 0;
}
