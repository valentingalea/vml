#pragma once

// For OpenGL
#include <GL/glew.h>

#include "vector.h"

// the two apis using glsl : vulkan and opengl

namespace vml {

    // Texture objects will always be mapped (therefore on the host memory), so, not to compromise on performance for the client,
    // The textures will be blitted (anyway, they should not be blitted / updated often, just very rarely to debug something)
    // Into a linearly tiled image on the cpu-side for the mapping

    // BUT: In OpenGL, can just directly call glGetTexImage
    namespace opengl
    {
        struct gpu_texture_object
        {
            GLint internal_format;
            GLenum format;
            GLenum component_type;
            uint32_t w, h;
            GLuint id;
        };

        struct sampler2D
        {
            gpu_texture_object original;

            void *mapped_data;

            sampler2D(const gpu_texture_object &original_image)
                : original(original_image)
            {
            }
            
            void
            prepare(void)
            {
                glBindTexture(GL_TEXTURE_2D, blitted_linear.id);
                glGetTexImage(GL_TEXTURE_2D, 0, original_image.format, original_image.component_type, &mapped_data);
            }
            
            // The magic
            vector<float, 0, 1, 2, 3>
            texture(const vector<float, 0, 1> &uvs) const = 0
            {
                // TODO: Implement sampling
            }
        };
    }
}
