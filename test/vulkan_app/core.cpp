#define GLFW_INCLUDE_VULKAN
#define VML_API_IN_USE VML_API_VULKAN

#include <glm.hpp>
#include <gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <chrono>
#include <cassert>
#include <vulkan/vulkan.h>

#include <vml/vector.h>
#include <vml/vector_functions.h>
#include <vml/vector_functions.h>
#include <vml/uniform/uniform.hpp>

#include <iostream>
#include <stdint.h>

#if defined (UNITY_BUILD)
#include "window.cpp"
#include "graphics_context.cpp"
#include "frame_capture.cpp"
#else
#include "window.hpp"
#include "graphics_context.hpp"
#include "frame_capture.hpp"
#endif

static struct renderer_t
{
    std::vector<framebuffer_t> composition_framebuffers;
    VkRenderPass composition_render_pass;

    gpu_buffer_t cube_vertices;
    gpu_buffer_t cube_indices;
    uint32_t index_count;

    model_t cube_model;
    graphics_pipeline_t lp_no_tex_pipeline;
    
    VkDescriptorSetLayout gbuffer_layout;
    VkDescriptorSet gbuffer_set;
    graphics_pipeline_t composition_pipeline;


    // --- Frame capture ---
    debug_vulkan::frame_capture_t frame_capture;
} g_renderer;

void render(frame_rendering_data_t &frame)
{
    glm::mat4 view = glm::lookAt(glm::vec3(3.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0, 1.0f, 0.0f));
        
    // Do rendering
    framebuffer_t *dst_framebuffer = &g_renderer.composition_framebuffers[frame.frame_index];
    begin_render_pass(frame.command_buffer, 
                      g_renderer.composition_render_pass, *dst_framebuffer, VK_SUBPASS_CONTENTS_INLINE,
                      make_clear_color_color(0.4, 0.4, 0.4, 1.0f),
                      make_clear_color_color(0.4, 0.4, 0.4, 1.0f),
                      make_clear_color_color(0.4, 0.4, 0.4, 1.0f),
                      make_clear_color_color(0.4, 0.4, 0.4, 1.0f),
                      make_clear_color_depth(1.0f, 0));
    {
        // Do world rendering
            
        VkViewport viewport = {};
        init_viewport(0, 0, dst_framebuffer->extent.width, dst_framebuffer->extent.height, 0.0f, 1.0f, &viewport);
        vkCmdSetViewport(frame.command_buffer, 0, 1, &viewport);
            
        vkCmdBindPipeline(frame.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_renderer.lp_no_tex_pipeline.pipeline);

        VkDeviceSize zero = 0;
        vkCmdBindVertexBuffers(frame.command_buffer,0, 1, &g_renderer.cube_vertices.buffer, &zero);

        vkCmdBindIndexBuffer(frame.command_buffer, g_renderer.cube_indices.buffer, 0, VK_INDEX_TYPE_UINT32);

        struct push_constant_t
        {
            glm::mat4 model_view;
            glm::mat4 projection;
            glm::vec4 color;
            float roughness;
            float metalness;                
        } push_constant;

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)dst_framebuffer->extent.width / (float)dst_framebuffer->extent.height, 0.1f, 100.0f);
        projection[1][1] *= -1.0f;

        push_constant.model_view = view;
        push_constant.projection = projection;
        push_constant.color = glm::vec4(118.0f / 255.0f, 169.0f / 255.0f, 72.0f / 255.0f, 1.0f);
        push_constant.roughness = 0.2f;
        push_constant.metalness = 0.8f;
            
        vkCmdPushConstants(frame.command_buffer,
                           g_renderer.lp_no_tex_pipeline.layout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(push_constant_t),
                           &push_constant);

        vkCmdDrawIndexed(frame.command_buffer, g_renderer.cube_model.index_data.index_count, 1, 0, 0, 0);
    }
    next_subpass(frame.command_buffer, VK_SUBPASS_CONTENTS_INLINE);
    {
        // Do lighting

        VkViewport viewport = {};
        init_viewport(0, 0, dst_framebuffer->extent.width, dst_framebuffer->extent.height, 0.0f, 1.0f, &viewport);
        vkCmdSetViewport(frame.command_buffer, 0, 1, &viewport);
            
        vkCmdBindPipeline(frame.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_renderer.composition_pipeline.pipeline);

        vkCmdBindDescriptorSets(frame.command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                g_renderer.composition_pipeline.layout,
                                0,
                                1,
                                &g_renderer.gbuffer_set,
                                0,
                                nullptr);

        struct lighting_push_constant_t
        {
            glm::vec4 light_direction;
            glm::mat4 view_matrix;
        } push_constant;

        push_constant.light_direction = glm::vec4(glm::normalize(glm::vec3(1.0f, -1.0f, -0.2f)), 1.0f);
        push_constant.view_matrix = view;

        vkCmdPushConstants(frame.command_buffer,
                           g_renderer.composition_pipeline.layout,
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(push_constant),
                           &push_constant);

        vkCmdDraw(frame.command_buffer,
                  4,
                  1,
                  0,
                  0);
    }
    end_render_pass(frame.command_buffer);    
}

