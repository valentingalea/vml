#include "frame_capture.hpp"

void frame_capture_t::push_input_attachment(const framebuffer_attachment_t &attachment)
{
    vml::gpu_texture_object texture_object = { attachment.format, framebuffer->extent.width, framebuffer->extent.height, attachment.image, attachment.view, attachment.memory };
    
    frame_capture_input_t input = { texture_object };
    inputs.push_back(input);
}

void frame_capture_t::initialize(VkDevice logical_device, VkPhysicalDevice hardware)
{
    for (frame_capture_input_t &input : inputs)
    {
        input.cpu_sampler.initialize(logical_device, hardware);
    }
}

void frame_capture_t::capture(VkCommandBuffer command_buffer, VkDevice logical_device, uint32_t cursor_x, uint32_t cursor_y)
{
    for (frame_capture_input_t &input : inputs)
    {
        input.prepare(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      command_buffer,
                      logical_device);
    }
}



#include <vml/matrix.h>
#include <vml/vector.h>
#include <vml/vector_functions.h>

namespace glsl
{
    using  vec4 = vml::vector<float, 0, 1, 2, 3>;
    using  vec3 = vml::vector<float, 0, 1, 2>;
    using  vec2 = vml::vector<float, 0, 1>;
    using   _01 = vml::indices_pack<0, 1>;
    using  _012 = vml::indices_pack<0, 1, 2>;
    using _0123 = vml::indices_pack<0, 1, 2, 3>;
    using  mat2 = vml::matrix<float, vml::vector, _01, _01>;
    using  mat3 = vml::matrix<float, vml::vector, _012, _012>;
    using  mat4 = vml::matrix<float, vml::vector, _0123, _0123>;
    using sampler2D = vml::sampler2D;
    using subpassInput = vml::sampler2D;

#define layout(...) // NOTHING

    namespace funccall_inout
    {
        using vec2 = vec2 &;
        using vec3 = vec3 &;
        using vec4 = vec4 &;
        using mat2 = mat2 &;
        using mat3 = mat3 &;
        using mat4 = mat4 &;
        using float32_t = float32_t &;
        using bool_t = bool &;
    }
    
// =============== INCLUDE GLSL CODE HERE =================
#include "shaders/.hpp"
// ===============
}

#define PUSH_CONSTANT (inside_shader_symbol, object) glsl::##inside_shader_symbol = object
#define SET_UNIFORM (inside_shader_symbol, object) glsl::##inside_shader_symbol = object

void frame_capture_t::step_into_shader(void)
{
    
}
