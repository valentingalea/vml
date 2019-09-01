#include <vector>
#include <iostream>
#include "graphics_context.hpp"
#include <intrin.h>
#include <fstream>
#include <iterator>
#include <string>

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

static vulkan_context_t g_context;

template <typename ...T> void vulkan_error(T ...items)
{
    (std::cout << ... << items) << std::endl;
}

uint32_t
find_memory_type_according_to_requirements(VkMemoryPropertyFlags properties, VkMemoryRequirements memory_requirements)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(g_context.gpu.hardware, &mem_properties);

    for (uint32_t i = 0
             ; i < mem_properties.memoryTypeCount
             ; ++i)
    {
        if (memory_requirements.memoryTypeBits & (1 << i)
            && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return(i);
        }
    }
	    
    vulkan_error("Failed to find suitable memory type");
    assert(false);
    return(0);
}

void allocate_gpu_memory(VkMemoryPropertyFlags properties,
                         VkMemoryRequirements memory_requirements,
                         VkDeviceMemory *dest_memory)
{
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type_according_to_requirements(properties, memory_requirements);

    if (vkAllocateMemory(g_context.gpu.logical_device,
                              &alloc_info,
                              nullptr,
                              dest_memory) != VK_SUCCESS)
    {
        vulkan_error("Failed to allocate GPU memory");
        assert(0);
    }
}

render_pass_dependency_t make_render_pass_dependency(int32_t src_index, VkPipelineStageFlags src_stage, uint32_t src_access,
                                                     int32_t dst_index, VkPipelineStageFlags dst_stage, uint32_t dst_access)
{
    return render_pass_dependency_t{ src_index, src_stage, src_access, dst_index, dst_stage, dst_access };
}

inline VkAttachmentDescription make_attachment_description(VkFormat format,
                                                           VkSampleCountFlagBits samples,
                                                           VkAttachmentLoadOp load, VkAttachmentStoreOp store,
                                                           VkAttachmentLoadOp stencil_load, VkAttachmentStoreOp stencil_store,
                                                           VkImageLayout initial_layout, VkImageLayout final_layout)
{
    VkAttachmentDescription desc = {};
    desc.format	= format;
    desc.samples = samples;
    desc.loadOp	= load;
    desc.storeOp = store;
    desc.stencilLoadOp = stencil_load;
    desc.stencilStoreOp	= stencil_store;
    desc.initialLayout = initial_layout;
    desc.finalLayout = final_layout;
    return(desc);
}

inline VkAttachmentReference make_attachment_reference(uint32_t attachment, VkImageLayout layout)
{
    VkAttachmentReference ref = {};
    ref.attachment = attachment;
    ref.layout = layout;
    return(ref);
}

inline VkSubpassDescription make_subpass_description(VkAttachmentReference *color_refs, uint32_t color_count,
                                                     VkAttachmentReference *depth, 
                                                     VkAttachmentReference *input_refs, uint32_t input_count)
{
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = color_count;
    subpass.pColorAttachments = color_refs;
    subpass.pDepthStencilAttachment = depth;
    subpass.inputAttachmentCount = input_count;
    subpass.pInputAttachments = input_refs;
    return(subpass);
}

inline VkSubpassDependency make_subpass_dependency(uint32_t src_subpass,
                                                   uint32_t dst_subpass,
                                                   VkPipelineStageFlags src_stage,
                                                   uint32_t src_access_mask,
                                                   VkPipelineStageFlags dst_stage,
                                                   uint32_t dst_access_mask,
                                                   VkDependencyFlagBits flags = (VkDependencyFlagBits)0)
{
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = src_subpass;
    dependency.dstSubpass = dst_subpass;
	
    dependency.srcStageMask = src_stage;
    dependency.srcAccessMask = src_access_mask;
	
    dependency.dstStageMask = dst_stage;
    dependency.dstAccessMask = dst_access_mask;

    dependency.dependencyFlags = flags;
	
    return(dependency);
}

