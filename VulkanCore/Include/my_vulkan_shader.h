#pragma once

#include <vulkan/vulkan.h>

namespace MyVK {
  VkShaderModule CreateShaderModuleFromBinary(VkDevice device, const char* pFilename);

  VkShaderModule CreateShaderModuleFromText(VkDevice device, const char* pFilename);
}