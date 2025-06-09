#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <chrono> // para controlar o framerate
#include <thread> // para controlar o framerate

#include "my_vulkan_util.h"
#include "my_vulkan_core.h"
#include "my_vulkan_wrapper.h"
#include "my_vulkan_shader.h"
#include "my_vulkan_graphics_pipeline.h"
#include "my_vulkan_simple_mash.h"
#include "objects_quantity.h"
#include "load_font.h"

#include "objects.h"

const int TARGET_FPS = 60; // para controlar o framerate
const std::chrono::duration<double> TARGET_FRAME_DURATION(1.0 / TARGET_FPS); // para controlar o framerate

int max_tex;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  
  switch (key) {
    case GLFW_KEY_DOWN:
      if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        key_pressed[0] = true;
        break;
      }

      key_pressed[0] = false;
      break;
    case GLFW_KEY_UP:
      if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        key_pressed[1] = true;
        break;
      }

      key_pressed[1] = false;
      break;
    default:
      break;
  }
}


class VulkanApp {
  public: 
    VulkanApp() {}

    ~VulkanApp() {
      m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
      m_vkCore.DestroyFramebuffers(m_frameBuffers);
      vkDestroyShaderModule(m_device, m_vs, NULL);
      vkDestroyShaderModule(m_device, m_fs, NULL);
      delete m_pPipeline;
      vkDestroyRenderPass(m_device, m_renderPass, NULL);

      // for (int i = 0; i < m_meshs.size(); i++) {
      //   m_meshs[i].Destroy(m_device);
      // }

      for (int i = 0; i < m_uniformBuffers.size(); i++) {
        m_uniformBuffers[i].Destroy(m_device);
      }
    }

