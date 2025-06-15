#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "my_vulkan_simple_mash.h"
#include "my_vulkan_graphics_pipeline.h"
#include "my_vulkan_core.h"

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
  alignas(4) unsigned int textureIndex; // 4 bytes
  alignas(4) int padding[3]; // 12 bytes para alinhar para 16
  alignas(4) int paddingExtra[44]; // 176 bytes extras para fechar 256
};

namespace Scene {
  struct Size {
    float w;
    float h;
  };
  struct Position {
    float x;
    float y;
  };
  class VisibleObject {
    public:
      VisibleObject(std::vector<Vertex> vertices);
      VisibleObject(std::vector<Vertex> vertices, uint32_t texture);
      VisibleObject(float x, float y, float w, float h);
      VisibleObject(float x, float y, float w, float h, uint32_t texture);
      VisibleObject(std::vector<Vertex> vertices, float x, float y, float w, float h);
      VisibleObject(std::vector<Vertex> vertices, uint32_t texture, float x, float y, float w, float h);
      
      virtual ~VisibleObject();
      
      static int getObjectsNumber();
     
      static void updateAll(MyVK::BufferAndMemory& uniformBuffer);
      
      void createVertexBuffer(VkDevice& device, MyVK::VulkanCore& vkCore);
      static void createVertexBufferAll(VkDevice& device, MyVK::VulkanCore& vkCore);

      void recordCommandBuffer(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index) const;
      static void recordCommandBufferAll(MyVK::GraphicsPipeline* pPipeline, std::vector<VkCommandBuffer>& cmdBufs, int index);

      void updateVertexBufferMapped(std::vector<Vertex> newVerts); 

      Position m_position;
      Size m_size;
      uint32_t m_index;

    protected:
      virtual void update();

      glm::mat4 m_transform;
      uint32_t m_textureIndex;

      static std::vector<VisibleObject*>& getAll();
    
    private: 
      void internalUpdate(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos);

      std::vector<Vertex> m_verts;
      MyVK::SimpleMesh m_mesh;
      uint32_t m_offset;
      uint32_t m_vertexCount;
      MyVK::VulkanCore m_vkCore;
      MyVK::GraphicsPipeline* m_pPipeline;
      std::vector<VkCommandBuffer> m_cmdBufs;
      VkDevice m_device;
  };
}

