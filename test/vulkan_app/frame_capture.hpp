#pragma once

#include "graphics_context.hpp"
#include <vml/uniform/uniform.hpp>

// Only need input, no need for output
struct frame_capture_input_t
{
    vml::sampler2D cpu_sampler;
};

struct frame_capture_t
{
    VkDevice logical_device;
    
    framebuffer_t *framebuffer;
    std::vector<frame_capture_input_t> inputs;

    uint32_t cursor_x, cursor_y;
    uint32_t screen_size_x, screen_size_y;
    
    void push_input_attachment(const framebuffer_attachment_t &attachment);
    // push_texture...
    // push_uniform_buffer...
    // etc...
    
    void initialize(VkDevice, VkPhysicalDevice, uint32_t screen_size_x, uint32_t screen_size_y);

    // Blits all the images into the linearly tiled images
    void capture(VkCommandBuffer command_buffer, uint32_t cursor_x, uint32_t cursor_y);

    // This function will be application specific
    void step_into_shader(void);
};
