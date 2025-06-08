#pragma once

#include <fstream>
#include <vector>

#include "objects.h"

namespace MyVK {
    std::vector<unsigned char> LoadFontAtlas(const char* font_path);

    std::vector<Vertex> TextToQuad(const char* text);
}
