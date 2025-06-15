#include "objects.h"
#include "load_font.h"
#include "game.h"

using namespace Scene;

float base_speed = 0.02f;

MyVK::FontText font_roboto("pong/Textures/playfair_font.ttf");
class Bot : public VisibleObject {
  public: 
    using VisibleObject::VisibleObject;

    void update() override {
      static float foo = m_position.y;
      static float movement = base_speed;

      foo += movement;
      if (foo >= 0.8f && movement > 0) {
        movement = -movement;
      } else if (foo <= -0.8f && movement < 0) {
        movement = -movement;
      }
      
      m_position.y = foo;
    }
};

class Player : public PhysicalObject {
  public: 
    using PhysicalObject::PhysicalObject;

    void update() override {
      static float position = m_position.y;
      static float movement = base_speed;

      if(key_pressed[0]) { //down
        if(position >= -0.8f) {
          position -= movement;
        }
      }

       if(key_pressed[1]) { //up
        if(position <= 0.8f) {
          position += movement;
        }
      }
     
       m_position.y = position;
    }
};

// stb carrega imagens com 0,0 = bottom left

static Bot bot(-1.3f, 0.9f, 0.08f, 0.4f, 1);

static Player player(1.3f, 0.9f, 0.08f, 0.4f, 1);

class Texto : public VisibleObject {
  public: 
    using VisibleObject::VisibleObject;

    void update() override {
      static int contador = 0;
      contador++;

      static int showNumber = 0;
      if(contador>= 60) {
        contador=0;
        showNumber++;
        if(showNumber >= 10) {
          updateVertexBufferMapped(font_roboto.TextToQuad(("Bot: " + std::to_string(showNumber)).c_str()));
        } else {
          updateVertexBufferMapped(font_roboto.TextToQuad(("Bot: 0" + std::to_string(showNumber)).c_str()));
        } 
      }
    }
};

static VisibleObject placarBot(
  font_roboto.TextToQuad("Bot: 00"), 0,
   0.0f, 0.9f, 1.0f, 1.0f
  ); 