void initialize_render_pass(const std::vector<render_pass_attachment_t> &attachments,
                            const std::vector<render_pass_subpass_t> &subpasses,
                            const std::vector<render_pass_dependency_t> &dependencies,
                            bool clear_every_frame,
                            VkRenderPass *render_pass)
{
    VkAttachmentDescription descriptions_vk[10] = {};
    uint32_t att_i = 0;
    for (; att_i < attachments.size(); ++att_i)
    {
        descriptions_vk[att_i] = make_attachment_description(attachments[att_i].format,
                                                             VK_SAMPLE_COUNT_1_BIT,
                                                             clear_every_frame ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                             VK_ATTACHMENT_STORE_OP_STORE,
                                                             VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                             VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                             VK_IMAGE_LAYOUT_UNDEFINED,
                                                             attachments[att_i].final_layout);
    }
    // arbitrary max is 5
    VkSubpassDescription subpasses_vk[5] = {};
    uint32_t sub_i = 0;
    VkAttachmentReference reference_buffer[30] = {};
    uint32_t reference_count = 0;
    for (; sub_i < subpasses.size(); ++sub_i)
    {
        // arbitrary max is 10
        uint32_t ref_i = 0;
        uint32_t color_reference_start = reference_count;
        for (; ref_i < subpasses[sub_i].color_attachment_count; ++ref_i, ++reference_count)
        {
            reference_buffer[reference_count] = make_attachment_reference(subpasses[sub_i].color_attachments[ref_i].index,
                                                                          subpasses[sub_i].color_attachments[ref_i].layout);
        }

        uint32_t input_reference_start = reference_count;
        uint32_t inp_i = 0;
        for (; inp_i < subpasses[sub_i].input_attachment_count; ++inp_i, ++reference_count)
        {
            reference_buffer[reference_count] = make_attachment_reference(subpasses[sub_i].input_attachments[inp_i].index,
                                                                          subpasses[sub_i].input_attachments[inp_i].layout);
        }

        uint32_t depth_reference_ptr = reference_count;
        if (subpasses[sub_i].enable_depth)
        {
            reference_buffer[reference_count++] = make_attachment_reference(subpasses[sub_i].depth_attachment.index,
                                                                            subpasses[sub_i].depth_attachment.layout);
        }

        // For small example like this, doesn't matter to keep this memory alive
        VkAttachmentReference *outputs_vector = new VkAttachmentReference[ref_i];
        memcpy(outputs_vector, &reference_buffer[color_reference_start], sizeof(VkAttachmentReference) * ref_i);
        
        VkAttachmentReference *inputs_vector = new VkAttachmentReference[inp_i];
        memcpy(inputs_vector, &reference_buffer[input_reference_start], sizeof(VkAttachmentReference) * inp_i);

        subpasses_vk[sub_i] = make_subpass_description(outputs_vector, ref_i, subpasses[sub_i].enable_depth ? &reference_buffer[depth_reference_ptr] : nullptr, inputs_vector, inp_i);
    }

    VkSubpassDependency dependencies_vk[10] = {};
    uint32_t dep_i = 0;
    for (; dep_i < dependencies.size(); ++dep_i)
    {
        const render_pass_dependency_t *info = &dependencies[dep_i];
        dependencies_vk[dep_i] = make_subpass_dependency(info->src_index, info->dst_index,
                                                         info->src_stage, info->src_access,
                                                         info->dst_stage, info->dst_access,
                                                         VK_DEPENDENCY_BY_REGION_BIT);
    }

    VkRenderPassCreateInfo render_pass_info	= {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = att_i;
    render_pass_info.pAttachments = descriptions_vk;
    render_pass_info.subpassCount = sub_i;
    render_pass_info.pSubpasses = subpasses_vk;
    render_pass_info.dependencyCount = dep_i;
    render_pass_info.pDependencies = dependencies_vk;

    if (vkCreateRenderPass(g_context.gpu.logical_device, &render_pass_info, nullptr, render_pass) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize render pass");
        assert(0);
    }
}

void initialize_framebuffer_attachment(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, framebuffer_attachment_t *attachment)
{
    attachment->format = format;
    
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = extent.width;
    image_info.extent.height = extent.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.flags = 0;

    if (vkCreateImage(g_context.gpu.logical_device, &image_info, nullptr, &attachment->image) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize image");
        assert(0);
    }

    VkMemoryRequirements mem_requirements = {};
    vkGetImageMemoryRequirements(g_context.gpu.logical_device,
                                 attachment->image,
                                 &mem_requirements);

    allocate_gpu_memory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_requirements, &attachment->memory);

    vkBindImageMemory(g_context.gpu.logical_device, attachment->image, attachment->memory, 0);

    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = attachment->image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT);
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(g_context.gpu.logical_device, &view_info, nullptr, &attachment->view) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize image view");
        assert(0);
    }
}

void initialize_framebuffer(std::vector<framebuffer_attachment_t> &color_attachments,
                            framebuffer_attachment_t &depth_attachment,
                            const VkExtent2D &extent,
                            VkRenderPass compatible_render_pass,
                            framebuffer_t *framebuffer)
{
    std::vector<VkImageView> image_view_attachments;
    image_view_attachments.resize(color_attachments.size() + 1 /* In this example, there will always be a depth */);
	
    for (uint32_t i = 0
             ; i < color_attachments.size()
             ; ++i)
    {
        VkImageView *image = &color_attachments[i].view;
        image_view_attachments[i] = *image;
    }

    VkImageView *depth_image = &depth_attachment.view;
    image_view_attachments[color_attachments.size()] = *depth_image;
	
    VkFramebufferCreateInfo fbo_info = {};
    fbo_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbo_info.renderPass = compatible_render_pass;
    fbo_info.attachmentCount = image_view_attachments.size();
    fbo_info.pAttachments = image_view_attachments.data();
    fbo_info.width = extent.width;
    fbo_info.height = extent.height;
    fbo_info.layers = 1;

    if (vkCreateFramebuffer(g_context.gpu.logical_device, &fbo_info, nullptr, &framebuffer->framebuffer) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize framebuffer");
        assert(0);
    }

    framebuffer->extent = extent;
    framebuffer->color_attachments = std::move(color_attachments);
}

inline void init_shader_pipeline_info(VkShaderModule *module,
                                      VkShaderStageFlagBits stage_bits,
                                      VkPipelineShaderStageCreateInfo *dest_info)
{
    dest_info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    dest_info->stage = stage_bits;
    dest_info->module = *module;
    dest_info->pName = "main";	
}

inline void init_pipeline_vertex_input_info(model_t *model, /* model contains input information required */
                                            VkPipelineVertexInputStateCreateInfo *info)
{
    if (model)
    {
        model->create_vertex_input_state_info(info);
    }
    else
    {
        info->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    }
}

inline void init_pipeline_input_assembly_info(VkPipelineInputAssemblyStateCreateFlags flags,
                                              VkPrimitiveTopology topology,
                                              VkBool32 primitive_restart,
                                              VkPipelineInputAssemblyStateCreateInfo *info)
{
    info->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info->topology = topology;
    info->primitiveRestartEnable = primitive_restart;
}

inline void init_rect2D(VkOffset2D offset,
                        VkExtent2D extent,
                        VkRect2D *rect)
{
    rect->offset = offset;
    rect->extent = extent;
}

inline void init_pipeline_viewport_info(VkViewport &viewport,
                                        VkRect2D &scissor,
                                        VkPipelineViewportStateCreateInfo *info)
{
    info->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info->viewportCount = 1;
    info->pViewports = &viewport;
    info->scissorCount = 1;
    info->pScissors = &scissor;
}

inline void init_pipeline_multisampling_info(VkSampleCountFlagBits rasterization_samples,
                                             VkPipelineMultisampleStateCreateFlags flags,
                                             VkPipelineMultisampleStateCreateInfo *info)
{
    info->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info->sampleShadingEnable = VK_FALSE;
    info->rasterizationSamples = rasterization_samples;
    info->minSampleShading = 1.0f;
    info->pSampleMask = nullptr;
    info->alphaToCoverageEnable = VK_FALSE;
    info->alphaToOneEnable = VK_FALSE;
    info->flags = flags;
}

