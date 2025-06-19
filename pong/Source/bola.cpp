#include "objects.h"
#include "game.h"
// #include <cmath> 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Bola : public Scene::PhysicalObject {
 using PhysicalObject::PhysicalObject;

 void start() override {
  m_velocity = {base_speed, base_speed};
 }

 void update() override {

   static float foo = m_position.y;
      foo += m_velocity.y;
      if (foo >= 0.95f && m_velocity.y > 0) {
        m_velocity.y = -m_velocity.y;
      } else if (foo <= -0.95f && m_velocity.y < 0) {
        m_velocity.y = -m_velocity.y;
      }

      
      // if (m_position.x >= 1.5f && m_velocity.x > 0) {
      //   m_velocity.x = -m_velocity.x;
      // } else if (m_position.x <= -1.5f && m_velocity.x < 0) {
      //   m_velocity.x = -m_velocity.x;
      // }

      std::vector<PhysicalObject*> touching = getCollidingObjects();

      if(touching.size() > 0) {
        touching[0]->isTouchingYou(*this);
      } 

      // if(hit_index == 0 && movementX < 0) {
      //   movementX = -movementX;
      // } else if (hit_index == 1 && movementX > 0) {
      //   movementX = -movementX;

      // printf("bola pos %f\n",m_position.y);
      // }
      
      m_position.y = foo;
      m_position.x += m_velocity.x;
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