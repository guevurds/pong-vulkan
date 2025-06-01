#include "objects.h"


using namespace Scene;

class Bot : public Object {
  public: 
    using Object::Object;

    void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) const override {
      static float foo = 0.0f;
      glm::mat4 Translate = glm::mat4(1.0);
      static float movement = 0.006f;
      Translate = glm::translate(Translate, glm::vec3(0.0f, foo, 1.0f));
      foo += movement;
      if (foo >= 1.7f) {
        movement = -movement;
      } else if (foo <= 0.0f) {
        movement = -movement;
      }
      uniformBuffer.Update(m_device, &Translate, sizeof(Translate), memPos);
    }
};

class Player : public Object {
  public: 
    using Object::Object;

    void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) const override {
      static float position = 0.0f;
      glm::mat4 Translate = glm::mat4(1.0);
      static float movement = 0.006f;
      Translate = glm::translate(Translate, glm::vec3(0.0f, position, 1.0f));

      if(key_pressed[0]) { //down
        if(position <= 1.7f) {
          position += movement;
        }
      }

       if(key_pressed[1]) { //up
        if(position >= 0.0f) {
          position -= movement;
        }
      }
      
      uniformBuffer.Update(m_device, &Translate, sizeof(Translate), memPos);
    }
};

static Bot bot({ //Bot
  Vertex({-0.95f, -1.0f, 0.0f}, {0.0f, 0.0f}), // top left
  Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}), // top right
  Vertex({-0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
  
  Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
  Vertex({-0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
  Vertex({-1.0f, -0.7f, 0.0f}, {0.0f, 1.0f})
});


static Object bola({ //bola
  Vertex({-0.05f, 0.0f, 0.0f}, {0.0f, 0.0f}), // top left
  Vertex({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}), // top right
  Vertex({-0.05f, -0.05f, 0.0f}, {1.0f, 1.0f}),
  
  Vertex({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
  Vertex({-0.05f, -0.05f, 0.0f}, {1.0f, 1.0f}),
  Vertex({0.0f, -0.05f, 0.0f}, {0.0f, 1.0f})
});

static Player player({
  Vertex({0.95f, -1.0f, 0.0f}, {0.0f, 0.0f}), // top left
  Vertex({1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}), // top right
  Vertex({0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
  
  Vertex({1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
  Vertex({0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
  Vertex({1.0f, -0.7f, 0.0f}, {0.0f, 1.0f})
});