inline void init_blend_state_attachment(VkColorComponentFlags color_write_flags,
                                        VkBool32 enable_blend,
                                        VkBlendFactor src_color,
                                        VkBlendFactor dst_color,
                                        VkBlendOp color_op,
                                        VkBlendFactor src_alpha,
                                        VkBlendFactor dst_alpha,
                                        VkBlendOp alpha_op,
                                        VkPipelineColorBlendAttachmentState *attachment)
{

    attachment->colorWriteMask = color_write_flags;
    attachment->blendEnable = enable_blend;
    attachment->srcColorBlendFactor = src_color;
    attachment->dstColorBlendFactor = dst_color;
    attachment->colorBlendOp = color_op;
    attachment->srcAlphaBlendFactor = src_alpha;
    attachment->dstAlphaBlendFactor = dst_alpha;
    attachment->alphaBlendOp = alpha_op;
}

inline void init_pipeline_blending_info(VkBool32 enable_logic_op,
                                        VkLogicOp logic_op,
                                        uint32_t state_count,
                                        VkPipelineColorBlendAttachmentState *states,
                                        VkPipelineColorBlendStateCreateInfo *info)
{
    info->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info->logicOpEnable = enable_logic_op;
    info->logicOp = logic_op;
    info->attachmentCount = state_count;
    info->pAttachments = states;
    info->blendConstants[0] = 0.0f;
    info->blendConstants[1] = 0.0f;
    info->blendConstants[2] = 0.0f;
    info->blendConstants[3] = 0.0f;
}

inline void init_pipeline_depth_stencil_info(VkBool32 depth_test_enable,
                                             VkBool32 depth_write_enable,
                                             float min_depth_bounds,
                                             float max_depth_bounds,
                                             VkBool32 stencil_enable,
                                             VkPipelineDepthStencilStateCreateInfo *info)
{
    info->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info->depthTestEnable = depth_test_enable;
    info->depthWriteEnable = depth_write_enable;
    info->depthCompareOp = VK_COMPARE_OP_LESS;
    info->depthBoundsTestEnable = VK_FALSE;
    info->minDepthBounds = min_depth_bounds;
    info->maxDepthBounds = max_depth_bounds;
    info->stencilTestEnable = VK_FALSE;
    info->front = {};
    info->back = {};
}

inline void init_pipeline_rasterization_info(VkPolygonMode polygon_mode,
                                             VkCullModeFlags cull_flags,
                                             float line_width,
                                             VkPipelineRasterizationStateCreateFlags flags,
                                             VkPipelineRasterizationStateCreateInfo *info,
                                             VkBool32 enable_depth_bias = VK_FALSE,
                                             float bias_constant = 0.0f,
                                             float bias_slope = 0.0f)
{
    info->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info->depthClampEnable = VK_FALSE;
    info->rasterizerDiscardEnable = VK_FALSE;
    info->polygonMode = polygon_mode;
    info->lineWidth = line_width;
    info->cullMode = cull_flags;
    info->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    info->depthBiasEnable = enable_depth_bias;
    info->depthBiasConstantFactor = bias_constant;
    info->depthBiasClamp = 0.0f;
    info->depthBiasSlopeFactor = bias_slope;
    info->flags = flags;
}

inline void init_push_constant_range(VkShaderStageFlags stage_flags,
                                     uint32_t size,
                                     uint32_t offset,
                                     VkPushConstantRange *rng)
{
    rng->stageFlags = stage_flags;
    rng->offset = offset;
    rng->size = size;
}

inline void init_pipeline_layout(std::vector<VkDescriptorSetLayout> &layouts,
                                 VkPushConstantRange &range,
                                 VkPipelineLayout *pipeline_layout)
{
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = layouts.size();
    layout_info.pSetLayouts = layouts.data();
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &range;

    if (vkCreatePipelineLayout(g_context.gpu.logical_device,
                               &layout_info,
                               nullptr,
                               pipeline_layout) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize pipeline layout");
        assert(0);
    }
}

void mapped_gpu_memory_t::begin(void)
{
    vkMapMemory(g_context.gpu.logical_device, *memory, offset, size, 0, &data);
}

void mapped_gpu_memory_t::fill(void *ptr, uint32_t size)
{
    memcpy(data, ptr, size);
}

void mapped_gpu_memory_t::flush(uint32_t offset, uint32_t size)
{
    VkMappedMemoryRange range = {};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = *memory;
    range.offset = offset;
    range.size = size;
    vkFlushMappedMemoryRanges(g_context.gpu.logical_device, 1, &range);
}

void mapped_gpu_memory_t::end(void)
{
    vkUnmapMemory(g_context.gpu.logical_device, *memory);
}

void initialize_command_buffer(VkCommandPool *command_pool_source, VkCommandBufferLevel level, VkCommandBuffer *command_buffer);
void begin_command_buffer(VkCommandBuffer *command_buffer, VkCommandBufferUsageFlags usage_flags, VkCommandBufferInheritanceInfo *inheritance);
void end_command_buffer(VkCommandBuffer *command_buffer);
void submit(VkCommandBuffer *command_buffer,
            VkSemaphore *wait_semaphore,
            VkSemaphore *signal_semaphore,
            VkPipelineStageFlags *wait_stage,
            VkFence *fence,
            VkQueue *queue);