    void Init(const char* pAppName, GLFWwindow* pWindow) {
      m_pWindow = pWindow;
      m_vkCore.Init(pAppName, pWindow);
      m_device = m_vkCore.GetDevice();
      m_numImages = m_vkCore.GetNumImages();
      m_pQueue = m_vkCore.GetQueue();
      m_renderPass = m_vkCore.CreateSimpleRenderPass();
      m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);
      max_tex = Scene::Object::getObjectsNumber();
      CreateShaders();
      CreateVertexBuffer();
      CreateUniformBuffers();
      CreatePipeline();
      CreateCommandBuffers();
      RecordCommandBuffers();
    }

    void RenderScene() {
      u32 ImageIndex = m_pQueue->AcquireNextImage();

      UpdateUniformBuffers(ImageIndex);

      m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

      m_pQueue->Present(ImageIndex);
    }
  
  private:
    void CreateCommandBuffers() {
      m_cmdBufs.resize(m_numImages);
      m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());

      printf("Created command buffers\n");
    }

    void CreateVertexBuffer() {
     

      // std::vector<std::vector<Vertex>> objects{
      //   { //Bot
      //     Vertex({-0.95f, -1.0f, 0.0f}, {0.0f, 0.0f}), // top left
      //     Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}), // top right
      //     Vertex({-0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
          
      //     Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
      //     Vertex({-0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
      //     Vertex({-1.0f, -0.7f, 0.0f}, {0.0f, 1.0f})
      //   },
      //   { //Player
      //     Vertex({0.95f, -1.0f, 0.0f}, {0.0f, 0.0f}), // top left
      //     Vertex({1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}), // top right
      //     Vertex({0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
          
      //     Vertex({1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
      //     Vertex({0.95f, -0.7f, 0.0f}, {1.0f, 1.0f}),
      //     Vertex({1.0f, -0.7f, 0.0f}, {0.0f, 1.0f})
      //   }
      // };

      // m_meshs.resize(objects.size());

      // for (int i=0; i<objects.size(); i++ ) {
      //   m_meshs[i].m_vertexBufferSize = sizeof(objects[i][0]) * objects[i].size();
      //   m_meshs[i].m_vb = m_vkCore.CreateVertexBuffer(objects[i].data(), m_meshs[i].m_vertexBufferSize);
      // }

      Scene::Object::createVertexBufferAll(m_device, m_vkCore);
    }

    void CreateUniformBuffers() {
      m_uniformBuffers= m_vkCore.CreateUniformBuffers(Scene::Object::getObjectsNumber() * sizeof(UniformData));
      MyVK::ImageAndMemory tex1 = m_vkCore.LoadTexture("Textures/azul.png");
      MyVK::ImageAndMemory tex2 = m_vkCore.LoadTexture("Textures/vermelho_verde.png");
      std::vector<unsigned char> rgba = font_roboto.LoadFontAtlas();
      MyVK::ImageAndMemory font = m_vkCore.LoadTextureFromMemory(rgba.data(), 512, 512, VK_FORMAT_R8G8B8A8_UNORM);

      m_textureInfos.push_back(m_vkCore.MakeDescriptorImageInfo(font));

      m_textureInfos.push_back(m_vkCore.MakeDescriptorImageInfo(tex1));
      m_textureInfos.push_back(m_vkCore.MakeDescriptorImageInfo(tex2));
      m_textureInfos.push_back(m_vkCore.MakeDescriptorImageInfo(tex2));
    }

    void CreateShaders() {
      m_vs = MyVK::CreateShaderModuleFromText(m_device, "test.vert");
      m_fs = MyVK::CreateShaderModuleFromText(m_device, "test.frag");
    }

    void CreatePipeline() {
      m_pPipeline = new MyVK::GraphicsPipeline(m_device, m_pWindow, m_renderPass, m_vs, m_fs, m_numImages, m_uniformBuffers, sizeof(UniformData), m_textureInfos);
    }

    void RecordCommandBuffers() {
      VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 0.0f};
      VkClearValue ClearValue;
      ClearValue.color = ClearColor;

      VkRenderPassBeginInfo RenderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = m_renderPass,
        .renderArea = {
          .offset = {
            .x = 0,
            .y = 0
          },
          .extent = {
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT
          }
        },
        .clearValueCount = 1,
        .pClearValues = &ClearValue
      };
      
      for (uint i = 0; i < m_cmdBufs.size(); i++ ) {
   
        MyVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        
        RenderPassBeginInfo.framebuffer = m_frameBuffers[i];
        vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        Scene::Object::recordCommandBufferAll(m_pPipeline, m_cmdBufs, i);

        // for (int mesh=0; mesh < m_meshs.size(); mesh++) {
        //   m_pPipeline->Bind(m_cmdBufs[i], i, mesh * sizeof(UniformData));
        //   VkDeviceSize vbOffset = 0;
        //   vkCmdBindVertexBuffers(
        //   m_cmdBufs[i],
        //   /* firstBinding = */ 0,                    // conforme o binding que você definiu no pipeline
        //   /* bindingCount = */ 1,
        //   &m_meshs[mesh].m_vb.m_buffer,                       // VkBuffer do primeiro mesh
        //   &vbOffset
        // );

        // u32 VertexCount = 6;
        // u32 InstaceCout = 1;
        // u32 FirstVertex = 0;
        // u32 FirstInstance = 0;

        // vkCmdDraw(m_cmdBufs[i], VertexCount, InstaceCout, FirstVertex, FirstInstance);

        // }

        vkCmdEndRenderPass(m_cmdBufs[i]);

        VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
        CHECK_VK_RESULT(res, "vkEndCommandBufffer\n");
      }

      printf("Command buffers recorded\n");
    }

    void UpdateUniformBuffers(uint32_t ImageIndex) {
      // static float foo = 0.0f;
      // glm::mat4 TranslatePlayer = glm::mat4(1.0);
      // glm::mat4 Translate = glm::mat4(1.0);

      // static float movementPlayer = 0.0f;

      // float move_value = 0.006f;

      // static float movement = 0.006f;
      // Translate = glm::translate(Translate, glm::vec3(0.0f, foo, 1.0f));
      // TranslatePlayer = glm::translate(TranslatePlayer, glm::vec3(0.0f, movementPlayer, 1.0f));
      
      // // Rotate = glm::rotate(Rotate, glm::radians(foo), glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)));
      // foo += movement;
      // if (foo >= 1.7f) {
      //   movement = -movement;
      // } else if (foo <= 0.0f) {
      //   movement = -movement;
      // }

      // if(key_pressed[0]) { //down
      //   if(movementPlayer <= 1.7f) {
      //     movementPlayer += move_value;
      //   }
      // }

      //  if(key_pressed[1]) { //up
      //   if(movementPlayer >= 0.0f) {
      //     movementPlayer -= move_value;
      //   }
      // }

      // m_uniformBuffers[ImageIndex].Update(m_device, &Translate, sizeof(Translate), 0);
      // m_uniformBuffers[ImageIndex].Update(m_device, &TranslatePlayer, sizeof(TranslatePlayer), sizeof(UniformData));

      Scene::Object::updateAll(m_uniformBuffers[ImageIndex]);
    }

    GLFWwindow* m_pWindow = NULL;
    MyVK::VulkanCore m_vkCore;
    MyVK::VulkanQueue* m_pQueue = NULL;
    VkDevice m_device = NULL;
    int m_numImages = 0;
    std::vector<VkCommandBuffer> m_cmdBufs;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_frameBuffers;
    VkShaderModule m_vs;
    VkShaderModule m_fs;
    MyVK::GraphicsPipeline* m_pPipeline = NULL;
    std::vector<MyVK::SimpleMesh> m_meshs;
    std::vector<MyVK::BufferAndMemory> m_uniformBuffers;
    std::vector<VkDescriptorImageInfo> m_textureInfos;
    
};

#define APP_NAME "Pong Vulkan"

int main(int argc, char* argv[]) {
  if(!glfwInit()) {
    return 1;
  }

  if(!glfwVulkanSupported()) {
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, NULL, NULL);

  if(!pWindow) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(pWindow, GLFW_KeyCallback);

  VulkanApp App;
  App.Init(APP_NAME, pWindow);

  using clock = std::chrono::high_resolution_clock; // para framerate

  while(!glfwWindowShouldClose(pWindow)) {
    auto frameStart = clock::now(); // para framerate

    App.RenderScene();
    glfwPollEvents();

    auto frameEnd = clock::now(); // para framerate
    std::chrono::duration<double> elapsed = frameEnd - frameStart; // para framerate

    if (elapsed < TARGET_FRAME_DURATION) {
      std::this_thread::sleep_for(TARGET_FRAME_DURATION - elapsed);
    }
  }

  glfwTerminate();

  return 0;
}