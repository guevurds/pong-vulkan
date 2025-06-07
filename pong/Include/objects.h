#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "my_vulkan_simple_mash.h"
#include "my_vulkan_graphics_pipeline.h"

// struct Vertex {
//   Vertex(const glm::vec3& p, const glm::vec2& t) {
//     Pos = p;
//     Tex = t;
//   }

//   glm::vec3 Pos;
//   glm::vec2 Tex;
// };

struct alignas(256) UniformData {
  glm::mat4 WVP;
  alignas(4) int textureIndex; // 4 bytes
  alignas(4) int padding[3]; // 12 bytes para alinhar para 16
  alignas(4) int paddingExtra[44]; // 176 bytes extras para fechar 256
};

extern bool key_pressed[2];

namespace Scene {
  class Object {
    public:
      Object(std::vector<Vertex> vertices);
      virtual ~Object();
      
      
      static int getObjectsNumber();
     
     
      virtual void update(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos) const;
      static void updateAll(MyVK::BufferAndMemory& uniformBuffer);
      
      void createVertexBuffer(VkDevice& device, MyVK::VulkanCore& vkCore);
      static void createVertexBufferAll(VkDevice& device, MyVK::VulkanCore& vkCore);

      void recordCommandBuffer(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) const;
      static void recordCommandBufferAll(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index);

    protected:
      VkDevice m_device;
    
    private: 
      static std::vector<Object*>& getAll();

      std::vector<Vertex> m_verts;
      MyVK::SimpleMesh m_mesh;
      uint32_t m_offset;
      uint32_t m_vertexCount;
  };
}