void initialize_gpu_buffer(gpu_buffer_t *buffer, uint32_t size, void *data, VkBufferUsageFlags usage, VkCommandPool *pool)
{
    VkDeviceSize buffer_size = size;
	
    gpu_buffer_t staging_buffer;
    staging_buffer.size = buffer_size;

    VkBufferCreateInfo buffer_info	= {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = staging_buffer.size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.flags = 0;

    if (vkCreateBuffer(g_context.gpu.logical_device, &buffer_info, nullptr, &staging_buffer.buffer) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize gpu buffer");
        assert(0);
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(g_context.gpu.logical_device, staging_buffer.buffer, &mem_requirements);

    allocate_gpu_memory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mem_requirements, &staging_buffer.memory);
	
    vkBindBufferMemory(g_context.gpu.logical_device, staging_buffer.buffer, staging_buffer.memory, 0);
    
    mapped_gpu_memory_t mapped_memory = staging_buffer.construct_map();
    mapped_memory.begin();
    mapped_memory.fill(data, size);
    mapped_memory.end();

    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.flags = 0;

    if (vkCreateBuffer(g_context.gpu.logical_device, &buffer_info, nullptr, &buffer->buffer) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize gpu buffer");
        assert(0);
    }

    vkGetBufferMemoryRequirements(g_context.gpu.logical_device, buffer->buffer, &mem_requirements);

    allocate_gpu_memory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_requirements, &buffer->memory);
	
    vkBindBufferMemory(g_context.gpu.logical_device, buffer->buffer, buffer->memory, 0);
    
    VkCommandBuffer command_buffer;
    initialize_command_buffer(pool,
                              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                              &command_buffer);

    begin_command_buffer(&command_buffer,
                         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                         nullptr);

    VkBufferCopy region = {};
    region.size = staging_buffer.size;
    vkCmdCopyBuffer(command_buffer,
                    staging_buffer.buffer,
                    buffer->buffer,
                    1,
                    &region);

    //destroy_single_use_command_buffer(&command_buffer, command_pool);
    end_command_buffer(&command_buffer);

    VkFence null_fence = VK_NULL_HANDLE;
    submit(&command_buffer,
           nullptr,
           nullptr,
           nullptr,
           &null_fence,
           &g_context.gpu.graphics_queue);

    vkQueueWaitIdle(g_context.gpu.graphics_queue);

    vkDestroyBuffer(g_context.gpu.logical_device, staging_buffer.buffer, nullptr);
    vkFreeMemory(g_context.gpu.logical_device, staging_buffer.memory, nullptr);
}

void initialize_graphics_pipeline(graphics_pipeline_t *ppln,
                                  const dynamic_states_t &dynamic,
                                  const shader_modules_t &modules,
                                  bool primitive_restart, VkPrimitiveTopology topology,
                                  VkPolygonMode polygonmode, VkCullModeFlags culling,
                                  std::vector<VkDescriptorSetLayout> &layouts,
                                  VkRenderPass *compatible,
                                  float depth_bias,
                                  bool enable_depth,
                                  uint32_t subpass,
                                  const shader_pk_data_t &pk,
                                  VkExtent2D viewport,
                                  const shader_blend_states_t &blends,
                                  model_t *model)
{
    VkShaderModule module_objects[shader_modules_t::MAX_SHADERS] = {};
    VkPipelineShaderStageCreateInfo infos[shader_modules_t::MAX_SHADERS] = {};
    for (uint32_t i = 0; i < modules.count; ++i)
    {
        std::ifstream bytecode(modules.modules[i].filename, std::ios::binary);
        if (!bytecode.good())
        {
            vulkan_error("Failed to find shader source: ", modules.modules[i].filename);
            assert(0);
        }
        std::vector<char> content = std::vector<char>(std::istreambuf_iterator<char>(bytecode), std::istreambuf_iterator<char>());

        VkShaderModuleCreateInfo shader_info = {};
        shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_info.codeSize = content.size();
        shader_info.pCode = reinterpret_cast<const uint32_t *>(content.data());

        if (vkCreateShaderModule(g_context.gpu.logical_device,
                                 &shader_info,
                                 nullptr,
                                 &module_objects[i]) != VK_SUCCESS)
        {
            vulkan_error("Failed to initialize shader module object");
            assert(0);
        }
        
        init_shader_pipeline_info(&module_objects[i], modules.modules[i].stage, &infos[i]);
    }
    VkPipelineVertexInputStateCreateInfo v_input = {};
    init_pipeline_vertex_input_info(model, &v_input);
    
    VkPipelineInputAssemblyStateCreateInfo assembly = {};
    init_pipeline_input_assembly_info(0, topology, primitive_restart, &assembly);
    
    VkPipelineViewportStateCreateInfo view_info = {};
    VkViewport view = {};
    init_viewport(0.0f, 0.0f, viewport.width, viewport.height, 0.0f, 1.0f, &view);
    
    VkRect2D scissor = {};
    init_rect2D(VkOffset2D{}, VkExtent2D{viewport.width, viewport.height}, &scissor);
    
    init_pipeline_viewport_info(view, scissor, &view_info);
    
    VkPipelineMultisampleStateCreateInfo multi = {};
    init_pipeline_multisampling_info(VK_SAMPLE_COUNT_1_BIT, 0, &multi);
    
    VkPipelineColorBlendStateCreateInfo blending_info = {};
    VkPipelineColorBlendAttachmentState blend_states[shader_blend_states_t::MAX_BLEND_STATES];
    for (uint32_t i = 0; i < blends.count; ++i)
    {
        if (blends.blend_states[i])
        {
            init_blend_state_attachment(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                                        VK_TRUE,
                                        VK_BLEND_FACTOR_SRC_ALPHA,
                                        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                        VK_BLEND_OP_ADD,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ZERO,
                                        VK_BLEND_OP_ADD,
                                        &blend_states[i]);
        }
        else
        {
            init_blend_state_attachment(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                                        VK_FALSE,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ZERO,
                                        VK_BLEND_OP_ADD,
                                        VK_BLEND_FACTOR_ONE,
                                        VK_BLEND_FACTOR_ZERO,
                                        VK_BLEND_OP_ADD,
                                        &blend_states[i]);
        }
    }
    init_pipeline_blending_info(VK_FALSE, VK_LOGIC_OP_COPY, blends.count, blend_states, &blending_info);

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = dynamic.count;
    dynamic_info.pDynamicStates = dynamic.dynamic_states;
    VkPipelineDepthStencilStateCreateInfo depth = {};
    init_pipeline_depth_stencil_info(enable_depth, enable_depth, 0.0f, 1.0f, VK_FALSE, &depth);
    VkPipelineRasterizationStateCreateInfo raster = {};
    init_pipeline_rasterization_info(polygonmode, culling, 2.0f, 0, &raster, depth_bias);

    VkPushConstantRange pk_range = {};
    init_push_constant_range(pk.stages, pk.size, pk.offset, &pk_range);
    
    init_pipeline_layout(layouts, pk_range, &ppln->layout);

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = modules.count;
    pipeline_info.pStages = infos;
    pipeline_info.pVertexInputState = &v_input;
    pipeline_info.pInputAssemblyState = &assembly;
    pipeline_info.pViewportState = &view_info;
    pipeline_info.pRasterizationState = &raster;
    pipeline_info.pMultisampleState = &multi;
    pipeline_info.pDepthStencilState = &depth;
    pipeline_info.pColorBlendState = &blending_info;
    pipeline_info.pDynamicState = &dynamic_info;

    pipeline_info.layout = ppln->layout;
    pipeline_info.renderPass = *compatible;
    pipeline_info.subpass = subpass;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(g_context.gpu.logical_device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipeline_info,
                                  nullptr,
                                  &ppln->pipeline) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize graphics pipeline");
        assert(0);
    }
    
    for (uint32_t i = 0; i < modules.count; ++i)
    {
        vkDestroyShaderModule(g_context.gpu.logical_device, module_objects[i], nullptr);
    }    
}

