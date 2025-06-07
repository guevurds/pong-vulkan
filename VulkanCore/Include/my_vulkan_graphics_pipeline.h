#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "my_vulkan_simple_mash.h"

struct Vertex {
        Vertex(const glm::vec3& p, const glm::vec2& t) {
          Pos = p;
          Tex = t;
        }

        glm::vec3 Pos;
        glm::vec2 Tex;
      };

namespace MyVK {
  class GraphicsPipeline {
    public:
      GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow, VkRenderPass RenderPass, VkShaderModule vs, VkShaderModule fs, int NumImages, std::vector<BufferAndMemory>& UniformBuffer, int UniformDataSize, std::vector<VkDescriptorImageInfo>& ImageInfos);

      ~GraphicsPipeline();

      void Bind(VkCommandBuffer CmdBuf, int ImageIndex, uint32_t dynamicOffset);

    private:

      void CreateDescriptorPool(int NumImages);
      void CreateDescriptorSets(int NumImages, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize, std::vector<VkDescriptorImageInfo>& ImageInfos);
      void CreateDescriptorSetLayout(std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);
      void AllocateDescriptorSets(int NumImages);
      void UpdateDescriptorSets(int NumImages, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize, std::vector<VkDescriptorImageInfo>& ImageInfos);

      VkDevice m_device = NULL;
      VkPipeline m_pipeline = NULL;
      VkPipelineLayout m_pipelineLayout = NULL;
      VkDescriptorPool m_descriptorPool;
      VkDescriptorSetLayout m_descriptorSetLayout;
      std::vector<VkDescriptorSet> m_descriptorSets;
  };
}