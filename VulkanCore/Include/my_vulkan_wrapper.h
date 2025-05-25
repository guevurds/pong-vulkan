#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "my_types.h"

namespace MyVK {
  void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags);

  VkSemaphore CreateSemaphore(VkDevice Device);
}