VkDescriptorSetLayoutBinding initialize_descriptor_layout_binding_s(uint32_t count,
                                                                    uint32_t binding,
                                                                    VkDescriptorType type,
                                                                    VkShaderStageFlags stage)
{
    VkDescriptorSetLayoutBinding ubo_binding = {};
    ubo_binding.binding = binding;
    ubo_binding.descriptorType = type;
    ubo_binding.descriptorCount	= count;
    ubo_binding.stageFlags = stage;
    ubo_binding.pImmutableSamplers = nullptr;
    return(ubo_binding);
}

void descriptor_layout_info_t::push(uint32_t count,
                                    uint32_t binding,
                                    VkDescriptorType uniform_type,
                                    VkShaderStageFlags shader_flags)
{
    bindings_buffer[binding_count++] = initialize_descriptor_layout_binding_s(count, binding, uniform_type, shader_flags);
}

VkDescriptorSetLayout initialize_descriptor_set_layout(descriptor_layout_info_t *blueprint)
{
    VkDescriptorSetLayout layout;
    
    VkDescriptorSetLayoutCreateInfo layout_info	= {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = blueprint->binding_count;
    layout_info.pBindings = blueprint->bindings_buffer;
	
    if (vkCreateDescriptorSetLayout(g_context.gpu.logical_device, &layout_info, nullptr, &layout) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize descriptor set layout");
        assert(0);
    }

    return layout;
}

VkDescriptorSet initialize_descriptor_set(VkDescriptorSetLayout *layout)
{
    VkDescriptorSet result;
	
    VkDescriptorSetAllocateInfo alloc_info	= {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = g_context.descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = layout;

    if (vkAllocateDescriptorSets(g_context.gpu.logical_device, &alloc_info, &result) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize descriptor set");
        assert(0);
    }

    return(result);
}

void update_descriptor_sets(VkWriteDescriptorSet *writes, uint32_t count)
{
    vkUpdateDescriptorSets(g_context.gpu.logical_device,
                           count,
                           writes,
                           0,
                           nullptr);
}

std::vector<const char *> initialize_vulkan_instance(void)
{
    std::vector<const char *> requested_validation_layers {"VK_LAYER_LUNARG_standard_validation"};
    
    uint32_t extension_count;
    const char **extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
    std::vector<const char *> extensions;
    extensions.resize(extension_count + 2 /* debug stuff */);
    memcpy(extensions.data(), extension_names, sizeof(const char *) * extension_count);
    extensions[extension_count] = "VK_EXT_debug_utils";
    extensions[extension_count + 1] = "VK_EXT_debug_report";

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "VML Debugging Test";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> properties;
    properties.resize(layer_count);
    
    vkEnumerateInstanceLayerProperties(&layer_count, properties.data());

    for (uint32_t r = 0; r < requested_validation_layers.size(); ++r)
    {
        bool found_layer = false;
        for (uint32_t l = 0; l < layer_count; ++l)
        {
            if (!strcmp(properties[l].layerName, requested_validation_layers[r])) found_layer = true;
        }

        if (!found_layer) assert(0);
    }

    instance_info.enabledLayerCount = requested_validation_layers.size();
    instance_info.ppEnabledLayerNames = requested_validation_layers.data();

    instance_info.enabledExtensionCount = extensions.size();
    instance_info.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&instance_info, nullptr, &g_context.instance) != VK_SUCCESS)
    {
        vulkan_error("Failed to create vulkan instance");
        assert(0);
    }

    return requested_validation_layers;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_proc(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  VkDebugUtilsMessageTypeFlagsEXT message_type,
                  const VkDebugUtilsMessengerCallbackDataEXT *message_data,
                  void *user_data)
{
    vulkan_error("Validation layer> ", message_data->pMessage);
    return(VK_FALSE);
}

void initialize_debug_messenger(void)
{
    // setup debugger
    VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = vulkan_debug_proc;
    debug_info.pUserData = nullptr;

    PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_context.instance, "vkCreateDebugUtilsMessengerEXT");
    
    assert(vk_create_debug_utils_messenger != nullptr);
    
    if (vk_create_debug_utils_messenger(g_context.instance, &debug_info, nullptr, &g_context.debug_messenger) != VK_SUCCESS)
    {
        vulkan_error("Failed to create vulkan instance");
        assert(0);
    }
}

void initialize_window_surface(window_data_t &window)
{
    glfwCreateWindowSurface(g_context.instance, window.window, nullptr, &g_context.surface);
}

