#include "objects.h"
#include "load_font.h"
#include "game.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <functional>


using namespace Scene;

void setTimeout(std::function<void()> func, int delayMs) {
    std::thread([func, delayMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        func();
    }).detach(); // roda em background
}


float base_speed = 0.02f;

MyVK::FontText font_roboto("pong/Textures/playfair_font.ttf");
class Bot : public PhysicalObject {
  public: 
    using PhysicalObject::PhysicalObject;

    void start() override{
      std::vector<VisibleObject *> list = getAll();
      if(list.size() >= 6) {
        setMyTarget(*list.at(6)); // 6 é o indice da bola na quantidade atual de elementos do jogo
      }
      
    }

    void update() override {
      static float foo = m_position.y;
      static float movement = base_speed;

      if(m_target_pos->y > m_position.y && movement < 0) {
          movement = -movement;
      } else if (m_target_pos->y < m_position.y && movement > 0) {
          movement = -movement;
      }

      if (foo <= 0.8f && movement > 0) {
        foo += movement;
      } else if (foo >= -0.8f && movement < 0) {
        foo += movement;
      }

      m_position.y = foo;
    }

    void isTouchingYou(PhysicalObject& target) override{
      if(target.m_velocity.x < 0) {
        target.m_velocity.x = -target.m_velocity.x*1.2f;
        target.m_velocity.y = target.m_velocity.y*1.2f;
      }
    }

    Position* m_target_pos;
      void setMyTarget(VisibleObject& target) {
      m_target_pos = &target.m_position;
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

    void isTouchingYou(PhysicalObject& target) override{
      if(target.m_velocity.x > 0) {
        target.m_velocity.x = -target.m_velocity.x*1.2f;
        target.m_velocity.y = target.m_velocity.y*1.2f;
      }
    }
};

class Goal: public PhysicalObject {
  public:
    Goal(Position pos, int& placar):PhysicalObject(0.0f, 0.0f, 0.08f, 2.0f, 2) {
      m_position = pos;
      m_placar = &placar;
    };

    int* m_placar;

    void isTouchingYou(PhysicalObject& target) override{
      target.m_position = {0.0f, 0.0f};
      *m_placar = *m_placar + 1;
      int direction[2];
      direction[0] = target.m_velocity.x > 0 ? 1 : -1;
      direction[1] = target.m_velocity.y > 0 ? 1 : -1;
      target.m_velocity = {0.0f, 0.0f};
      std::this_thread::sleep_for(std::chrono::seconds(1));
      setTimeout([&target, direction]() {
        printf("executou direction[0] is %d\n", direction[0]);
        target.m_velocity = {base_speed*direction[0], base_speed*direction[1]};
      }, 2000);
    }
};

// stb carrega imagens com 0,0 = bottom left

static Bot robot(-1.3f, 0.9f, 0.08f, 0.4f, 1);

static Player player(1.3f, 0.9f, 0.08f, 0.4f, 1);

class Placar : public VisibleObject {
  public: 
    Placar(std::string user, int& num, Position pos)
    :VisibleObject(
      font_roboto.TextToQuad((user+ ": " + std::to_string(num)).c_str()), 
      0,
      pos.x,
      pos.y, 
      2.f, 
      2.f
    ) {
      m_user = user;
      m_num = &num;
    }; 

    std::string m_user;
    int* m_num;

    void update() override {
      updateVertexBufferMapped(font_roboto.TextToQuad((m_user+ ": " + std::to_string(*m_num)).c_str()));
    }
};

static int numPlacarPlayer = 0;
static int numPlacarBot = 0;

static Placar placarBot((std::string)"Bot", numPlacarBot, {-1.0f, 0.85f}); 
static Placar placarPlayer((std::string)"Player", numPlacarPlayer, {0.75f, 0.85f}); 

static Goal goalPlayer({1.6f, 0.0f}, numPlacarBot);
static Goal goalBot({-1.6f, 0.0f}, numPlacarPlayer);