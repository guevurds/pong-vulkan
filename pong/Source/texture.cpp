#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

#include "my_vulkan_core.h"
#include "my_vulkan_simple_mash.h"

using namespace MyVK;

 Texture VulkanCore::LoadTexture(VkCommandPool commandPool, VkQueue graphicsQueue, const char* filePath) {
  Texture texture{};

  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load("../Textures/azul.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if(!pixels) {
    throw std::runtime_error("Failed to load texture image!");
  }

  // 1. Criar staging buffer
  BufferAndMemory staging = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  
 }

