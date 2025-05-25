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

const int TARGET_FPS = 60; // para controlar o framerate
const std::chrono::duration<double> TARGET_FRAME_DURATION(1.0 / TARGET_FPS); // para controlar o framerate


#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
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
      m_mesh.Destroy(m_device);

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
      struct Vertex {
        Vertex(const glm::vec3& p, const glm::vec2& t) {
          Pos = p;
          Tex = t;
        }

        glm::vec3 Pos;
        glm::vec2 Tex;
      };

      std::vector<Vertex> Vertices = {
        Vertex({-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}), // top left
        Vertex({0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}), // top right
        Vertex({0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}),
        
        Vertex({-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}),
        Vertex({-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f})
      };

      std::vector<Vertex> Triangulo = {
        Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
        Vertex({-0.9f, -1.0f, 0.0f}, {1.0f, 1.0f}),
        Vertex({-0.9f, -0.9f, 0.0f}, {0.0f, 1.0f})
      };

      m_mesh.m_vertexBufferSize = sizeof(Vertices[0]) * Vertices.size();
      m_mesh.m_vb = m_vkCore.CreateVertexBuffer(Vertices.data(), m_mesh.m_vertexBufferSize);

      m_mesh2.m_vertexBufferSize = sizeof(Triangulo[0]) * Triangulo.size();
      m_mesh2.m_vb = m_vkCore.CreateVertexBuffer(Triangulo.data(), m_mesh2.m_vertexBufferSize);
    }

    struct UniformData {
      glm::mat4 WVP;
    };

    void CreateUniformBuffers() {
      m_uniformBuffers= m_vkCore.CreateUniformBuffers(sizeof(UniformData));
      m_uniformBuffers2= m_vkCore.CreateUniformBuffers(sizeof(UniformData));
    }

    void CreateShaders() {
      m_vs = MyVK::CreateShaderModuleFromText(m_device, "test.vert");
      m_fs = MyVK::CreateShaderModuleFromText(m_device, "test.frag");
    }

    void CreatePipeline() {
      m_pPipeline = new MyVK::GraphicsPipeline(m_device, m_pWindow, m_renderPass, m_vs, m_fs, &m_mesh, m_numImages, m_uniformBuffers, sizeof(UniformData));
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

        // m_pPipeline->BindWithSet(m_numImages, &m_mesh, m_uniformBuffers, sizeof(UniformData));
        m_pPipeline->Bind(m_cmdBufs[i], i);
        u32 VertexCount = 6;
        u32 InstaceCout = 1;
        u32 FirstVertex = 0;
        u32 FirstInstance = 0;
        vkCmdDraw(m_cmdBufs[i], VertexCount, InstaceCout, FirstVertex, FirstInstance);

       
        // m_pPipeline->BindWithSet(m_numImages, &m_mesh2, m_uniformBuffers2, sizeof(UniformData));
        m_pPipeline->Bind(m_cmdBufs[i], i);
        vkCmdDraw(m_cmdBufs[i], 3 , InstaceCout, FirstVertex, FirstInstance);

        vkCmdEndRenderPass(m_cmdBufs[i]);

        VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
        CHECK_VK_RESULT(res, "vkEndCommandBufffer\n");
      }

      printf("Command buffers recorded\n");
    }

    void UpdateUniformBuffers(uint32_t ImageIndex) {
      static float foo = 0.0f;
      glm::mat4 Rotate = glm::mat4(1.0);
      glm::mat4 Translate = glm::mat4(1.0);
      static float movement = 0.004f;
      Translate = glm::translate(Translate, glm::vec3(0.0f, foo, 1.0f));
      // Rotate = glm::rotate(Rotate, glm::radians(foo), glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)));
      foo += movement;
      if (foo >= 1.5f) {
        movement = -movement;
      } else if (foo <= -1.5f) {
        movement = -movement;
      }
      m_uniformBuffers[ImageIndex].Update(m_device, &Translate, sizeof(Translate));
      m_uniformBuffers2[ImageIndex].Update(m_device, &Rotate, sizeof(Rotate));
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
    MyVK::SimpleMesh m_mesh;
    MyVK::SimpleMesh m_mesh2;
    std::vector<MyVK::BufferAndMemory> m_uniformBuffers;
    std::vector<MyVK::BufferAndMemory> m_uniformBuffers2;
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