void tick(const window_data_t &window, float dt)
{
    bool did_capture = window.mouse_buttons[GLFW_MOUSE_BUTTON_LEFT];
    
    frame_rendering_data_t frame = begin_frame_rendering();
    {
        render(frame);

        // This function blits / copies all the images from the original VkImage object, to the mappable
        // This needs to happen within the command buffer recording
        // When command buffer recording is finished, we can map the images (because the command buffer will have been submitted and the memory will have been copied)
        if (did_capture)
        {
            g_renderer.frame_capture.capture(frame.command_buffer, window.cursor_pos_x, window.cursor_pos_y);
        }
    }
    
    end_frame_rendering_and_refresh();

    // Function first creates all the memory maps for the linear image-copies and steps into the shader
    if (did_capture)
    {
        g_renderer.frame_capture.step_into_shader();
    }
}

void initialize_deferred_render_pass(const window_data_t &window, const swapchain_information_t &swapchain);
void initialize_deferred_framebuffer(const window_data_t &window, const swapchain_information_t &swapchain);
void initialize_cube_data(VkCommandPool *pool);
void initialize_deferred_graphics_pipelines(const window_data_t &window, const swapchain_information_t &swapchain);


// --- Frame capture initialize ---
void initialize_frame_capture(const window_data_t &window, const swapchain_information_t &swapchain)
{
    // Set the framebuffer, so that debug_vulkan::frame_capture_t knows about the extent (width / height) of the attachments
    g_renderer.frame_capture.framebuffer = &g_renderer.composition_framebuffers[0];

    // Set all the inputs of the frame capture
    // These inputs (vml::sampler2D) will be used later to "sample" from the GPU textures that were allocated
    // To emulate what would be happening on the GPU
    // When it comes time to actually stepping into the GLSL code, the user should have mapped all the sampler2Ds / subpassInputs / etc... to the appropriate cpu-side input
    {
        // color attachment 1 = albedo, color attachment 2 = position, color attachment 3 = normal   (0 = swapchain image)
        g_renderer.frame_capture.push_input_attachment(g_renderer.composition_framebuffers[0].color_attachments[1]);
        g_renderer.frame_capture.push_input_attachment(g_renderer.composition_framebuffers[0].color_attachments[2]);
        g_renderer.frame_capture.push_input_attachment(g_renderer.composition_framebuffers[0].color_attachments[3]);
    }

    // To capture the data of the attached image, vml::sampler2D will create a mappable copy of the image that lies in the GPU
    // This function creates all the mappable Vulkan images and allocates the memory required
    g_renderer.frame_capture.initialize(swapchain.gpu->logical_device, swapchain.gpu->hardware, window.width, window.height);
}


void initialize_scene(const window_data_t &window, const swapchain_information_t &swapchain)
{
    initialize_deferred_render_pass(window, swapchain);

    initialize_deferred_framebuffer(window, swapchain);

    initialize_cube_data(swapchain.pool);
    
    initialize_deferred_graphics_pipelines(window, swapchain);

    initialize_frame_capture(window, swapchain);
}

