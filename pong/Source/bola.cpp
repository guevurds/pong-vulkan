#include "objects.h"


class Bola : public Scene::Object {
 using Object::Object;

 void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) const override {

   static float foo = 0.0f;
      glm::mat4 Translate = glm::mat4(1.0);
      static float movement = 0.006f;
      Translate = glm::translate(Translate, glm::vec3(foo, 0.0f, 1.0f));
      foo += movement;
      if (foo >= 1.7f) {
        movement = -movement;
      } else if (foo <= 0.0f) {
        movement = -movement;
      }

  uniformBuffer.Update(m_device, &Translate, sizeof(Translate), memPos);
 }

};


static Bola bola({ //bola
  Vertex({-0.05f, 0.0f, 0.0f}, {0.0f, 0.0f}), // top left
  Vertex({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}), // top right
  Vertex({-0.05f, -0.05f, 0.0f}, {1.0f, 1.0f}),
  
  Vertex({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
  Vertex({-0.05f, -0.05f, 0.0f}, {1.0f, 1.0f}),
  Vertex({0.0f, -0.05f, 0.0f}, {0.0f, 1.0f})
});