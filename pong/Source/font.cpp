#include "load_font.h"
#include "game.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace MyVK {

  FontText::FontText(const char* font_path) {
    std::ifstream file(font_path, std::ios::binary);
    file.read((char*)m_ttf_buffer, sizeof(m_ttf_buffer));

    //gera um atlas para caracteres 32...126 (imprimiveis)
    int res = stbtt_BakeFontBitmap(
      m_ttf_buffer, 0, 32.0, // fonte, offset, tamanho
      m_bitmap, 512, 512, // destino, largura, altura
      32, 96, // char range
      m_cdata // saida com os dados
    );

     if (res <= 0) {
      printf("Erro: stbtt_BakeFontBitmap falhou (return=%d)\n", res);
      exit(1);
    }
  }

  std::vector<unsigned char> FontText::LoadFontAtlas() {
    std::vector<unsigned char> rgba(512 * 512 * 4);
    for(int i=0; i<512 * 512; ++i) {
      if (m_bitmap[i] > 0) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = m_bitmap[i];
      } else {
        rgba[i * 4 + 0] = 0;
        rgba[i * 4 + 1] = 0;
        rgba[i * 4 + 2] = 0;
        rgba[i * 4 + 3] = 0;
      }
    }
    return rgba;
  }

  std::vector<Vertex> FontText::TextToQuad(const char* text) {

    std::vector<Vertex> vertices;
    float x = 640;
    float y = 360; // y fixo para linha do texto

    auto NormalizeX = [](float px) {
      return (px / WINDOW_WIDTH) * 2.0f - 1.0f;
    };

    auto NormalizeY = [](float py) {
      return (1.0f - (py / WINDOW_HEIGHT) * 2.0f); // antes eu invertia aqui mas vou inverter no render pra deixar as posições iguais ao do plano cartesiano
    };

    for(int i =0; text[i]; i++) {
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(m_cdata, 512, 512, text[i] - 32, &x, &y, &q, 1); // 512 = tamanho da textura

      float normalized_x0 = NormalizeX(q.x0);
      float normalized_x1 = NormalizeX(q.x1);

      float normalized_y0 = NormalizeY(q.y0);
      float normalized_y1 = NormalizeY(q.y1);

      vertices.push_back(Vertex({normalized_x0, normalized_y0, 0.0f}, {q.s0, q.t0}));
      vertices.push_back(Vertex({normalized_x1, normalized_y0, 0.0f}, {q.s1, q.t0}));
      vertices.push_back(Vertex({normalized_x1, normalized_y1, 0.0f}, {q.s1, q.t1}));

      vertices.push_back(Vertex({normalized_x0, normalized_y0, 0.0f}, {q.s0, q.t0}));
      vertices.push_back(Vertex({normalized_x1, normalized_y1, 0.0f}, {q.s1, q.t1}));
      vertices.push_back(Vertex({normalized_x0, normalized_y1, 0.0f}, {q.s0, q.t1}));
     }

    return vertices;
  }

  
}




