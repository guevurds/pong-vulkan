#include "objects.h"
// #include <cmath> 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Bola : public Scene::Object {
 using Object::Object;

 void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) const override {

   static float foo = 0.0f;
      static float movement = 0.000f;
      foo += movement;
      if (foo >= 0.9f) {
        movement = -movement;
      } else if (foo <= -0.9f) {
        movement = -movement;
      }

    //Ajustar para aplicar em todos
    // Cria a matriz de modelo (translação)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(foo, 0.0f, 0.0f));

    // (opcional) se quiser identidade no view
    glm::mat4 view = glm::mat4(1.0f);

    // Corrige a projeção com aspecto da janela (isso resolve a distorção!)
    float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

    // Para Vulkan: inverte Y da projeção ortográfica
    proj[1][1] *= -1.0f;

    // Multiplica tudo: World * View * Projection
    glm::mat4 WVP = proj * view * model;

  uniformBuffer.Update(m_device, &WVP, sizeof(WVP), memPos);
 }

};

std::vector<Vertex> createCircle() {
  std::vector<Vertex> circleVertices;
  const int segments = 20;
  const float radius = 0.05f;
  const glm::vec3 center = {0.0f, 0.0f, 0.0f};
  const glm::vec2 centerUV = {0.9f, 0.9f};

  for (int i =0; i<segments; ++i) {
    float angle0 = (float)i / segments * 2.0f * M_PI;
    float angle1 = (float)(i+1) / segments * 2.0f * M_PI;

    const glm::vec3 p0 = {cos(angle0) * radius, sin(angle0) * radius, 0.0f};
    const glm::vec3 p1 = {cos(angle1) * radius, sin(angle1) * radius, 0.0f};

    circleVertices.push_back(Vertex(center, centerUV));
    circleVertices.push_back(Vertex(p0, centerUV));
    circleVertices.push_back(Vertex(p1, centerUV));
  }

  return circleVertices;
}


static Bola bola(createCircle());