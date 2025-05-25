#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "my_vulkan_simple_mash.h"

namespace MyVK {
  class GraphicsPipeline {
    public:
      GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow, VkRenderPass RenderPass, VkShaderModule vs, VkShaderModule fs, const SimpleMesh* pMesh, int NumImages, std::vector<BufferAndMemory>& UniformBuffer, int UniformDataSize);

      ~GraphicsPipeline();

      void Bind(VkCommandBuffer CmdBuf, int ImageIndex);
      void BindWithSet(int NumImages, const SimpleMesh* pMesh, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);

    private:

      void CreateDescriptorPool(int NumImages);
      void CreateDescriptorSets(int NumImages, const SimpleMesh* pMesh, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);
      void CreateDescriptorSetLayout(std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);
      void AllocateDescriptorSets(int NumImages);
      void UpdateDescriptorSets(int NumImages, const SimpleMesh* pMesh, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize);

      VkDevice m_device = NULL;
      VkPipeline m_pipeline = NULL;
      VkPipelineLayout m_pipelineLayout = NULL;
      VkDescriptorPool m_descriptorPool;
      VkDescriptorSetLayout m_descriptorSetLayout;
      std::vector<VkDescriptorSet> m_descriptorSets;
  };
}