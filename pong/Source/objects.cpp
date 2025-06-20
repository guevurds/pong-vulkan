#include <string>
#include <iostream>

#include "objects.h"
#include "game.h"

using namespace std;
using namespace Scene;

bool key_pressed[2] = {false, false};

vector<Vertex> baseQuad () {
  return {
    Vertex({-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}),
    Vertex({ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}),
    Vertex({ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}),
    Vertex({-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}),
    Vertex({ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}),
    Vertex({-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f})
  };
}

void VisibleObject::construct(vector<Vertex> vertices) {
  Init(vertices);
}

void VisibleObject::construct(vector<Vertex> vertices, uint32_t texture) {
  Init(vertices, {},{},texture);
}

void VisibleObject::construct(float x, float y, float w, float h) {
  Init(baseQuad(),{x,y},{w,h});
}

void VisibleObject::construct(vector<Vertex> vertices, float x, float y, float w, float h) {
  Init(vertices, {x,y},{w,h});
}

void VisibleObject::construct(vector<Vertex> vertices, uint32_t texture, float x, float y, float w, float h) {
  Init(vertices, {x,y},{w,h}, texture);
}

void VisibleObject::construct(float x, float y, float w, float h, uint32_t texture) {
  Init(baseQuad(), {x,y},{w,h}, texture);
}

void VisibleObject::Init(vector<Vertex> vertices, Position pos, Size size, uint32_t tex) {
  m_verts = vertices;
  m_index = getAll().size();
  m_offset = getAll().size() * sizeof(UniformData);
  m_vertexCount = m_verts.size();
  getAll().push_back(this);
  m_textureIndex = tex;
  m_transform = glm::mat4(1.0);
  m_size = {
    .w = size.w,
    .h = size.h
  };
  m_position = {
    .x = pos.x,
    .y = pos.y
  };
}

VisibleObject::~VisibleObject() {
  m_mesh.Destroy(m_device);
}

int VisibleObject::getObjectsNumber() {
  int size = getAll().size();
  return size;
}

void VisibleObject::update() {}

void VisibleObject::onUpdate() {
  update();
}

void VisibleObject::updateAll(MyVK::BufferAndMemory& uniformBuffer) {
  auto& lista = getAll();
  for(size_t i = 0; i < lista.size(); i++) {
    lista[i]->internalUpdate(uniformBuffer, i * sizeof(UniformData));
  }
}

void VisibleObject::internalUpdate(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) {
    onUpdate();
    // corrigir distorcoes
    float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

    glm::vec3 position = {m_position.x, m_position.y, 0.f};
    glm::vec3 size = {m_size.w, m_size.h, 1.0f};

    m_transform = glm::translate(glm::mat4(1.0f), position);
    m_transform = glm::scale(m_transform, size);

    proj[1][1] *= -1.0f; // inverte y para ficar como plano cartesiano

    glm::mat4 WVP = proj * glm::mat4(1.0f) * m_transform;

    UniformData ubo {};
    ubo.WVP = WVP;
    ubo.textureIndex = m_textureIndex;
  uniformBuffer.Update(m_device, &ubo, sizeof(ubo), memPos);
}

void VisibleObject::createVertexBuffer(VkDevice& device, MyVK::VulkanCore& vkCore) {
  m_vkCore = vkCore;
  m_device = device;
  m_mesh.m_vertexBufferSize = sizeof(m_verts[0]) * m_verts.size();
  vkDeviceWaitIdle(m_device);
  m_mesh.m_vb.Destroy(m_device);
  m_mesh.m_vb = m_vkCore.CreateVertexBuffer(m_verts.data(), m_mesh.m_vertexBufferSize);
}

void VisibleObject::createVertexBufferAll(VkDevice& device, MyVK::VulkanCore& vkCore) {
  auto& lista = getAll();
  for(size_t i = 0; i < lista.size(); i++) {
    lista[i]->createVertexBuffer(device, vkCore);
  }
}