void gpu_t::find_queue_families(VkSurfaceKHR *surface)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(hardware, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_properties;
    queue_properties.resize(queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(hardware, &queue_family_count, queue_properties.data());

    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        if (queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queue_properties[i].queueCount > 0)
        {
            queue_families.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(hardware, i, *surface, &present_support);
	
        if (queue_properties[i].queueCount > 0 && present_support)
        {
            queue_families.present_family = i;
        }
	
        if (queue_families.complete())
        {
            break;
        }
    }
}

bool check_if_physical_device_supports_extensions(std::vector<const char *> &extensions, VkPhysicalDevice gpu)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extension_properties;
    extension_properties.resize(extension_count);
    
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, extension_properties.data());
    
    uint32_t required_extensions_left = extensions.size();
    
    for (uint32_t i = 0; i < extension_count && required_extensions_left > 0; ++i)
    {
        for (uint32_t j = 0; j < extensions.size(); ++j)
        {
            if (!strcmp(extension_properties[i].extensionName, extensions[j]))
            {
                --required_extensions_left;
            }
        }
    }

    return(!required_extensions_left);
}

void get_swapchain_support(VkSurfaceKHR *surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_context.gpu.hardware, *surface, &g_context.gpu.swapchain_support.capabilities);
    uint32_t available_formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_context.gpu.hardware, *surface, &available_formats_count, nullptr);
    g_context.gpu.swapchain_support.available_formats.resize(available_formats_count);

    if (g_context.gpu.swapchain_support.available_formats.size() != 0)
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR(g_context.gpu.hardware
                                             , *surface
                                             , &available_formats_count
                                             , g_context.gpu.swapchain_support.available_formats.data());
    }

    uint32_t present_modes_count = 0 ;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g_context.gpu.hardware, *surface, &present_modes_count, nullptr);
    g_context.gpu.swapchain_support.available_present_modes.resize(present_modes_count);    
    if (g_context.gpu.swapchain_support.available_present_modes.size() != 0)
    {
        vkGetPhysicalDeviceSurfacePresentModesKHR(g_context.gpu.hardware
                                                  , *surface
                                                  , &present_modes_count
                                                  , g_context.gpu.swapchain_support.available_present_modes.data());
    }
}

