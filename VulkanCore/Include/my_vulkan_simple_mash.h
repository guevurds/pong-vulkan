#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "my_types.h"
#include "my_vulkan_core.h"

namespace MyVK {
  struct SimpleMesh {
    BufferAndMemory m_vb;
    size_t m_vertexBufferSize = 0;

    void Destroy(VkDevice Device) {
      m_vb.Destroy(Device);
    }
  };
}