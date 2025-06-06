#pragma once // uma diretiva para evitar que o cabeçalho seja definido mais de uma vez, substitui o ifndef define endif

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "my_vulkan_util.h"
#include "my_vulkan_device.h"
#include "my_vulkan_queue.h"

namespace MyVK {

  class BufferAndMemory {
    public:
      BufferAndMemory() {}
      VkBuffer m_buffer = NULL;
      VkDeviceMemory m_mem = NULL;
      VkDeviceSize m_allocationSize = 0;

      void Update(VkDevice Device, const void* pData, size_t Size, VkDeviceSize Offset);

      void Destroy(VkDevice Device);
  };

  struct ImageAndMemory {
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_mem = VK_NULL_HANDLE;
  };

  class VulkanCore {
    public:
      VulkanCore();
      ~VulkanCore();

      void Init(const char* pAppName, GLFWwindow* pWindow);

      ImageAndMemory LoadTexture(const char* filename);
      void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
      void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

      VkRenderPass CreateSimpleRenderPass();

      std::vector<VkFramebuffer> CreateFramebuffers(VkRenderPass RenderPass) const;
      
      void DestroyFramebuffers(std::vector<VkFramebuffer>& Framebuffers);

      VkDevice& GetDevice() {return m_device;}

      int GetNumImages() const {return (int)m_images.size();}

      const VkImage& GetImage(int Index) const;

      VulkanQueue* GetQueue() {return &m_queue;}

      u32 GetQueueFamily() const {return m_queueFamily;}

      // const std::vector<VkFramebuffer>& GetFramebuffers() const {return m_frameBuffers;}

      void CreateCommandBuffers(u32 Count, VkCommandBuffer* pCmdbufs);
      
      void FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs);

      BufferAndMemory CreateVertexBuffer(const void* pVertices, size_t Size);

      std::vector<BufferAndMemory> CreateUniformBuffers(size_t DataSize);

      void GetFramebufferSize(int& Width, int& Height) const;
    
    private:
      void CreateInstance(const char* pAppName);
      void CreateDebugCallback();
      void CreateSurface();
      void CreateDevice();
      void CreateSwapChain();
      void CreateCommandBufferPool();
      BufferAndMemory CreateUniformBuffer(int Size);

      u32 GetMemoryTypeIndex(u32 memTypeBits, VkMemoryPropertyFlags memPropFlags);

      void CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);

      BufferAndMemory CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);

      VkInstance m_instance = VK_NULL_HANDLE; // m_ sufixo para indicar que pertence a uma classe, VKInstance é um tipo definido pela api Vulkan
      VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE; // var para debug, VK_NULL_HANDLE para garantir um estado inicial valido
      
      GLFWwindow* m_pWindow = NULL;
      VkSurfaceKHR m_surface = VK_NULL_HANDLE;
      VulkanPhysicalDevices m_physDevices;
      u32 m_queueFamily = 0;
      VkDevice m_device;
      VkSurfaceFormatKHR m_swapChainSurfaceFormat;
      VkSwapchainKHR m_swapChain;
      std::vector<VkImage> m_images;
      std::vector<VkImageView> m_imageViews;
      VkCommandPool m_cmdBufPool;
      VulkanQueue m_queue;
      VkCommandBuffer m_copyCmdBuf;
      // std::vector<VkFramebuffer> m_frameBuffers;
  };
}