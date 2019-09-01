#include "frame_capture.hpp"

void frame_capture_t::push_input_attachment(const framebuffer_attachment_t &attachment)
{
    vml::gpu_texture_object texture_object = { attachment.format, framebuffer->extent.width, framebuffer->extent.height, attachment.image, attachment.view, attachment.memory };
    
    frame_capture_input_t input = { texture_object };
    inputs.push_back(input);
}

void frame_capture_t::initialize(VkDevice in_logical_device, VkPhysicalDevice hardware, uint32_t in_screen_size_x, uint32_t in_screen_size_y)
{
    this->logical_device = in_logical_device;
    this->screen_size_x = in_screen_size_x;
    this->screen_size_y = in_screen_size_y;
    for (frame_capture_input_t &input : inputs)
    {
        input.cpu_sampler.initialize(in_logical_device, hardware);
    }
}

void frame_capture_t::capture(VkCommandBuffer command_buffer, uint32_t in_cursor_x, uint32_t in_cursor_y)
{
    cursor_x = in_cursor_x;
    cursor_y = in_cursor_y;
    
    for (frame_capture_input_t &input : inputs)
    {
        input.cpu_sampler.capture(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  command_buffer,
                                  logical_device);
    }
}

#include <vml/matrix.h>
#include <vml/vector.h>
#include <vml/vector_functions.h>

namespace glsl
{
    // When preparing, need to set this, so that functions loading from GPU memory can actually interface with the GPU
    static VkDevice *logical_device;
    
    using  vec4 = vml::vector<float, 0, 1, 2, 3>;
    using  vec3 = vml::vector<float, 0, 1, 2>;
    using  vec2 = vml::vector<float, 0, 1>;
    using   _01 = vml::indices_pack<0, 1>;
    using  _012 = vml::indices_pack<0, 1, 2>;
    using _0123 = vml::indices_pack<0, 1, 2, 3>;
    using  mat2 = vml::matrix<float, vml::vector, _01, _01>;
    using  mat3 = vml::matrix<float, vml::vector, _012, _012>;
    using  mat4 = vml::matrix<float, vml::vector, _0123, _0123>;
    using sampler2D = vml::sampler2D *;
    using subpassInput = vml::sampler2D *;
    using uint = unsigned int;

#define layout(...) // NOTHING
#define uniform static
#define push_k_t struct push_constant_t

    namespace funccall_inout
    {
        using vec2 = vec2 &;
        using vec3 = vec3 &;
        using vec4 = vec4 &;
        using mat2 = mat2 &;
        using mat3 = mat3 &;
        using mat4 = mat4 &;
        using bool_t = bool &;
    }

#define in
#define out funccall_inout::
#define inout funccall_inout::

    static vec2 gl_FragCoord;
    static vec2 screen_size;

    vec4 subpassLoad(subpassInput &input)
    {
        vec2 uvs = gl_FragCoord / screen_size;
        
        return input->glsl_texture(uvs, *logical_device);
    }

    vec4 texture(sampler2D &sampler, vec2 uvs)
    {
        return sampler->glsl_texture(uvs, *logical_device);
    }
    
// =============== INCLUDE GLSL CODE HERE =================
#include "shaders/deferred_lighting.frag.hpp"
// ===============
    
}

#define PUSH_CONSTANT(name, object) glsl::##name = object
#define SET_UNIFORM(name, object) glsl::##name = object

void frame_capture_t::step_into_shader(void)
{
    for (frame_capture_input_t &input : inputs)
    {
        input.cpu_sampler.begin(logical_device);
    }
    
    glsl::logical_device = &logical_device;
    
    // In reality, gl_FragCoord uses center coords (add .5 to the values), but here, just set it to the pixel coord for simplicity
    glsl::gl_FragCoord = glsl::vec2(float(cursor_x), float(cursor_y));
    glsl::screen_size = glsl::vec2(float(screen_size_x), float(screen_size_y));

    // Set the uniforms (input attachments)
    {
        SET_UNIFORM(g_buffer_albedo, &inputs[glsl::G_BUFFER_ALBEDO].cpu_sampler);
        SET_UNIFORM(g_buffer_position, &inputs[glsl::G_BUFFER_POSITION].cpu_sampler);
        SET_UNIFORM(g_buffer_normal, &inputs[glsl::G_BUFFER_NORMAL].cpu_sampler);
    }

    // Push constants
    {
        glsl::push_constant_t push_constant = {};
        glm::vec4 light_direction = glm::vec4(glm::normalize(glm::vec3(1.0f, -1.0f, -0.2f)), 1.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(3.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0, 1.0f, 0.0f));

        push_constant.light_direction = glsl::vec4(light_direction.x, light_direction.y, light_direction.z, 1.0f);
        for (uint32_t x = 0; x < 4; ++x)
        {
            for (uint32_t y = 0; y < 4; ++y)
            {
                push_constant.view_matrix[x][y] = view[x][y];
            }
        }
        PUSH_CONSTANT(push_k, push_constant);
    }

    // Set vertex shader outputs (fragment shader inputs)
    {
        glsl::in_uvs = glsl::vec2(float(cursor_x) / float(screen_size_x), float(cursor_y) / float(screen_size_y));
    }
    
    __debugbreak();
    glsl::main();

    for (frame_capture_input_t &input : inputs)
    {
        input.cpu_sampler.end(logical_device);
    }
}
