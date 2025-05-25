#pragma once 

#include <stdio.h>

#include <vulkan/vulkan.h>

#include "my_types.h"

namespace MyVK {
  class VulkanQueue {
    public:
      VulkanQueue() {}
      ~VulkanQueue() {}

      void Init(VkInstance& Instance, VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex); //modificado pra usar o Instance

      void Destroy();

      u32 AcquireNextImage();

      void SubmitSync(VkCommandBuffer CmbBuf);

      void SubmitAsync(VkCommandBuffer CmbBuf);

      void Present(u32 ImageIndex);

      void WaitIdle();

    private:
      void CreateSemaphores();

      VkDevice m_device = VK_NULL_HANDLE;
      VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
      VkQueue m_queue = VK_NULL_HANDLE;
      VkSemaphore m_renderCompleSem;
      VkSemaphore m_presentCompleteSem;

      VkInstance m_instance = VK_NULL_HANDLE; //adicionei pra ver se resolvo o erro vkAcquireNextImageKHR(): Semaphore must not have any pending operations.
  };


}