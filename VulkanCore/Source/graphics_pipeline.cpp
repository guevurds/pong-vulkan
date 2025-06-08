#include <stdio.h>
#include <array>

#include "my_types.h"
#include "my_util.h"
#include "my_vulkan_util.h"
#include "my_vulkan_graphics_pipeline.h"

namespace MyVK{
  GraphicsPipeline::GraphicsPipeline(VkDevice Device, GLFWwindow* pWindow, VkRenderPass RenderPass, VkShaderModule vs, VkShaderModule fs, int NumImages, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize, std::vector<VkDescriptorImageInfo>& ImageInfos) {
    m_device = Device;

    CreateDescriptorSets(NumImages, UniformBuffers, UniformDataSize, ImageInfos); 

    VkPipelineShaderStageCreateInfo ShaderStageCreateInfo[2] = {
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vs,
        .pName = "main"
      },
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fs,
        .pName = "main"
      }
    };

    // Descreve como cada vértice está empacotado na struct Vertex:
      VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
      };

    VkVertexInputAttributeDescription attributeDescriptions[2] = {};
    // location = 0 → vec3 Pos
    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(Vertex, Pos);
    // location = 1 → vec2 Tex
    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(Vertex, Tex);


    VkPipelineVertexInputStateCreateInfo VertexInputInfo {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount = 2,
      .pVertexAttributeDescriptions = attributeDescriptions
    };

    VkPipelineInputAssemblyStateCreateInfo PipelineIACreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE
    };

    int WindowWidth, WindowHeight;
    glfwGetWindowSize(pWindow, &WindowWidth, &WindowHeight);

    VkViewport VP = {
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)WindowWidth,
      .height = (float)WindowHeight,
      .minDepth = 0.0f,
      .maxDepth = 1.0f
    };

    VkRect2D Scissor {
      .offset = {
        .x = 0,
        .y = 0,
      },
      .extent = {
        .width = (u32)WindowWidth,
        .height = (u32)WindowHeight
      }
    };

    VkPipelineViewportStateCreateInfo VPCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &VP,
      .scissorCount = 1,
      .pScissors = &Scissor
    };

    VkPipelineRasterizationStateCreateInfo RastCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo PipelineMSCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f
    };

    VkPipelineColorBlendAttachmentState BlendAttachState = {
      .blendEnable = VK_TRUE,  // antes estava VK_FALSE
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo BlendCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &BlendAttachState
    };


    VkPipelineLayoutCreateInfo LayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &m_descriptorSetLayout
    };

    VkResult res = vkCreatePipelineLayout(m_device, &LayoutInfo, NULL, &m_pipelineLayout);
    CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");

    VkGraphicsPipelineCreateInfo PipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = ARRAY_SIZE_IN_ELEMENTS(ShaderStageCreateInfo),
      .pStages = &ShaderStageCreateInfo[0],
      .pVertexInputState = &VertexInputInfo,
      .pInputAssemblyState = &PipelineIACreateInfo,
      .pViewportState = &VPCreateInfo,
      .pRasterizationState = &RastCreateInfo,
      .pMultisampleState = &PipelineMSCreateInfo,
      .pColorBlendState = &BlendCreateInfo,
      .layout = m_pipelineLayout,
      .renderPass = RenderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1
    };

    res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &PipelineInfo, NULL, &m_pipeline);
    CHECK_VK_RESULT(res, "vkCreateGraphicsPipelines\n");

    printf("Graphics pipeline created\n");
    printf("Pipeline pointer: %p\n", m_pipeline);
  }

  GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);
    vkDestroyPipeline(m_device, m_pipeline, NULL);
  }

  void GraphicsPipeline::Bind(VkCommandBuffer CmdBuf, int ImageIndex, uint32_t dynamicOffset) {
    vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    if(m_descriptorSets.size() > 0) {
      vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 
        0, // firstSet
        1, // descriptorSetCount
        &m_descriptorSets[ImageIndex],
        1, // dynamicOffsetCount
        &dynamicOffset); //pDynamicOffsets
    }
  }

  void GraphicsPipeline::CreateDescriptorSets(int NumImages, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize, std::vector<VkDescriptorImageInfo>& ImageInfos) {
    CreateDescriptorPool(NumImages);
    CreateDescriptorSetLayout(UniformBuffers, UniformDataSize);
    AllocateDescriptorSets(NumImages);
    UpdateDescriptorSets(NumImages, UniformBuffers, UniformDataSize, ImageInfos);
  }

  void GraphicsPipeline::CreateDescriptorPool(int NumImages) {
    printf("numero de images: %i", NumImages);
    std::vector<VkDescriptorPoolSize> PoolSizes(1);

    // PoolSizes[0] = {
    //   .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //   .descriptorCount = (u32)(NumImages)
    // };

    PoolSizes[0] = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
      .descriptorCount = (u32)(NumImages)
    };


    VkDescriptorPoolCreateInfo PoolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = 0,
      .maxSets = (u32)NumImages,
      .poolSizeCount = (u32)PoolSizes.size(),
      .pPoolSizes = PoolSizes.data()
    };

    VkResult res = vkCreateDescriptorPool(m_device, &PoolInfo, NULL, &m_descriptorPool);
    CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
    printf("Descriptor pool created\n");
  }

  void GraphicsPipeline::CreateDescriptorSetLayout(std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize) {
    std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

    // VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_VB = {
    //   .binding = 0,
    //   .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //   .descriptorCount = 1,
    //   .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    // };

    VkDescriptorSetLayoutBinding VertexShaderLayoutBinding_Uniform = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };

    VkDescriptorSetLayoutBinding FragmentShaderLayoutBinding_Texture = {
      .binding = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 3, //max_textures
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr
    };


    if (UniformBuffers.size() > 0) {
      LayoutBindings.push_back(VertexShaderLayoutBinding_Uniform);

      LayoutBindings.push_back(FragmentShaderLayoutBinding_Texture);
    }
    
    // LayoutBindings.push_back(VertexShaderLayoutBinding_VB);

    VkDescriptorSetLayoutCreateInfo LayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = NULL,
      .flags = 0, // reserved - must be zero
      .bindingCount = (u32)LayoutBindings.size(),
      .pBindings = LayoutBindings.data()
    };

    VkResult res = vkCreateDescriptorSetLayout(m_device, &LayoutInfo, NULL, &m_descriptorSetLayout);
    CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
  }

  void GraphicsPipeline::AllocateDescriptorSets(int NumImages) {
    std::vector<VkDescriptorSetLayout> Layouts(NumImages, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo AllocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = NULL,
      .descriptorPool = m_descriptorPool,
      .descriptorSetCount = (u32)NumImages,
      .pSetLayouts = Layouts.data()
    };

    m_descriptorSets.resize(NumImages);

    VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, m_descriptorSets.data());
    CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");
  }

  void GraphicsPipeline::UpdateDescriptorSets(int NumImages, std::vector<BufferAndMemory>& UniformBuffers, int UniformDataSize
    , std::vector<VkDescriptorImageInfo>& ImageInfos) {

    std::vector<VkWriteDescriptorSet> WriteDescriptorSet;

    for (int i = 0; i < NumImages; ++i) {
      assert(ImageInfos[0].imageView != VK_NULL_HANDLE);
      assert(ImageInfos[0].sampler != VK_NULL_HANDLE);
      assert(m_descriptorSets[i] != VK_NULL_HANDLE);
    }

    // printf("numImages %d\n", NumImages);
    // printf("ImageInfos length %d\n", ImageInfos.size());

    for (size_t i = 0; i< NumImages; i++) {

      VkDescriptorBufferInfo BufferInfo_Uniform = {
        .buffer = UniformBuffers[i].m_buffer, 
        .offset = 0,
        .range = (VkDeviceSize)UniformDataSize
      };

      WriteDescriptorSet.push_back(
        VkWriteDescriptorSet{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = m_descriptorSets[i],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
          .pBufferInfo = &BufferInfo_Uniform

        }
      );

      if (ImageInfos[0].imageView == VK_NULL_HANDLE || ImageInfos[0].sampler == VK_NULL_HANDLE) {
        printf("deu ruim no UpdateDescriptorSets");
      }

      WriteDescriptorSet.push_back(
        VkWriteDescriptorSet{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = m_descriptorSets[i],
          .dstBinding = 1,
          .dstArrayElement = 0,
          .descriptorCount = 3,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &ImageInfos[0]
        }
      );
    }

    vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
  }
}