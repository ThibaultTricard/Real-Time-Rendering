#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <iostream>
#include <vector>
#include <cmath>

std::string root = PROJECT_ROOT;

void plotSignal(std::vector<float> V){
    const int W = 80, H = 15;
    float vmin = *std::min_element(V.begin(), V.end());
    float vmax = *std::max_element(V.begin(), V.end());
    if (vmax == vmin) vmax = vmin + 1.0f;
    std::cout << "\n--- Signal (ASCII art) ---\n";
    for (int row = H - 1; row >= 0; row--) {
        std::cout << ' ';
        for (int col = 0; col < W; col++) {
            int idx = col;
            int valueRow = (int)std::round((V[idx] - vmin) / (vmax - vmin) * (H - 1));
            if(row < valueRow){
                std::cout << '|';
            }else if(row == valueRow){
                std::cout << '*';
            }else {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
    std::cout << '+';
    for (int col = 0; col < W; col++) std::cout << '-';
}

using namespace LavaCake;

// Appliquer 3 fois le filtre boite sur le signal A (avec le shader boxfilter.comp) :
//
//   Passe 1 : B[i] = (A[i-1] + A[i] + A[i+1]) / 3
//   Passe 2 : C[i] = (B[i-1] + B[i] + B[i+1]) / 3
//   Passe 3 : D[i] = (C[i-1] + C[i] + C[i+1]) / 3
//
// Le meme pipeline est reutilise pour les 3 passes avec des descriptor sets differents.

int main() {

    LavaCake::Device device = LavaCake::createHeadlessDevice(0, 1);

    {
        int size = 1000;

        // Signal d'entree : creneau
        std::vector<float> A(size);
        for (int i = 0; i < size; i++)
            A[i] = (i / 8) % 2 == 0 ? 1.0f : 0.0f;

        // TODO 1 : Creer les 4 buffers GPU : ABuffer (depuis A), BBuffer, CBuffer, DBuffer
        //   ABuffer peut etre un buffer classique (lecture seule depuis le GPU).
        //   BBuffer, CBuffer et DBuffer doivent etre accessibles depuis le CPU pour
        //   pouvoir lire le resultat. Utilisez les flags :
        //     vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
        //     vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite


        // TODO 2 : Creer un DescriptorSetLayout avec 2 bindings storage buffer (binding 0 et 1)
        //          et un DescriptorPool adapte pour 3 descriptor sets


        // TODO 3 : Allouer et lier les 3 descriptor sets (ping-pong entre buffers)
        //          Set 0 : ABuffer -> BBuffer
        //          Set 1 : BBuffer -> CBuffer
        //          Set 2 : CBuffer -> DBuffer


        // TODO 4 : Creer le ComputePipeline depuis shaders/TP1/boxfilter.comp


        // TODO 5 : Enregistrer les 3 dispatches dans un CommandBuffer et soumettre
        //   N'oubliez pas les barrieres memoire entre chaque passe :
        //     cmdBuffer.pipelineBarrier(eComputeShader, eComputeShader, {}, memoryBarrier, {}, {})


        // TODO 6 : Lire le resultat de DBuffer vers le vecteur D via DBuffer.map()

        std::vector<float> D(size, 0.0f); // Remplacez par le vrai readback GPU

        plotSignal(D);

        device.waitForAllCommands();

    } // Les objets GPU sont detruits ici

    device.releaseDevice();
    return 0;
}
