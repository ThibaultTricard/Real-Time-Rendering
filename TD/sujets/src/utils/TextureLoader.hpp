#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

struct TextureData {
    std::vector<uint8_t> pixels;
    int width  = 0;
    int height = 0;
};

inline TextureData loadTexture(const std::string& path) {
    int w, h, ch;
    stbi_uc* px = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (!px) {
        std::cerr << "Erreur texture : " << path << std::endl;
        std::exit(1);
    }
    TextureData tex;
    tex.pixels = std::vector<uint8_t>(px, px + w * h * 4);
    tex.width  = w;
    tex.height = h;
    stbi_image_free(px);
    return tex;
}
