#define GLFW_INCLUDE_VULKAN
#define VML_API_IN_USE VML_API_VULKAN

#include <glm/glm.hpp>
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
#else
#include "window.hpp"
#include "graphics_context.hpp"
#endif

static struct renderer_t
{
    std::vector<framebuffer_t> composition_framebuffers;
    VkRenderPass composition_render_pass;

    gpu_buffer_t cube_vertices;
    gpu_buffer_t cube_indices;    
    model_t cube_model;
    graphics_pipeline_t lp_no_tex_pipeline;
    VkDescriptorSetLayout gbuffer_layout;
    graphics_pipeline_t composition_pipeline;
} g_renderer;

void tick(const window_data_t &window, float dt)
{
    frame_rendering_data_t frame = begin_frame_rendering();
    {
        // Do rendering
        begin_render_pass(frame.command_buffer,
                          g_renderer.composition_render_pass, g_renderer.composition_framebuffers[frame.frame_index], VK_SUBPASS_CONTENTS_INLINE,
                          make_clear_color_color(0, 0.4, 0.7, 0),
                          make_clear_color_color(0, 0.4, 0.7, 0),
                          make_clear_color_color(0, 0.4, 0.7, 0),
                          make_clear_color_color(0, 0.4, 0.7, 0),
                          make_clear_color_depth(1.0f, 0));
        {
            // Do world rendering
        }
        next_subpass(frame.command_buffer, VK_SUBPASS_CONTENTS_INLINE);
        {
            // Do lighting
        }
        end_render_pass(frame.command_buffer);
    }
    end_frame_rendering_and_refresh();
}

void initialize_deferred_render_pass(const window_data_t &window, const swapchain_information_t &swapchain);
void initialize_deferred_framebuffer(const window_data_t &window, const swapchain_information_t &swapchain);
void initialize_cube_data(VkCommandPool *pool);
void initialize_deferred_graphics_pipelines(const window_data_t &window, const swapchain_information_t &swapchain);

void initialize_scene(const window_data_t &window, const swapchain_information_t &swapchain)
{
    initialize_deferred_render_pass(window, swapchain);

    initialize_deferred_framebuffer(window, swapchain);

    initialize_cube_data(swapchain.pool);
    
    initialize_deferred_graphics_pipelines(window, swapchain);
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
        initialize_framebuffer_attachment(swapchain.swapchain_extent, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &color_attachments[1]);
        initialize_framebuffer_attachment(swapchain.swapchain_extent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &color_attachments[2]);
        initialize_framebuffer_attachment(swapchain.swapchain_extent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &color_attachments[3]);

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
        initialize_graphics_pipeline(&g_renderer.composition_pipeline, dynamic, modules, VK_FALSE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, std::vector<VkDescriptorSetLayout>{g_renderer.gbuffer_layout}, &dfr_render_pass,
                                     0.0f, false, 1, push_k, swapchain.swapchain_extent, blending, nullptr);
    }
}

