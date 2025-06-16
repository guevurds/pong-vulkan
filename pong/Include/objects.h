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

    Position operator+(const Position& other) {
      return Position{x + other.x, y + other.y};
    }

    Position& operator+=(const Position& other) {
      x += other.x;
      y += other.y;
      return *this;
    }
  };
  class VisibleObject {
    public:
      template<typename... Args>
      VisibleObject(Args&&... args) {
        construct(std::forward<Args>(args)...);
      }
      
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
      virtual void onUpdate();

      glm::mat4 m_transform;
      uint32_t m_textureIndex;

      static std::vector<VisibleObject*>& getAll();
    
    private: 
      void internalUpdate(MyVK::BufferAndMemory& uniformBuffer, VkDeviceSize memPos);

      void construct(std::vector<Vertex> vertices);
      void construct(std::vector<Vertex> vertices, uint32_t texture);
      void construct(float x, float y, float w, float h);
      void construct(float x, float y, float w, float h, uint32_t texture);
      void construct(std::vector<Vertex> vertices, float x, float y, float w, float h);
      void construct(std::vector<Vertex> vertices, uint32_t texture, float x, float y, float w, float h);

      void Init(std::vector<Vertex> vertices, Position pos = {0.0f, 0.0f}, Size size = {1.0f, 1.0f}, uint32_t tex = 1);

      std::vector<Vertex> m_verts;
      MyVK::SimpleMesh m_mesh;
      uint32_t m_offset;
      uint32_t m_vertexCount;
      MyVK::VulkanCore m_vkCore;
      MyVK::GraphicsPipeline* m_pPipeline;
      std::vector<VkCommandBuffer> m_cmdBufs;
      VkDevice m_device;
  };

  struct InColision {
      bool x = false;
      bool y = false;
  };

  struct Hitbox {
    float x1;
    float x2;
    float y1;
    float y2;
  };

  class PhysicalObject : public VisibleObject{
    public:
      template<typename... Args>
      PhysicalObject(Args&&... args): VisibleObject::VisibleObject(std::forward<Args>(args)...) {
        Init();
      }
      virtual void isTouchingYou(PhysicalObject& target);

      Position m_velocity;
    protected:
      virtual void start();
      void onUpdate() override;
      std::vector<PhysicalObject*>& getCollidingObjects();

      InColision m_inColision; // não usado no momento;
      Hitbox m_hitbox;

    private:
      void Init();
      static std::vector<PhysicalObject*>& getAllPhysicalObjects();
      std::vector<PhysicalObject*> m_collidingObjects;
      void calculateCollidingObjects();

      bool m_started = false;
  };
}

