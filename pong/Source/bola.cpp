#include "objects.h"
#include "game.h"
// #include <cmath> 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Bola : public Scene::VisibleObject {
 using VisibleObject::VisibleObject;

 void update() override {

   static float foo = m_position.y;
      static float movement = base_speed;
      static float movementX = base_speed;
      foo += movement;
      if (foo >= 0.95f && movement > 0) {
        movement = -movement;
      } else if (foo <= -0.95f && movement < 0) {
        movement = -movement;
      }

      
      if (m_position.x >= 1.5f && movementX > 0) {
        movementX = -movementX;
      } else if (m_position.x <= -1.5f && movementX < 0) {
        movementX = -movementX;
      }

      int hit_index = hitbox();

      if(hit_index == 0 && movementX < 0) {
        movementX = -movementX;
      } else if (hit_index == 1 && movementX > 0) {
        movementX = -movementX;
      }
      
      m_position.y = foo;
      m_position.x += movementX;
 }

 int hitbox() {
  auto& objects = getAll();

  float m_x1 = m_position.x + m_size.w/2; // direita
  float m_x2 = m_position.x - m_size.w/2; // esquerda
  float m_y1 = m_position.y + m_size.h/2; // cima
  float m_y2 = m_position.y - m_size.h/2; // baixo

  for (auto obj:objects) {
    if(obj->m_index == m_index) continue;

    float x1 = obj->m_position.x + obj->m_size.w/2;
    float x2 = obj->m_position.x - obj->m_size.w/2;
    float y1 = obj->m_position.y + obj->m_size.h/2;
    float y2 = obj->m_position.y - obj->m_size.h/2;

    bool overlayX = (x1 > m_x2) && (x2 < m_x1);
    bool overlayY = (y1> m_y2) && (y2 < m_y1);

    if (overlayX&&overlayY) {
      return obj->m_index;
    }
  }
  return -1;
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


static Bola bola(0.0f, 0.0f, 0.1f, 0.1f, 1);