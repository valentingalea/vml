#define GLFW_INCLUDE_VULKAN
#define VML_API_IN_USE VML_API_VULKAN

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

    VkPipeline lp_no_tex_pipeline;
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
void initialize_deferred_graphics_pipelines(const window_data_t &window, const swapchain_information_t &swapchain);

void initialize_scene(const window_data_t &window, const swapchain_information_t &swapchain)
{
    initialize_deferred_render_pass(window, swapchain);

    initialize_deferred_framebuffer(window, swapchain);

    
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

void initialize_deferred_graphics_pipelines(const window_data_t &window, const swapchain_information_t &swapchain)
{
    
}
