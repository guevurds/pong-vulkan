#include <vulkan/vulkan.h>

#include "my_vulkan_util.h"
#include "my_vulkan_queue.h"
#include "my_vulkan_wrapper.h"

namespace MyVK {
  void VulkanQueue::Init(VkInstance& Instance , VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex) {  
    m_device = Device;
    m_swapChain = SwapChain;
    m_instance = Instance; // modificado para tentar evitar o erro  vkAcquireNextImageKHR(): Semaphore must not have any pending operations.

    vkGetDeviceQueue(Device, QueueFamily, QueueIndex, &m_queue);

    printf("Queue acquired\n");

    CreateSemaphores();
  }

  void VulkanQueue::Destroy() {
    vkDestroySemaphore(m_device, m_presentCompleteSem, NULL);
    vkDestroySemaphore(m_device, m_renderCompleSem, NULL);
  }

  void VulkanQueue::CreateSemaphores() {
    m_presentCompleteSem = CreateSemaphore(m_device);
    m_renderCompleSem = CreateSemaphore(m_device);
  }

  void VulkanQueue::WaitIdle() {
    vkQueueWaitIdle(m_queue);
  }

  u32 VulkanQueue::AcquireNextImage() {
    u32 ImageIndex = 0;
    PFN_vkAcquireNextImageKHR vkAcquireNextImage = VK_NULL_HANDLE;
    vkAcquireNextImage = (PFN_vkAcquireNextImageKHR)vkGetInstanceProcAddr(m_instance, "vkAcquireNextImageKHR"); // modificado pra tentar resolver um bug (não deu certo)

    VkResult res = vkAcquireNextImage(m_device, m_swapChain, UINT64_MAX, m_presentCompleteSem, NULL, &ImageIndex);
    CHECK_VK_RESULT(res, "vkAcquireNextImageKHR\n");
    return ImageIndex;
  }

  void VulkanQueue::SubmitSync(VkCommandBuffer CmbBuf) {
    VkSubmitInfo SubmitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = NULL,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = VK_NULL_HANDLE,
      .pWaitDstStageMask = VK_NULL_HANDLE, //?
      .commandBufferCount = 1,
      .pCommandBuffers = &CmbBuf,
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = VK_NULL_HANDLE
    };

    VkResult res = vkQueueSubmit(m_queue, 1, &SubmitInfo, NULL);
    CHECK_VK_RESULT(res, "vkQueueSubmit\n");
  }

  void VulkanQueue::SubmitAsync(VkCommandBuffer CmbBuf) {
    VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo SubmitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = NULL,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &m_presentCompleteSem,
      .pWaitDstStageMask = &waitFlags, //?
      .commandBufferCount = 1,
      .pCommandBuffers = &CmbBuf,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &m_renderCompleSem
    };

    VkResult res = vkQueueSubmit(m_queue, 1, &SubmitInfo, NULL);
    CHECK_VK_RESULT(res, "vkQueueSubmit\n");
  }

  void VulkanQueue::Present(u32 ImageIndex) {
    VkPresentInfoKHR PresentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = NULL,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &m_renderCompleSem,
      .swapchainCount = 1, 
      .pSwapchains = &m_swapChain,
      .pImageIndices = &ImageIndex
    };

    VkResult res = vkQueuePresentKHR(m_queue, &PresentInfo);
    CHECK_VK_RESULT(res, "vkQueuePresentKHR\n")

    WaitIdle(); // TODO: looks like a workaround but we're getting error messages without it
  }


}