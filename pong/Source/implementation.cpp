#include "objects.h"
#include "load_font.h"

using namespace Scene;

MyVK::FontText font_roboto("Textures/playfair_font.ttf");
class Bot : public Object {
  public: 
    using Object::Object;

    void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) override {
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

      UniformData ubo {};
      ubo.WVP = Translate;
      ubo.textureIndex = 2;

      uniformBuffer.Update(m_device, &ubo, sizeof(ubo), memPos);
    }
};

class Player : public Object {
  public: 
    using Object::Object;

    void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) override {
      static float position = 0.0f;

      UniformData ubo {};
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

      ubo.WVP = Translate;
      ubo.textureIndex = 2;
      
      uniformBuffer.Update(m_device, &ubo, sizeof(ubo), memPos);
    }
};

// stb carrega imagens com 0,0 = bottom left

static Bot bot({ //Bot
  Vertex({-0.95f, -1.0f, 0.0f}, {0.0f, 1.0f}), // top left
  Vertex({-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}), // top right
  Vertex({-0.95f, -0.7f, 0.0f}, {0.0f, 0.0f}),
  
  Vertex({-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}),
  Vertex({-0.95f, -0.7f, 0.0f}, {0.0f, 0.0f}),
  Vertex({-1.0f, -0.7f, 0.0f}, {1.0f, 0.0f})
});


static Player player({
  Vertex({0.95f, -1.0f, 0.0f}, {0.0f, 1.0f}), 
  Vertex({1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}), 
  Vertex({0.95f, -0.7f, 0.0f}, {0.0f, 0.0f}),
  
  Vertex({1.0f, -1.0f, 0.0f},{1.0f, 1.0f}),
  Vertex({0.95f, -0.7f, 0.0f},{0.0f, 0.0f}),
  Vertex({1.0f, -0.7f, 0.0f}, {1.0f, 0.0f})
});


class Texto : public Object {
  public: 
    using Object::Object;

    void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) override {
      static int contador = 0;
      contador++;

      static int showNumber = 0;
      if(contador>= 60) {
        contador=0;
        showNumber++;
        if(showNumber >= 10) {
          updateVertexBufferMapped(font_roboto.TextToQuad(("Bot: " + std::to_string(showNumber)).c_str(), (WINDOW_WIDTH/2)/2, 36.0f));
        } else {
          updateVertexBufferMapped(font_roboto.TextToQuad(("Bot: 0" + std::to_string(showNumber)).c_str(), (WINDOW_WIDTH/2)/2, 36.0f));
        }
        
      }
      glm::mat4 flipY = glm::mat4(1.0f);

      UniformData ubo {};
      ubo.WVP = flipY;
      ubo.textureIndex = 0;

      uniformBuffer.Update(m_device, &ubo, sizeof(ubo), memPos);
    }
};

static Texto placarBot(font_roboto.TextToQuad("Bot: 00", (WINDOW_WIDTH/2)/2, 36.0f)); 