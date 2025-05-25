#pragma once 

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>

#include "my_types.h"

//macro para verificar se o retorno de uma operação da api foi bem sucedido
// \ permite quebrar uma linha concatenando as no C/C++ 
// define permite multiplas linhas
// define na vdd define um macro que substitui o texto na hora da compilação
#define CHECK_VK_RESULT(res, msg)  \
  if(res != VK_SUCCESS) { \
    fprintf(stderr, "Error in %s:%d - %s, code %x\n", __FILE__, __LINE__, msg, res); \
    exit(1); \
  }

  namespace MyVK{
    const char* GetDebugServerityStr(VkDebugUtilsMessageSeverityFlagBitsEXT Severity);
    const char* GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type);
  }