int32_t main(int32_t argc, char *argv[])
{
    window_data_t window_data = {};
    initialize_window(&window_data);
    swapchain_information_t swapchain_info = initialize_graphics_context(window_data);
    initialize_scene(window_data, swapchain_info);
    bool32_t is_running = 1;
    float dt = 0.0f;

    while(is_running)
    {
        time_stamp_t tick_begin = get_current_time_stamp();
        {
            glfwPollEvents();
            tick(window_data, dt);
            check_if_window_is_open(window_data, is_running);
        }
        time_stamp_t tick_end = get_current_time_stamp();
        dt = get_time_difference(tick_begin, tick_end);
    }
}

void initialize_deferred_render_pass(const window_data_t &window, const swapchain_information_t &swapchain)
{
    // Initialize composition render pass
    std::vector<render_pass_attachment_t> attachments = { /*FINAL*/render_pass_attachment_t{swapchain.swapchain_format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
                                                          /*ALBEDO*/render_pass_attachment_t{VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                          /*POSITION*/render_pass_attachment_t{VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                          /*NORMAL*/render_pass_attachment_t{VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                          /*DEPTH*/render_pass_attachment_t{swapchain.supported_depth_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL} };
    std::vector<render_pass_subpass_t> subpasses = {}; subpasses.resize(2);
    subpasses[0].set_color_attachment_references(render_pass_attachment_reference_t{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                 render_pass_attachment_reference_t{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                 render_pass_attachment_reference_t{ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                 render_pass_attachment_reference_t{ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    subpasses[0].enable_depth = 1;
    subpasses[0].depth_attachment = render_pass_attachment_reference_t{ 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    subpasses[1].set_color_attachment_references(render_pass_attachment_reference_t{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        
    subpasses[1].set_input_attachment_references(render_pass_attachment_reference_t{ 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
                                                 render_pass_attachment_reference_t{ 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
                                                 render_pass_attachment_reference_t{ 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    
    std::vector<render_pass_dependency_t> dependencies = {}; dependencies.resize(3);
    dependencies[0] = make_render_pass_dependency(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                                                  0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    dependencies[1] = make_render_pass_dependency(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                  1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    dependencies[2] = make_render_pass_dependency(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                  VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT);

    initialize_render_pass(attachments, subpasses, dependencies, true, &g_renderer.composition_render_pass);    
}

void initialize_deferred_framebuffer(const window_data_t &window, const swapchain_information_t &swapchain)
{
    // Initialize the framebuffers (the count of depends on the image count of the swapchain)
    g_renderer.composition_framebuffers.resize(swapchain.image_views.size());
    for (uint32_t i = 0; i < g_renderer.composition_framebuffers.size(); ++i)
    {
        framebuffer_t *current_fbo = &g_renderer.composition_framebuffers[i];

        framebuffer_attachment_t depth_attachment;
        initialize_framebuffer_attachment(swapchain.swapchain_extent, swapchain.supported_depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &depth_attachment);
        
        std::vector<framebuffer_attachment_t> color_attachments;
        color_attachments.resize(4);
        // This will "initialize" the first attachment
        color_attachments[0].view = swapchain.image_views[i];
        initialize_framebuffer_attachment(swapchain.swapchain_extent, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, &color_attachments[1]);
        initialize_framebuffer_attachment(swapchain.swapchain_extent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, &color_attachments[2]);
        initialize_framebuffer_attachment(swapchain.swapchain_extent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, &color_attachments[3]);

        initialize_framebuffer(color_attachments, depth_attachment, swapchain.swapchain_extent, g_renderer.composition_render_pass, current_fbo);
    }
}

void initialize_cube_data(VkCommandPool *pool)
{
    g_renderer.cube_model.bindings.resize(1);
    g_renderer.cube_model.attributes.resize(1);
    model_binding_t *binding0 = &g_renderer.cube_model.bindings[0];
    binding0->begin_attributes_creation(g_renderer.cube_model.attributes.data());
    {
        binding0->push_attribute(0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3));
    }
    binding0->end_attributes_creation();
    
    float radius = 1.0f;
    
    struct cube_vertex_t { glm::vec3 pos; };
    cube_vertex_t vertices[]
    {
        {{-radius, -radius, radius }},
        {{radius, -radius, radius }},
        {{radius, radius, radius }},
        {{-radius, radius, radius }},
        {{-radius, -radius, -radius }},
        {{radius, -radius, -radius }},
        {{radius, radius, -radius }},
        {{-radius, radius, -radius }},
    };

    initialize_gpu_buffer(&g_renderer.cube_vertices, sizeof(vertices), vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, pool);

    binding0->buffer = g_renderer.cube_vertices.buffer;
    g_renderer.cube_model.create_vbo_list();

    uint32_t mesh_indices[]
    {
        0, 1, 2,
        2, 3, 0,
        1, 5, 6,
        6, 2, 1,
        7, 6, 5,
        5, 4, 7,
        3, 7, 4,
        4, 0, 3,
        4, 5, 1,
        1, 0, 4,
        3, 2, 6,
        6, 7, 3,
    };

    g_renderer.cube_model.index_data.index_type = VK_INDEX_TYPE_UINT32;
    g_renderer.cube_model.index_data.index_offset = 0;
    g_renderer.cube_model.index_data.index_count = sizeof(mesh_indices) / sizeof(mesh_indices[0]);

    initialize_gpu_buffer(&g_renderer.cube_indices, sizeof(mesh_indices), mesh_indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, pool);

    g_renderer.cube_model.index_data.index_buffer = g_renderer.cube_indices.buffer;
}

void initialize_deferred_graphics_pipelines(const window_data_t &window, const swapchain_information_t &swapchain)
{
    {
        VkRenderPass &dfr_render_pass = g_renderer.composition_render_pass;
        shader_modules_t modules(shader_module_info_t{"shaders/SPV/lp_no_tex_cube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
                                 shader_module_info_t{"shaders/SPV/lp_no_tex_cube.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT},
                                 shader_module_info_t{"shaders/SPV/lp_no_tex_cube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT});
        shader_uniform_layouts_t layouts;
        shader_pk_data_t push_k = {160, 0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};
        shader_blend_states_t blending(false, false, false, false);
        dynamic_states_t dynamic(VK_DYNAMIC_STATE_VIEWPORT);
        initialize_graphics_pipeline(&g_renderer.lp_no_tex_pipeline, dynamic, modules, VK_FALSE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, std::vector<VkDescriptorSetLayout>{}, &dfr_render_pass,
                                     0.0f, true, 0, push_k, swapchain.swapchain_extent, blending, &g_renderer.cube_model);
    }

    {
        descriptor_layout_info_t uniform_info = {};
        uniform_info.push(1, 0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT);
        uniform_info.push(1, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT);
        uniform_info.push(1, 2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT);
        g_renderer.gbuffer_layout = initialize_descriptor_set_layout(&uniform_info);
        
        VkRenderPass &dfr_render_pass = g_renderer.composition_render_pass;
        shader_modules_t modules(shader_module_info_t{"shaders/SPV/deferred_lighting.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
                                 shader_module_info_t{"shaders/SPV/deferred_lighting.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT});
        shader_uniform_layouts_t layouts (g_renderer.gbuffer_layout);
        shader_pk_data_t push_k{ 160, 0, VK_SHADER_STAGE_FRAGMENT_BIT };
        shader_blend_states_t blending(false);
        dynamic_states_t dynamic(VK_DYNAMIC_STATE_VIEWPORT);
        initialize_graphics_pipeline(&g_renderer.composition_pipeline, dynamic, modules, VK_FALSE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                     VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, std::vector<VkDescriptorSetLayout>{g_renderer.gbuffer_layout}, &dfr_render_pass,
                                     0.0f, false, 1, push_k, swapchain.swapchain_extent, blending, nullptr);
    }

    // Initialize descriptors
    framebuffer_attachment_t *albedo_tx = &g_renderer.composition_framebuffers[0].color_attachments[1];
    framebuffer_attachment_t *position_tx = &g_renderer.composition_framebuffers[0].color_attachments[2];
    framebuffer_attachment_t *normal_tx = &g_renderer.composition_framebuffers[0].color_attachments[3];

    g_renderer.gbuffer_set = initialize_descriptor_set(&g_renderer.gbuffer_layout);
    update_descriptor_set(&g_renderer.gbuffer_set,
                          update_binding_t{INPUT_ATTACHMENT, albedo_tx, 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                          update_binding_t{INPUT_ATTACHMENT, position_tx, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                          update_binding_t{INPUT_ATTACHMENT, normal_tx, 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}

