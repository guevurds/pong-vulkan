#include "load_font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


namespace MyVK {
  std::vector<unsigned char> LoadFontAtlas(const char* font_path) {
    unsigned char ttf_buffer[1<<20]; // 1MB 
    unsigned char bitmap[512*512]; // tamanho do atlas

    stbtt_bakedchar cdata[96]; // ASCII 32..126

    std::ifstream file(font_path, std::ios::binary);
    file.read((char*)ttf_buffer, sizeof(ttf_buffer));

    //gera um atlas para caracteres 32...126 (imprimiveis)
    int res = stbtt_BakeFontBitmap(
      ttf_buffer, 0, 32.0, // fonte, offset, tamanho
      bitmap, 512, 512, // destino, largura, altura
      32, 96, // char range
      cdata // saida com os dados
    );

     if (res <= 0) {
      printf("Erro: stbtt_BakeFontBitmap falhou (return=%d)\n", res);
      exit(1);
    }

    stbi_write_png("debug_font_bitmap.png", 512, 512, 1, bitmap, 512);

    std::vector<unsigned char> rgba(512 * 512 * 4);

    for(int i=0; i<512 * 512; ++i) {
      if (bitmap[i] > 0) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = bitmap[i];
      } else {
        rgba[i * 4 + 0] = 0;
        rgba[i * 4 + 1] = 0;
        rgba[i * 4 + 2] = 0;
        rgba[i * 4 + 3] = 0;
      }
    }

    return rgba;
  }

  auto NormalizeX = [](float px) {
    // return px;
    return (px / WINDOW_WIDTH) * 2.0f - 1.0f;
  };

  auto NormalizeY = [](float py) {
    // return py;
    return -(1.0f - (py / WINDOW_HEIGHT) * 2.0f);
  };

  std::vector<Vertex> TextToQuad(const char* text) {
    unsigned char ttf_buffer[1<<20]; // 1MB 
    unsigned char bitmap[512*512]; // tamanho do atlas

    stbtt_bakedchar cdata[96]; // ASCII 32..126

    std::ifstream file("Textures/roboto_font.ttf", std::ios::binary);
    
    file.read((char*)ttf_buffer, sizeof(ttf_buffer));

    //gera um atlas para caracteres 32...126 (imprimiveis)
    int res = stbtt_BakeFontBitmap(
      ttf_buffer, 0, 32.0, // fonte, offset, tamanho
      bitmap, 512, 512, // destino, largura, altura
      32, 96, // char range
      cdata // saida com os dados
    );

    // remover 
    std::vector<unsigned char> rgba(512 * 512 * 4);

    for(int i=0; i<512 * 512; ++i) {
      rgba[i * 4 + 0] = 255;
      rgba[i * 4 + 1] = 255;
      rgba[i * 4 + 2] = 255;
      rgba[i * 4 + 3] = bitmap[i];
    }

    for (int y = 250; y < 260; ++y) {
      for (int x = 250; x < 260; ++x) {
        int idx = (y * 512 + x) * 4;
        unsigned char a = rgba[idx + 3];
        printf("%3d ", a);
      }
      printf("\n");
    }
    //remover

    if (res <= 0) {
      printf("Erro: stbtt_BakeFontBitmap falhou (return=%d)\n", res);
      exit(1);
    }
    //gluint??

    std::vector<Vertex> vertices;
    float x = 20.0f;
    float y = 300.0f; // y fixo para linha do texto

    for(int i =0; text[i]; i++) {
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, 512, 512, text[i] - 32, &x, &y, &q, 1); // 512 = tamanho da textura

      printf("positions\n qx0: %f, qy0: %f\n qx1: %f, qy1: %f\n", NormalizeX(q.x0), NormalizeY(q.y0),NormalizeX(q.x1), NormalizeY(q.y1));

      vertices.push_back(Vertex({NormalizeX(q.x0), NormalizeY(q.y0), 0.0f}, {q.s0, q.t0}));
      vertices.push_back(Vertex({NormalizeX(q.x1), NormalizeY(q.y0), 0.0f}, {q.s1, q.t0}));
      vertices.push_back(Vertex({NormalizeX(q.x1), NormalizeY(q.y1), 0.0f}, {q.s1, q.t1}));

      vertices.push_back(Vertex({NormalizeX(q.x0), NormalizeY(q.y0), 0.0f}, {q.s0, q.t0}));
      vertices.push_back(Vertex({NormalizeX(q.x1), NormalizeY(q.y1), 0.0f}, {q.s1, q.t1}));
      vertices.push_back(Vertex({NormalizeX(q.x0), NormalizeY(q.y1), 0.0f}, {q.s0, q.t1}));
     }

    return vertices;
  }

  
}




