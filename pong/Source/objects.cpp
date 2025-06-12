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

Object::Object(vector<Vertex> vertices): m_verts(vertices) {
  m_index = getAll().size();
  m_offset = getAll().size() * sizeof(UniformData);
  m_vertexCount = vertices.size();
  getAll().push_back(this);
  m_textureIndex = 1;
  m_transform = glm::mat4(1.0);
  m_size = {
    .w = 1.0f,
    .h = 1.0f
  };
  m_position = {
    .x = 0.0f,
    .y = 0.0f
  };
}

Object::Object(vector<Vertex> vertices, uint32_t texture): Object(vertices) {
     m_textureIndex = texture;
}

Object::Object(float x, float y, float w, float h): Object(baseQuad()) {
  m_size = {
    .w = w,
    .h = h
  };
  m_position = {
    .x = x,
    .y = y
  };
}

Object::Object(vector<Vertex> vertices, float x, float y, float w, float h): Object(vertices) {
  m_size = {
    .w = w,
    .h = h
  };
  m_position = {
    .x = x,
    .y = y
  };
}

Object::Object(vector<Vertex> vertices, uint32_t texture, float x, float y, float w, float h): Object(vertices, x, y, w, h) {
  m_textureIndex = texture;
}

Object::Object(float x, float y, float w, float h, uint32_t texture): Object(x, y, w, h) {
  m_textureIndex = texture;
}

Object::~Object() {
  m_mesh.Destroy(m_device);
}

int Object::getObjectsNumber() {
  int size = getAll().size();
  return size;
}

void Object::update() {}

void Object::updateAll(MyVK::BufferAndMemory& uniformBuffer) {
  auto& lista = getAll();
  for(size_t i = 0; i < lista.size(); i++) {
    lista[i]->internalUpdate(uniformBuffer, i * sizeof(UniformData));
  }
}

void Object::internalUpdate(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) {
    update();
    // corrigir distorcoes
    float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

    glm::vec3 position = {m_position.x, m_position.y, 0.f};
    glm::vec3 size = {m_size.w, m_size.h, 1.0f};

    m_transform = glm::translate(glm::mat4(1.0f), position);
    m_transform = glm::scale(m_transform, size);

    proj[1][1] *= -1.0f; // inverte y para ficar como plano cartesiano

    glm::mat4 WVP = proj * glm::mat4(1.0f) * m_transform;

    UniformData ubo {
      .WVP = WVP,
      .textureIndex = m_textureIndex
    };
  uniformBuffer.Update(m_device, &ubo, sizeof(ubo), memPos);
}

void Object::createVertexBuffer(VkDevice& device, MyVK::VulkanCore& vkCore) {
  m_vkCore = vkCore;
  m_device = device;
  m_mesh.m_vertexBufferSize = sizeof(m_verts[0]) * m_verts.size();
  vkDeviceWaitIdle(m_device);
  m_mesh.m_vb.Destroy(m_device);
  m_mesh.m_vb = m_vkCore.CreateVertexBuffer(m_verts.data(), m_mesh.m_vertexBufferSize);
}

void Object::createVertexBufferAll(VkDevice& device, MyVK::VulkanCore& vkCore) {
  auto& lista = getAll();
  for(size_t i = 0; i < lista.size(); i++) {
    lista[i]->createVertexBuffer(device, vkCore);
  }
}

void Object::recordCommandBuffer(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) const {
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

void Object::recordCommandBufferAll(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) {

  auto& list = getAll();
  for(size_t i = 0; i < list.size(); i++) {
      list[i]->m_pPipeline = pPipeline;
      list[i]->m_cmdBufs = cmdBufs;
    list[i]->recordCommandBuffer(pPipeline, cmdBufs, index);
  }
}

vector<Object*>& Object::getAll() {
  static vector<Object*> all;
  return all;
}

void Object::updateVertexBufferMapped(std::vector<Vertex> newVerts) {
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
