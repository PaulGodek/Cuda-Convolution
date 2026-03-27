#include "student_functions.h"

#include <ATen/ops/amax.h>
#include <ATen/ops/amin.h>
#include <ATen/ops/zeros.h>
#include <ATen/ops/zeros_like.h>

torch::Tensor threshold_cpu(torch::Tensor input, float threshold) {
    // Exemple guide :
    // cette fonction montre d'abord le minimum vital pour manipuler un
    // `torch::Tensor` en C++ avant de passer a la normalisation.
    torch::Tensor inputFloat = input.scalar_type() == torch::kFloat32 ? input : input.to(torch::kFloat32);
    torch::Tensor output = torch::zeros(
        inputFloat.sizes(),
        torch::TensorOptions().dtype(torch::kUInt8).device(inputFloat.device()));

    auto inputAccessor = inputFloat.accessor<float, 2>();
    auto outputAccessor = output.accessor<uint8_t, 2>();

    const int height = static_cast<int>(inputFloat.size(0));
    const int width = static_cast<int>(inputFloat.size(1));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            outputAccessor[y][x] = (inputAccessor[y][x] >= threshold) ? 255 : 0;
        }
    }

    return output;
}

torch::Tensor normalize_cpu(torch::Tensor input) {
    // TODO:
    // 1. convertir / forcer le tenseur en float32 si besoin ;
    // 2. calculer min et max avec l'API torch C++ ;
    // 3. allouer un tenseur de sortie ;
    // 4. normaliser les valeurs sur CPU ;
    // 5. retourner le tenseur resultat.


    return output;
}
