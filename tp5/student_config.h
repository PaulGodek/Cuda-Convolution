#ifndef STUDENT_CONFIG_H
#define STUDENT_CONFIG_H

// Parametres par defaut du tp5.
// Le programme traite un seul masque GT par execution.
struct tp5Config {
    // Chemin du masque charge si aucun argument n'est fourni.
    const char* defaultGroundTruthPath = "../data/GT/pointCloud_data_2026-02-19_12-03-32_rgb_semantic_mask.png";
    // Taille minimale d'une composante pour qu'elle soit conservee.
    int defaultMinComponentSize = 2;
    // Marge ajoutee autour d'une composante avant le recadrage.
    int defaultCropPadding = 16;
    // Longueur maximale d'une petite branche terminale a supprimer.
    int defaultBranchLengthThreshold = 20;
    // Pas cible du reechantillonnage de la ligne principale.
    double defaultResampleSpacing = 1.0;
    // Nombre de passes de lissage sur la ligne principale.
    int defaultSmoothingIterations = 2;
};

inline constexpr tp5Config kStudentConfig{};

#endif // STUDENT_CONFIG_H
