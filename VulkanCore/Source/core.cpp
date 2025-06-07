#include <vector>
#include <assert.h>
#include <iostream>

#include "my_types.h"
#include "my_util.h"
#include "my_vulkan_core.h"
#include "my_vulkan_util.h"
#include "my_vulkan_wrapper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace MyVK {


  //static para chamar a função somente no arquivo em que foi denifida
  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( // VKAPI_ATTR, VKAPI_CALL macros para garantir que a função esta sendo chamada corretamente, VkBool32 um tipo do vulkan 
    VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
    VkDebugUtilsMessageTypeFlagsEXT Type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
      printf("Debug callback: %s\n", pCallbackData->pMessage);
      printf("Severity %s\n", GetDebugServerityStr(Severity));
      printf("Type %s\n", GetDebugType(Type));
      printf("Objects");

      for (u32 i =0; i< pCallbackData->objectCount; i++) {
        #ifdef _WIN32
          printf("%llux ", pCallbackData->pObjects[i].objectHandle);
        #else 
          printf("%lux ", pCallbackData->pObjects[i].objectHandle);
        #endif
      }

      printf("\n");

      return VK_FALSE; // The calling function should not be aborted
    }


  VulkanCore::VulkanCore() {}

  VulkanCore::~VulkanCore() {
    printf("----------------------\n");

    vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &m_copyCmdBuf);

    vkDestroyCommandPool(m_device, m_cmdBufPool, NULL);

    // for(int i = 0; i < m_frameBuffers.size(); i++) {
    //   vkDestroyFramebuffer(m_device, m_frameBuffers[i], NULL);
    // }

    m_queue.Destroy();

    for(int i= 0; i< m_imageViews.size(); i++) {
      vkDestroyImageView(m_device, m_imageViews[i], NULL);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, NULL);
    printf("swapChain destroyed\n");


    vkDestroyDevice(m_device, NULL);
    printf("logical device destroyed\n");

    //destruir o surface
    PFN_vkDestroySurfaceKHR vkDestroySurface = VK_NULL_HANDLE;
    vkDestroySurface = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
    if(!vkDestroySurface) {
      MY_ERROR0("Cannot find address of vkDestroyDebugUtilsMessenger\n");
      exit(1);
    }

    vkDestroySurface(m_instance, m_surface, NULL);

    printf("GLFW window surface destroyed\n");

    //destruir o mensageiro
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE; //PFN_vkDestroyDebugUtilsMessengerEXT é um ponteiro pra uma função
    vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"); //vkGetInstanceProcAddr func para carregar dinamicamente funções da API vulkan na instancia com os parametros, m_instance (a instancia), "vkDestroyDebugUtilsMessengerEXT" (nome da func)
    if(!vkDestroyDebugUtilsMessenger) {
      MY_ERROR0("Cannot find address of vkDestroyDebugUtilsMessenger");
      exit(1);
    }
    vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, NULL); //Destroi debug mensenger para liberar recurso 

    printf("Debug callback destroyed\n");

    vkDestroyInstance(m_instance, NULL);
    printf("Vulkan instance destroyed\n");
  }

  void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow) {
    m_pWindow = pWindow;
    CreateInstance(pAppName);
    CreateDebugCallback();

    if(!pWindow) {
      return;
    }

    CreateSurface();
    m_physDevices.Init(m_instance, m_surface);
    m_queueFamily = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
    CreateDevice();
    CreateSwapChain();
    CreateCommandBufferPool();
    m_queue.Init(m_instance ,m_device, m_swapChain, m_queueFamily, 0);
    CreateCommandBuffers(1, &m_copyCmdBuf);
  }

  const VkImage& VulkanCore::GetImage(int Index) const {
    if (Index >= m_images.size()) {
      MY_ERROR("Invalid image index %d\n", Index);
      exit(1);
    }

    return m_images[Index];
  }

  void VulkanCore::CreateInstance(const char* pAppName) {
    std::vector<const char*> Layers = {
      "VK_LAYER_KHRONOS_validation" // Camada de validação do vulkan, khronos é a org responsavel pelo vulkan
    };

    std::vector<const char*> Extensions = { //std::vector<const char*> vetor dinamico que armazena ponteiros para um string constantes
      VK_KHR_SURFACE_EXTENSION_NAME, //constante do vulkan que permite que a api intereja com superficies (locais de renderização)
      #if defined (_WIN32) // diretiva de pré processador para escrever codigo condicional de acordo com a plataforma
        "VK_KHR_win32_surface", // necessaria para surfaces no windows 
      #endif
      #if defined (__APPLE__)
        "VK_MVK_macos_surface",
      #endif
      #if defined (__linux__)
        "VK_KHR_xcb_surface",
      #endif
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME // para depuração
    };

    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = { // Adicional?
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, 
      .pNext = NULL,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, 
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &DebugCallback, 
      .pUserData = NULL 
    };


    VkApplicationInfo AppInfo = { // uma struct definida pelo Vulkan
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, // tipo da estrutura = informações sobre aplicação
      .pNext = NULL, // usado para permitiar cadeia de extensões ou estruturas associadas, NULL = não há estruturas ou extensões
      .pApplicationName = pAppName, // define o nome da aplicação
      .pEngineName = "My Vulkan Studies", // define o nome da engine ou framework utilizado para criar a aplicação 
      .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0), // define a versão da engine 
      .apiVersion = VK_API_VERSION_1_0 // define a versão da api vulkan utilizada
    };
    // o uso do . é para inicialização designada usado para quando tem mts campos, para garantir que esta inicializando corretamente
    // ou para quando algum valor n sera iniciado 

    VkInstanceCreateInfo CreateInfo = { // utilizado para criar uma instancia do Vulkan
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // tipo da estrutura usada para criar uma instância Vulkan
      .pNext = &MessengerCreateInfo, 
      .flags = 0,  // Reservado para o futuro, deve ser 0 // usado para criar instancias com configurações especificas
      .pApplicationInfo = &AppInfo, // VKApllicationInfo
      .enabledLayerCount = (u32)(Layers.size()),  // define quantas funcionalidades adicionas podem ser ativadas 
      .ppEnabledLayerNames = Layers.data(), // um ponteiro para um array de strings com os nomes das camadas a serem adicionas, Layers.data() retorna o ponteiro
      .enabledExtensionCount = (u32)(Extensions.size()), // define quantas extensões serão habilitadas para a instancia (como suporte para diferentes tipos de renderização)
      .ppEnabledExtensionNames = Extensions.data() // ponteiro para os nomes das extensões 
    };

    VkResult res = vkCreateInstance(&CreateInfo, NULL, &m_instance); 
    // VkResult enum que identifica se a operação foi bem sucedida
    // vkCreateInstance cria a instancia Vulkan
    // primeiro parametro ponteiro para struct VkInstanceCreateInfo
    // segundo parametro ponteiro para um alocador de memória personalizado
    // terceiro parametro  ponteiro para a variável que receberá a instância Vulkan criada.
    CHECK_VK_RESULT(res, "Create instance");
    printf("Vulkan instance created\n");
  }


  void VulkanCore::CreateDebugCallback() {
    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = { 
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, // tipo para criar um mensageiro de depuração
      .pNext = NULL,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | // niveis de severidade das mensagens a serem capturadas
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, 
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | // tipos de mensagem a serem capturadas
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &DebugCallback, //ponteiro da função que sera chamada ao capturar uma mensagem
      .pUserData = NULL // ponteira para "dados de usuario", dados adicionais para passar para a função callback, como o estado da aplicação
    };

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE; // função ponteiro 
    vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if(!vkCreateDebugUtilsMessenger) {
      MY_ERROR0("Cannot find address of vkCreateDebugUtilsMessenger\n");
      exit(1);
    }

    VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, NULL, &m_debugMessenger); // crio a mensageria 
    CHECK_VK_RESULT(res, "debug utils messenger");

    printf("Debug utils messenger created\n");
  }


  void VulkanCore::CreateSurface() {
    VkResult res = glfwCreateWindowSurface(m_instance, m_pWindow, NULL, &m_surface);
    CHECK_VK_RESULT(res, "glfwCreateWindowSurface");

    printf("GLFW window surface created\n");
  }

  void VulkanCore::CreateDevice() {
    float qPriorities[] = {1.0f};

    VkDeviceQueueCreateInfo qInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0, // must be zero
      .queueFamilyIndex = m_queueFamily,
      .queueCount = 1,
      .pQueuePriorities = &qPriorities[0]
    };

    std::vector<const char*> DevExts = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
    };

    if (m_physDevices.Selected().m_features.geometryShader == VK_FALSE) {
      MY_ERROR0("The Geometry Shader is not supported!\n");
    }

    if (m_physDevices.Selected().m_features.tessellationShader == VK_FALSE) {
      MY_ERROR0("The Tessellation Shader is not supported!\n");
    }

    VkPhysicalDeviceFeatures DeviceFeatures = {0};
    DeviceFeatures.geometryShader = VK_TRUE;
    DeviceFeatures.tessellationShader = VK_TRUE;

    VkDeviceCreateInfo DeviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &qInfo,
      .enabledLayerCount = 0, // DEPRECATED
      .ppEnabledLayerNames = NULL, // DEPRECATED
      .enabledExtensionCount = (u32)DevExts.size(),
      .ppEnabledExtensionNames = DevExts.data(),
      .pEnabledFeatures = &DeviceFeatures
    };

    VkResult res = vkCreateDevice(m_physDevices.Selected().m_physDevice, &DeviceCreateInfo, NULL, &m_device);
    CHECK_VK_RESULT(res, "Create device\n");

    printf("\nDevice created\n");
  }

  static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& PresentModes) { //?
    for (int i = 0; i < PresentModes.size(); i++) {
      if(PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
        return PresentModes[i];
      }
    }

    // Fallback to FIFO which is always supported
    return VK_PRESENT_MODE_FIFO_KHR;
  }
  
  static u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR& Capabilities) { //?
    u32 RequestedNumImages = Capabilities.minImageCount + 1; //?

    int FinalNumImages = 0;

    if((Capabilities.maxImageCount > 0) && (RequestedNumImages > Capabilities.maxImageCount)) {
      FinalNumImages = Capabilities.maxImageCount;
    } else {
      FinalNumImages = RequestedNumImages;
    }

    return FinalNumImages;
  }

  static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& SurfaceFormats) {
    for (int i = 0; i < SurfaceFormats.size(); i++) {
      if((SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) && (SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) { //?
        return SurfaceFormats[i];
      }
    }
    return SurfaceFormats[0];
  }

  VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, VkImageViewType ViewType, u32 LayerCount, u32 mipLevels) { //?
    VkImageViewCreateInfo ViewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // Tipo da estrutura (sempre deve ser `VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO`)
      .pNext = NULL,  // Estrutura adicional, se necessário (geralmente é NULL)
      .flags = 0,  // Flags adicionais (geralmente 0)
      .image = Image,  // A imagem que será usada
      .viewType = ViewType,  // Tipo de view da imagem (2D, 3D, cubo, etc.)
      .format = Format,  // O formato da imagem (exemplo: `VK_FORMAT_R8G8B8A8_UNORM`)
      .components = {  // Configurações de swizzle (não altera os componentes de cor da imagem)
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,  // Usar o componente R da imagem
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,  // Usar o componente G da imagem
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,  // Usar o componente B da imagem
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,  // Usar o componente A da imagem
      },
      .subresourceRange = {  // Define o intervalo de subrecursos (mip levels e layers da imagem)
        .aspectMask = AspectFlags,  // Quais aspectos da imagem usar (cor, profundidade, stencil, etc.)
        .baseMipLevel = 0,  // Nível base de mipmap
        .levelCount = mipLevels,  // Número de níveis de mipmap
        .baseArrayLayer = 0,  // Camada base da imagem (para imagens 3D ou cubo)
        .layerCount = LayerCount  // Número de camadas (se for uma imagem 2D ou 3D com várias camadas)
      }
    };

    VkImageView ImageView;
    VkResult res = vkCreateImageView(Device, &ViewInfo, NULL, &ImageView); //?
    CHECK_VK_RESULT(res, "vkCreateImageView");
    return ImageView;
  }

  void VulkanCore::CreateSwapChain() {
    const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.Selected().m_surfaceCaps;

    u32 NumImages = ChooseNumImages(SurfaceCaps);

    const std::vector<VkPresentModeKHR>& PresentModes = m_physDevices.Selected().m_presentModes;
    VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);

    m_swapChainSurfaceFormat = ChooseSurfaceFormatAndColorSpace(m_physDevices.Selected().m_surfaceFormats);

    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = NULL,
      .flags = 0,
      .surface = m_surface,
      .minImageCount = NumImages,
      .imageFormat = m_swapChainSurfaceFormat.format,
      .imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
      .imageExtent = SurfaceCaps.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount =1,
      .pQueueFamilyIndices = &m_queueFamily,
      .preTransform = SurfaceCaps.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = PresentMode,
      .clipped = VK_TRUE
    };

    VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
    CHECK_VK_RESULT(res, "vkCreateSwapchainKHR\n");

    printf("Swap chain created\n");

    uint NumSwapChainImages = 0;
    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
    CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
    assert(NumImages <= NumSwapChainImages);

    printf("Requested %d images, created %d images \n", NumImages, NumSwapChainImages);

    m_images.resize(NumSwapChainImages);
    m_imageViews.resize(NumSwapChainImages);

    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, m_images.data());
    CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");

    int LayerCount = 1;
    int MipLevels = 1;
    for(u32 i = 0; i< NumSwapChainImages; i++) {
      m_imageViews[i] = CreateImageView(m_device, m_images[i], m_swapChainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, LayerCount, MipLevels);
    }
  }

  void VulkanCore::CreateCommandBufferPool() {
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .queueFamilyIndex = m_queueFamily
    };

    VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, NULL, &m_cmdBufPool);
    CHECK_VK_RESULT(res, "vkCreateCommandPool\n");

    printf("Command buffer pool created\n");
  }

  void VulkanCore::CreateCommandBuffers(u32 count, VkCommandBuffer* cmdBufs) {
    VkCommandBufferAllocateInfo cmdBufAllocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = NULL,
      .commandPool = m_cmdBufPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = count
    };

    VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, cmdBufs);
    CHECK_VK_RESULT(res, "vkAllocateCommandBuffers\n");

    printf("%d command buffers created\n", count);
  }

  void VulkanCore::FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs) {
    m_queue.WaitIdle();
    vkFreeCommandBuffers(m_device, m_cmdBufPool, Count, pCmdBufs);
  }

  VkRenderPass VulkanCore::CreateSimpleRenderPass() {
    VkAttachmentDescription AttachDesc = {
      .flags = 0,
      .format = m_swapChainSurfaceFormat.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference AttachRef = {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription SubpassDesc = {
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = NULL,
      .colorAttachmentCount = 1,
      .pColorAttachments = &AttachRef,
      .pResolveAttachments = NULL,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = NULL
    };

    VkRenderPassCreateInfo RenderPassCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .attachmentCount = 1,
      .pAttachments = &AttachDesc,
      .subpassCount = 1,
      .pSubpasses = &SubpassDesc,
      .dependencyCount = 0,
      .pDependencies = NULL
    };

    VkRenderPass RenderPass;

    VkResult res = vkCreateRenderPass(m_device, &RenderPassCreateInfo, NULL, &RenderPass);
    CHECK_VK_RESULT(res, "vkCreateRenderPass\n");

    printf("Created a simple render pass\n");

    return RenderPass;
  }

  std::vector<VkFramebuffer> VulkanCore::CreateFramebuffers(VkRenderPass RenderPass) const {

    std::vector<VkFramebuffer> frameBuffers;
    frameBuffers.resize(m_images.size());

    int WindowWidth, WindowHeight;
    GetFramebufferSize(WindowWidth, WindowHeight);

    VkResult res;

    for (uint i = 0; i < m_images.size(); i++) {
      VkFramebufferCreateInfo fbCreateInfo = {};
      fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      fbCreateInfo.renderPass = RenderPass;
      fbCreateInfo.attachmentCount = 1;
      fbCreateInfo.pAttachments = &m_imageViews[i];
      fbCreateInfo.width = WindowWidth;
      fbCreateInfo.height = WindowHeight;
      fbCreateInfo.layers = 1;

      res = vkCreateFramebuffer(m_device, &fbCreateInfo, NULL, &frameBuffers[i]);
      CHECK_VK_RESULT(res, "vkCreateFramebuffer\n");
    }

    printf("Framebuffers created\n");

    return frameBuffers;
  }

  void VulkanCore::DestroyFramebuffers(std::vector<VkFramebuffer>& Framebuffers){
    for (int i = 0; i < Framebuffers.size(); i++) {
      vkDestroyFramebuffer(m_device, Framebuffers[i], NULL);
    }
  }

  BufferAndMemory VulkanCore::CreateVertexBuffer(const void* pVertices, size_t Size) {
    // Step 1: create the staging buffer
    VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    BufferAndMemory StagingVB = CreateBuffer(Size, Usage, MemProps);

    //Step 2: map the memory of the stage buffer
    void* pMem = NULL;
    VkDeviceSize Offset = 0;
    VkMemoryMapFlags Flags = 0;
    VkResult res = vkMapMemory(m_device, StagingVB.m_mem, Offset, StagingVB.m_allocationSize, Flags, &pMem);
    CHECK_VK_RESULT(res, "VkMapMemory\n");

    //Step 3: copy the vertices to the staging buffer
    memcpy(pMem, pVertices, Size);

    //Step 4: unmap/release the mapped memory
    vkUnmapMemory(m_device, StagingVB.m_mem);

    //Step 5: create the final buffer
    Usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    BufferAndMemory VB = CreateBuffer(Size, Usage, MemProps);

    //Step 6: copy the staging buffer to the final buffer
    CopyBuffer(VB.m_buffer, StagingVB.m_buffer, Size);

    //Step 7: release the resources of the staging buffer
    StagingVB.Destroy(m_device);

    return VB;
  }

  BufferAndMemory VulkanCore::CreateUniformBuffer(int Size) {
    BufferAndMemory Buffer;

    Buffer = CreateBuffer(Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    return Buffer;
  }

  BufferAndMemory VulkanCore::CreateBuffer (VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties) {
    VkBufferCreateInfo vbCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = Size,
      .usage = Usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    BufferAndMemory Buf;

    // Step 1: create a buffer
    VkResult res = vkCreateBuffer(m_device, &vbCreateInfo, NULL, &Buf.m_buffer);
    CHECK_VK_RESULT(res, "vkCreateBuffer\n");
    printf("Buffer created\n");

    // Step 2: get the buffer memory requirements
    VkMemoryRequirements MemReqs = {};
    vkGetBufferMemoryRequirements(m_device, Buf.m_buffer, &MemReqs);
    printf("Buffer requires %d bytes\n", (int)MemReqs.size);

    Buf.m_allocationSize = MemReqs.size;

    // Step 3: get the memory type index
    u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
    printf("Memory type index %d\n", MemoryTypeIndex);

    //Step 4: allocate memory
    VkMemoryAllocateInfo MemAllocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = MemReqs.size,
      .memoryTypeIndex = MemoryTypeIndex
    };

    res = vkAllocateMemory(m_device, &MemAllocInfo, NULL, &Buf.m_mem);
    CHECK_VK_RESULT(res, "vkAllocateMemory error %d\n");

    // Step 5: bind memory
    res = vkBindBufferMemory(m_device, Buf.m_buffer, Buf.m_mem, 0);
    CHECK_VK_RESULT(res, "vkBindBufferMemory error\n");

    return Buf;
  }

  void VulkanCore::CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size) {
    BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferCopy BufferCopy = {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = Size
    };

    vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &BufferCopy);

    vkEndCommandBuffer(m_copyCmdBuf);
  
    m_queue.SubmitSync(m_copyCmdBuf);

    m_queue.WaitIdle();
  }

  u32 VulkanCore::GetMemoryTypeIndex(u32 MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags) {
    const VkPhysicalDeviceMemoryProperties& MemProps = m_physDevices.Selected().m_memProps;

    for (uint i = 0; i< MemProps.memoryTypeCount; i++) {
      const VkMemoryType& MemType = MemProps.memoryTypes[i];
      uint CurBitMask = (1 << i);
      bool IsCurMemTypeSupported = (MemTypeBitsMask & CurBitMask);
      bool HasRequiredMemProps((MemType.propertyFlags & ReqMemPropFlags) == ReqMemPropFlags);
    
      if(IsCurMemTypeSupported && HasRequiredMemProps) {
        return i;
      }
    }

    printf("Cannot find memory type for type %x requested mem props %x\n", MemTypeBitsMask, ReqMemPropFlags);
    exit(1);
    return -1;
  }

  void BufferAndMemory::Destroy(VkDevice Device) {
    if(m_mem) {
      vkFreeMemory(Device, m_mem, NULL);
    }

    if(m_buffer) {
      vkDestroyBuffer(Device, m_buffer, NULL);
    }
  }

  void VulkanCore::GetFramebufferSize(int& Width, int& Height) const {
    glfwGetWindowSize(m_pWindow, &Width, &Height);
  }

  std::vector<BufferAndMemory> VulkanCore::CreateUniformBuffers(size_t DataSize)  {
    std::vector<BufferAndMemory> UniformBuffers;

    UniformBuffers.resize(m_images.size());

    for (int i = 0; i < UniformBuffers.size(); i++) {
      UniformBuffers[i] = CreateUniformBuffer((int)DataSize);
    }

    return UniformBuffers;
  }

  void BufferAndMemory::Update(VkDevice Device, const void* pData, size_t Size, VkDeviceSize Offset) {
    void* pMem = NULL;
    VkResult res = vkMapMemory(Device, m_mem, Offset, Size, 0, &pMem);
    CHECK_VK_RESULT(res, "vkMapMemory"); 
    memcpy(pMem, pData, Size);
    vkUnmapMemory(Device, m_mem);
  }


  ImageAndMemory VulkanCore::LoadTexture(const char* filename) {
    ImageAndMemory textureImage;
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels) {
      throw std::runtime_error("Failed to load texture image!");
    }

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    // Staging buffer --- buffer temporario
    BufferAndMemory stagingBuffer = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Update(m_device, pixels, (size_t)imageSize, 0);
    stbi_image_free(pixels);

    //Criar imagem
    VkImageCreateInfo imageInfo = {
      .sType= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .extent = { (uint32_t)texWidth, (uint32_t)texHeight, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    
    VkResult res = vkCreateImage(m_device, &imageInfo, nullptr, &textureImage.m_image);
    CHECK_VK_RESULT(res, "vkCreateImage");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, textureImage.m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    res = vkAllocateMemory(m_device, &allocInfo, nullptr, &textureImage.m_mem);
    CHECK_VK_RESULT(res, "vkAllocateMemory (texture)");

    res = vkBindImageMemory(m_device, textureImage.m_image, textureImage.m_mem, 0);
    CHECK_VK_RESULT(res, "vkBindImageMemory (texture)");

    //Transição e cópia
    TransitionImageLayout(textureImage.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(stagingBuffer.m_buffer, textureImage.m_image, (uint32_t)texWidth, (uint32_t)texHeight);

    TransitionImageLayout(textureImage.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    stagingBuffer.Destroy(m_device);

    textureImage.m_view = CreateImageView(m_device, textureImage.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
    textureImage.m_sampler = CreateTextureSampler();

    return textureImage;
  }

  void VulkanCore::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer cmd = m_copyCmdBuf;
    BeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(cmd);
    m_queue.SubmitSync(cmd);
  }

  void VulkanCore::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer cmd = m_copyCmdBuf;
    BeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
      }, 
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1}
    };

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    vkEndCommandBuffer(cmd);
    m_queue.SubmitSync(cmd);
  }

  VkImageView VulkanCore::CreateTextureImageView(VkImage image) {
    return CreateImageView(
      m_device,
      image,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_ASPECT_COLOR_BIT,
      VK_IMAGE_VIEW_TYPE_2D,
      1, // layer count
      1 // mip levels
    );
  }

  VkSampler VulkanCore::CreateTextureSampler() {
    VkSamplerCreateInfo samplerInfo = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,

      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,

      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = 16,
      
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler;
    VkResult res = vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler);
    CHECK_VK_RESULT(res, "vkCreate?Sampler");

    return sampler;
  }

  VkDescriptorImageInfo VulkanCore::MakeDescriptorImageInfo(const ImageAndMemory& imageAndMemory) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageAndMemory.m_view;
    imageInfo.sampler = imageAndMemory.m_sampler;
    return imageInfo;
  }

}