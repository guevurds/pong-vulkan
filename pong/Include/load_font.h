#pragma once

#include <fstream>
#include <vector>

#include "stb_truetype.h"

#include "objects.h"
namespace MyVK {
    class FontText {
        public:
            FontText(const char* font_path); 

            std::vector<unsigned char> LoadFontAtlas();
            std::vector<Vertex> TextToQuad(const char* text, float posx, float posy);
        private:
            unsigned char m_ttf_buffer[1<<20]; // 1MB 
            unsigned char m_bitmap[512*512]; // tamanho do atlas
            stbtt_bakedchar m_cdata[96]; // ASCII 32..126
    };

}

extern MyVK::FontText font_roboto;