void VisibleObject::recordCommandBuffer(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) const {
  pPipeline->Bind(cmdBufs[index], index, m_offset);
  VkDeviceSize vbOffset = 0;
  vkCmdBindVertexBuffers(
    cmdBufs[index],
    0, 1,
    &m_mesh.m_vb.m_buffer,                      
    &vbOffset
  );
  u32 InstaceCout = 1;
  u32 FirstVertex = 0;
  u32 FirstInstance = 0;

  vkCmdDraw(cmdBufs[index], m_vertexCount, InstaceCout, FirstVertex, FirstInstance);
}

void VisibleObject::recordCommandBufferAll(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) {

  auto& list = getAll();
  for(size_t i = 0; i < list.size(); i++) {
      list[i]->m_pPipeline = pPipeline;
      list[i]->m_cmdBufs = cmdBufs;
    list[i]->recordCommandBuffer(pPipeline, cmdBufs, index);
  }
}

vector<VisibleObject*>& VisibleObject::getAll() {
  static vector<VisibleObject*> all;
  return all;
}

void VisibleObject::updateVertexBufferMapped(std::vector<Vertex> newVerts) {
   m_verts = newVerts;
  if(newVerts.size() * sizeof(Vertex) > m_mesh.m_vertexBufferSize) {
    // createVertexBuffer(m_device, m_vkCore);
    // recordCommandBuffer(m_pPipeline, m_cmdBufs, m_index); /
  } else {
    void* data;
    vkMapMemory(m_device, m_mesh.m_vb.m_mem, 0, sizeof(Vertex) * m_verts.size(), 0, &data);
    memcpy(data, m_verts.data(), sizeof(Vertex) * m_verts.size());
    vkUnmapMemory(m_device, m_mesh.m_vb.m_mem);
    // m_vertexCount = newVerts.size();
  }
}

// implementation physicalObject

void PhysicalObject::Init() { 
  getAllPhysicalObjects().push_back(this);
  m_velocity = {0.0f, 0.0f};
  m_hitbox = { 
    m_position.x - m_size.w/2, // esquerda
    m_position.x + m_size.w/2, // direita
    m_position.y + m_size.h/2, // cima
    m_position.y - m_size.h/2 // baixo
  };
  m_started = false;
}

vector<PhysicalObject*>& PhysicalObject::getAllPhysicalObjects() {
  static vector<PhysicalObject*> all;
  return all;
}

void PhysicalObject::calculateCollidingObjects() {
  auto& objects = getAllPhysicalObjects();

  vector<PhysicalObject*> inCollidingObjs;

  for (auto obj:objects) {
    if(obj->m_index == m_index) continue;

    bool overlayX = (obj->m_hitbox.x1 < m_hitbox.x2) && (obj->m_hitbox.x2 > m_hitbox.x1);
    bool overlayY = (obj->m_hitbox.y1 > m_hitbox.y2) && (obj->m_hitbox.y2 < m_hitbox.y1);

    if (overlayX&&overlayY) {
      inCollidingObjs.push_back(obj);
    }
  }
  m_collidingObjects = inCollidingObjs;
 }

 void PhysicalObject::isTouchingYou(PhysicalObject& target) {}

 void PhysicalObject::onUpdate() {
  if(!m_started) {
    start();
    m_started = true;
  }

   m_hitbox = { 
    m_position.x - m_size.w/2, // esquerda
    m_position.x + m_size.w/2, // direita
    m_position.y + m_size.h/2, // cima
    m_position.y - m_size.h/2 // baixo
  };
  // m_position += m_velocity;  // melhorar para teste de colisão no futuro
  calculateCollidingObjects();
  VisibleObject::onUpdate();
 }

 std::vector<PhysicalObject*>& PhysicalObject::getCollidingObjects() {
  return m_collidingObjects;
 }

 void PhysicalObject::start() {}