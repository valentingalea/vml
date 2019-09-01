#pragma once

#include "graphics_context.hpp"
#include <vml/uniform/uniform.hpp>

namespace debug_vulkan
{
    
    // Only need input, no need for output
    struct frame_capture_input_t
    {
        // This structure contains the image to be sampled from (device local, optimal tiling etc...)
        // AND a copy of this image in the form of a linearly tiled, host visible (mappable) image
        // The copy is what will be used to sample the texture / input attachment on the CPU-side
        vml::sampler2D cpu_sampler;
    };

    struct frame_capture_t
    {
        // Cache the VkDevice of the program, to use in the step_into_shader() function
        VkDevice logical_device;

        // The framebuffer concerned with the capture (in this example, it is the composition / deferred framebuffer)
        framebuffer_t *framebuffer;
        
        // List of all the inputs (input attachments or textures) required for the shader to work
        std::vector<frame_capture_input_t> inputs;

        // Cursor position is needed so that we can step into the pixel that the user clicked on
        uint32_t cursor_x, cursor_y;
        // Cache the screen resolution to calculate gl_FragCoord
        uint32_t screen_size_x, screen_size_y;
    
        void push_input_attachment(const framebuffer_attachment_t &attachment);
        // push_texture...
        // push_uniform_buffer...
        // etc...

        // Function creates all the linear copies of the sampled images (inputs)
        void initialize(VkDevice, VkPhysicalDevice, uint32_t screen_size_x, uint32_t screen_size_y);

        // Blits all the images into the linearly tiled images
        void capture(VkCommandBuffer command_buffer, uint32_t cursor_x, uint32_t cursor_y);

        // This function will be application specific, but will simply perform the memory maps and step into the image
        void step_into_shader(void);
    };

}
