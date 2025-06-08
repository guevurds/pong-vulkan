#include <string>
#include <iostream>

#include "objects.h"

using namespace std;
using namespace Scene;

bool key_pressed[2] = {false, false};

Object::Object(vector<Vertex> vertices): m_verts(vertices) {
  m_offset = getAll().size() * sizeof(UniformData);
  m_vertexCount = vertices.size();
  getAll().push_back(this);
}

Object::~Object() {
  m_mesh.Destroy(m_device);
}

int Object::getObjectsNumber() {
  int size = getAll().size();
  return size;
}

void Object::update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) const {
  glm::mat4 Transform = glm::mat4(1.0);
  UniformData ubo {
    .WVP = Transform,
    .textureIndex = 0
  };
  uniformBuffer.Update(m_device, &ubo, sizeof(ubo), memPos);
}

void Object::updateAll(MyVK::BufferAndMemory& uniformBuffer) {
  auto& lista = getAll();
  for(size_t i = 0; i < lista.size(); i++) {
    lista[i]->update(uniformBuffer, i * sizeof(UniformData));
  }
}

void Object::createVertexBuffer(VkDevice& device, MyVK::VulkanCore& vkCore) {
  m_device = device;
  m_mesh.m_vertexBufferSize = sizeof(m_verts[0]) * m_verts.size();
  m_mesh.m_vb = vkCore.CreateVertexBuffer(m_verts.data(), m_mesh.m_vertexBufferSize);
}

void Object::createVertexBufferAll(VkDevice& device, MyVK::VulkanCore& vkCore) {
  auto& lista = getAll();
  for(size_t i = 0; i < lista.size(); i++) {
    lista[i]->createVertexBuffer(device, vkCore);
  }
}

void Object::recordCommandBuffer(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) const{
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
    list[i]->recordCommandBuffer(pPipeline, cmdBufs, index);
  }
}

vector<Object*>& Object::getAll() {
  static vector<Object*> all;
  return all;
}