bool check_if_physical_device_is_suitable(std::vector<const char *> &extensions, VkSurfaceKHR *surface)
{
    g_context.gpu.find_queue_families(surface);

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(g_context.gpu.hardware, &device_properties);
    
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(g_context.gpu.hardware, &device_features);

    bool swapchain_supported = check_if_physical_device_supports_extensions(extensions, g_context.gpu.hardware);

    bool swapchain_usable = false;
    if (swapchain_supported)
    {
        get_swapchain_support(surface);
        swapchain_usable = g_context.gpu.swapchain_support.available_formats.size() && g_context.gpu.swapchain_support.available_present_modes.size();
    }

    return(swapchain_supported && swapchain_usable
           && (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
           && g_context.gpu.queue_families.complete()
           && device_features.geometryShader
           && device_features.wideLines
           && device_features.textureCompressionBC
           && device_features.samplerAnisotropy
           && device_features.fillModeNonSolid);
}

VkFormat find_supported_format(const VkFormat *candidates, uint32_t candidate_size, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (uint32_t i = 0; i < candidate_size; ++i)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(g_context.gpu.hardware, candidates[i], &properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
        {
            return(candidates[i]);
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
        {
            return(candidates[i]);
        }
    }

    vulkan_error("Failed to find a supported depth format");
    assert(false);

    return VkFormat{};
}

void find_depth_format(void)
{
    VkFormat formats[] { VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT };
    
    g_context.gpu.supported_depth_format = find_supported_format(formats,
                                                                 3,
                                                                 VK_IMAGE_TILING_OPTIMAL,
                                                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

inline constexpr uint32_t left_shift(uint32_t n) { return 1 << n; }

struct bitset32_t
{
    uint32_t bitset = 0;

    inline uint32_t pop_count(void)
    {
        // Windows
	return __popcnt(bitset);  
    }
    inline void set1(uint32_t bit) { bitset |= left_shift(bit); }
    inline void set0(uint32_t bit) { bitset &= ~(left_shift(bit)); }
    inline bool get(uint32_t bit) { return bitset & left_shift(bit); }
};

void initialize_device(std::vector<const char *> &validation_layers)
{
    std::vector<const char *> gpu_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(g_context.instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> hardware_list;
    hardware_list.resize(device_count);

    
    vkEnumeratePhysicalDevices(g_context.instance, &device_count, hardware_list.data());

    for (uint32_t i = 0; i < device_count; ++i)
    {
        g_context.gpu.hardware = hardware_list[i];
	
        // check if device is suitable
        if (check_if_physical_device_is_suitable(gpu_extensions, &g_context.surface))
        {
            vkGetPhysicalDeviceProperties(g_context.gpu.hardware, &g_context.gpu.properties);
            break;
        }
    }

    if (g_context.gpu.hardware == VK_NULL_HANDLE)
    {
        vulkan_error("Failed to find suitable hardware");
        assert(0);
    }

    find_depth_format();

    // create the logical device
    queue_families_t *indices = &g_context.gpu.queue_families;

    bitset32_t bitset;
    bitset.set1(indices->graphics_family);
    bitset.set1(indices->present_family);

    uint32_t unique_sets = bitset.pop_count();

    std::vector<uint32_t> unique_family_indices;
    unique_family_indices.resize(unique_sets);
    
    std::vector<VkDeviceQueueCreateInfo> unique_queue_infos;
    unique_queue_infos.resize(unique_sets);

    // fill the unique_family_indices with the indices
    for (uint32_t b = 0, set_bit = 0; b < 32 && set_bit < unique_sets; ++b)
    {
        if (bitset.get(b))
        {
            unique_family_indices[set_bit++] = b;
        }
    }
    
    float priority1 = 1.0f;
    for (uint32_t i = 0; i < unique_sets; ++i)
    {
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = unique_family_indices[i];
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &priority1;
        unique_queue_infos[i] = queue_info;
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.wideLines = VK_TRUE;
    device_features.geometryShader = VK_TRUE;
    device_features.fillModeNonSolid = VK_TRUE;
	
    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pQueueCreateInfos = unique_queue_infos.data();
    device_info.queueCreateInfoCount = unique_sets;
    device_info.pEnabledFeatures = &device_features;
    device_info.enabledExtensionCount = gpu_extensions.size();
    device_info.ppEnabledExtensionNames = gpu_extensions.data();
    device_info.ppEnabledLayerNames = validation_layers.data();
    device_info.enabledLayerCount = validation_layers.size();
    
    if(vkCreateDevice(g_context.gpu.hardware,
                      &device_info,
                      nullptr,
                      &g_context.gpu.logical_device) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize device");
        assert(0);
    }

    vkGetDeviceQueue(g_context.gpu.logical_device, g_context.gpu.queue_families.graphics_family, 0, &g_context.gpu.graphics_queue);
    vkGetDeviceQueue(g_context.gpu.logical_device, g_context.gpu.queue_families.present_family, 0, &g_context.gpu.present_queue);
}

VkSurfaceFormatKHR choose_surface_format(VkSurfaceFormatKHR *available_formats, uint32_t format_count)
{
    if (format_count == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        VkSurfaceFormatKHR format;
        format.format		= VK_FORMAT_B8G8R8A8_UNORM;
        format.colorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    for (uint32_t i = 0
             ; i < format_count
             ; ++i)
    {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return(available_formats[i]);
        }
    }

    return(available_formats[0]);
}

VkExtent2D choose_swapchain_extent(window_data_t *window, const VkSurfaceCapabilitiesKHR *capabilities)
{
    if (capabilities->currentExtent.width != std::numeric_limits<uint64_t>::max())
    {
        return(capabilities->currentExtent);
    }
    else
    {
        int32_t width = window->width, height = window->height;

        VkExtent2D actual_extent	= { (uint32_t)width, (uint32_t)height };
        actual_extent.width		= MAX(capabilities->minImageExtent.width, MIN(capabilities->maxImageExtent.width, actual_extent.width));
        actual_extent.height	= MAX(capabilities->minImageExtent.height, MIN(capabilities->maxImageExtent.height, actual_extent.height));

        return(actual_extent);
    }
}

VkPresentModeKHR choose_surface_present_mode(const VkPresentModeKHR *available_present_modes, uint32_t present_modes_count)
{
    // supported by most hardware
    VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < present_modes_count; ++i)
    {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return(available_present_modes[i]);
        }
        else if (available_present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            best_mode = available_present_modes[i];
        }
    }
    return(best_mode);
}

void initialize_swapchain(window_data_t *window_data)
{
    swapchain_details_t *swapchain_details = &g_context.gpu.swapchain_support;
    VkSurfaceFormatKHR surface_format = choose_surface_format(swapchain_details->available_formats.data(), swapchain_details->available_formats.size());
    VkExtent2D surface_extent = choose_swapchain_extent(window_data, &swapchain_details->capabilities);
    VkPresentModeKHR present_mode = choose_surface_present_mode(swapchain_details->available_present_modes.data(), swapchain_details->available_present_modes.size());

    // add 1 to the minimum images supported in the swapchain
    uint32_t image_count = swapchain_details->capabilities.minImageCount + 1;
    if (image_count > swapchain_details->capabilities.maxImageCount)
    {
        image_count = swapchain_details->capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = g_context.surface;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = surface_format.format;
    swapchain_info.imageColorSpace = surface_format.colorSpace;
    swapchain_info.imageExtent = surface_extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = { (uint32_t)g_context.gpu.queue_families.graphics_family, (uint32_t)g_context.gpu.queue_families.present_family };

    if (g_context.gpu.queue_families.graphics_family != g_context.gpu.queue_families.present_family)
    {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_info.preTransform = swapchain_details->capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(g_context.gpu.logical_device, &swapchain_info, nullptr, &g_context.swapchain.swapchain) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize swapchain");
        assert(0);
    }

    vkGetSwapchainImagesKHR(g_context.gpu.logical_device, g_context.swapchain.swapchain, &image_count, nullptr);

    g_context.swapchain.imgs.resize(image_count);
	
    vkGetSwapchainImagesKHR(g_context.gpu.logical_device, g_context.swapchain.swapchain, &image_count, g_context.swapchain.imgs.data());
	
    g_context.swapchain.extent = surface_extent;
    g_context.swapchain.format = surface_format.format;
    g_context.swapchain.present_mode = present_mode;

    g_context.swapchain.views.resize(image_count);
	
    for (uint32_t i = 0; i < image_count; ++i)
    {
        VkImage *image = &g_context.swapchain.imgs[i];

        initialize_image_view(g_context.swapchain.format,
                              VK_IMAGE_ASPECT_COLOR_BIT,
                              VK_IMAGE_VIEW_TYPE_2D,
                              1,
                              image,
                              &g_context.swapchain.views[i]);
    }
}

void initialize_image_view(VkFormat format, VkImageAspectFlags aspect_flags, VkImageViewType type, uint32_t layers, VkImage *image, VkImageView *view)
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = *image;
    view_info.viewType = type;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = layers;

    if(vkCreateImageView(g_context.gpu.logical_device, &view_info, nullptr, view) != VK_SUCCESS)
    {
        vulkan_error("Failed to create image view");
        assert(0);
    }
}

void initialize_command_pool(uint32_t family_index, VkCommandPoolCreateFlags flags, VkCommandPool *command_pool)
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = family_index;
    pool_info.flags = flags;

    if(vkCreateCommandPool(g_context.gpu.logical_device, &pool_info, nullptr, command_pool) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize command pool");
        assert(0);
    }
}

void initialize_semaphore(VkSemaphore *semaphore)
{
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
    if(vkCreateSemaphore(g_context.gpu.logical_device,
                               &semaphore_info,
                               nullptr,
                               semaphore) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize semaphore");
        assert(0);
    }
}

void initialize_fence(VkFenceCreateFlags flags, VkFence *fence)
{
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = flags;

    if(vkCreateFence(g_context.gpu.logical_device,
                           &fence_info,
                           nullptr,
                           fence) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize fence");
        assert(0);
    }
}

void initialize_command_buffer(VkCommandPool *command_pool_source,
                               VkCommandBufferLevel level,
                               VkCommandBuffer *command_buffer)
{
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = level;
    allocate_info.commandPool = *command_pool_source;
    allocate_info.commandBufferCount = 1;

    vkAllocateCommandBuffers(g_context.gpu.logical_device, &allocate_info, command_buffer);
}

struct next_image_return_t {VkResult result; uint32_t image_index;};
next_image_return_t acquire_next_image(VkSemaphore *semaphore, VkFence *fence)
{
    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(g_context.gpu.logical_device,
                                            g_context.swapchain.swapchain,
                                            std::numeric_limits<uint64_t>::max(),
                                            *semaphore,
                                            *fence,
                                            &image_index);
    return(next_image_return_t{result, image_index});
}

void wait_fence(VkFence *fence)
{
    vkWaitForFences(g_context.gpu.logical_device,
                    1,
                    fence,
                    VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
}

void reset_fence(VkFence *fence)
{
    vkResetFences(g_context.gpu.logical_device, 1, fence);
}

void begin_command_buffer(VkCommandBuffer *command_buffer, VkCommandBufferUsageFlags usage_flags, VkCommandBufferInheritanceInfo *inheritance)
{
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = usage_flags;
    begin_info.pInheritanceInfo = inheritance;

    vkBeginCommandBuffer(*command_buffer, &begin_info);
}

void end_command_buffer(VkCommandBuffer *command_buffer)
{
    vkEndCommandBuffer(*command_buffer);
}

void submit(VkCommandBuffer *command_buffer,
            VkSemaphore *wait_semaphore,
            VkSemaphore *signal_semaphore,
            VkPipelineStageFlags *wait_stage,
            VkFence *fence,
            VkQueue *queue)
{
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = command_buffer;

    submit_info.waitSemaphoreCount = (wait_semaphore ? 1 : 0);
    submit_info.pWaitSemaphores = wait_semaphore;
    submit_info.pWaitDstStageMask = wait_stage;

    submit_info.signalSemaphoreCount = (signal_semaphore ? 1 : 0);
    submit_info.pSignalSemaphores = signal_semaphore;

    vkQueueSubmit(*queue, 1, &submit_info, *fence);
}

VkResult present(VkSemaphore *signal_semaphore,
                 uint32_t *image_index,
                 VkQueue *present_queue)
{
    VkSwapchainKHR *swapchains = &g_context.swapchain.swapchain;
    
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphore;

    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = image_index;

    return(vkQueuePresentKHR(*present_queue, &present_info));
}

frame_rendering_data_t begin_frame_rendering(void)
{
    VkFence null_fence = VK_NULL_HANDLE;
    
    auto next_image_data = acquire_next_image(&g_context.target_image_available_semaphore, &null_fence);
    
    if (next_image_data.result == VK_ERROR_OUT_OF_DATE_KHR)
    {
	// ---- recreate swapchain ----
	return {};
    }
    else if (next_image_data.result != VK_SUCCESS && next_image_data.result != VK_SUBOPTIMAL_KHR)
    {
	vulkan_error("Failed to acquire swapchain image");
    }
    
    wait_fence(&g_context.cpu_wait);
    reset_fence(&g_context.cpu_wait);

    g_context.current_image_index = next_image_data.image_index;
    
    begin_command_buffer(&g_context.primary_command_buffer, 0, nullptr);

    return {g_context.current_image_index, g_context.primary_command_buffer};
}

void end_frame_rendering_and_refresh(void)
{
    end_command_buffer(&g_context.primary_command_buffer);

    VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;;

    submit(&g_context.primary_command_buffer,
           &g_context.target_image_available_semaphore,
           &g_context.rendering_finished_semaphore,
           &wait_stages,
           &g_context.cpu_wait,
           &g_context.gpu.graphics_queue);
    
    VkSemaphore signal_semaphores[] = {g_context.rendering_finished_semaphore};

    present(&g_context.rendering_finished_semaphore,
            &g_context.current_image_index,
            &g_context.gpu.present_queue);
}

void
init_descriptor_pool_size(VkDescriptorType type,
                          uint32_t count,
                          VkDescriptorPoolSize *size)
{
    size->type = type;
    size->descriptorCount = count;
}

void initialize_descriptor_pool(void)
{
    VkDescriptorPoolSize pool_sizes[3] = {};

    init_descriptor_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20, &pool_sizes[0]);
    init_descriptor_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20, &pool_sizes[1]);
    init_descriptor_pool_size(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 20, &pool_sizes[2]);
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 3;
    pool_info.pPoolSizes = pool_sizes;

    pool_info.maxSets = 30;

    if (vkCreateDescriptorPool(g_context.gpu.logical_device, &pool_info, nullptr, &g_context.descriptor_pool) != VK_SUCCESS)
    {
        vulkan_error("Failed to initialize descriptor pool");
        assert(0);
    }
}

swapchain_information_t initialize_graphics_context(window_data_t &window)
{
    std::vector<const char *> validation_layers = initialize_vulkan_instance();
    initialize_debug_messenger();
    initialize_window_surface(window);
    initialize_device(validation_layers);
    initialize_swapchain(&window);
    
    initialize_command_pool(g_context.gpu.queue_families.graphics_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &g_context.command_pool_reset);
    initialize_semaphore(&g_context.target_image_available_semaphore);
    initialize_semaphore(&g_context.rendering_finished_semaphore);
    initialize_fence(VK_FENCE_CREATE_SIGNALED_BIT, &g_context.cpu_wait);
    initialize_command_buffer(&g_context.command_pool_reset, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &g_context.primary_command_buffer);
    initialize_descriptor_pool();

    return swapchain_information_t{ g_context.swapchain.format, g_context.gpu.supported_depth_format, g_context.swapchain.extent, g_context.swapchain.views, &g_context.command_pool_reset, &g_context.gpu };
}
