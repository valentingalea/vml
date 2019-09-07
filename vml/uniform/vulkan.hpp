#pragma once

// For vulkan
#include <vulkan/vulkan.h>

#include "../vector.h"

namespace vml {

    // Texture objects will always be mapped (therefore on the host memory), so, not to compromise on performance for the client,
    // The textures will be blitted (anyway, they should not be blitted / updated often, just very rarely to debug something)
    // Into a linearly tiled image on the cpu-side for the mapping
    // For now only support color images, not depth
    // TODO: Support depth formats
    // TODO: Support mipmapped textures (but should work with mipped textures)
    // TODO: Support cubemaps

    namespace detail
    {
        float
        float16_to_float32(uint16_t f)
        {
            int f32i32 =  ((f & 0x8000) << 16);
            f32i32 |= ((f & 0x7fff) << 13) + 0x38000000;

            float ret;
            memcpy(&ret, &f32i32, sizeof(float));
            return ret;
        }
    }
        
    // With this library, for debugging, texture tiling will always be linear when blitted
    struct gpu_texture_object
    {
        VkFormat format;
        uint32_t w, h;
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
    };
        
    struct sampler2D
    {
        gpu_texture_object original_image;
        // Used for mapping
        gpu_texture_object blitted_linear;

        void *mapped_data;

        sampler2D(void) = default;
        sampler2D(const gpu_texture_object &original_image)
            : original_image(original_image), blitted_linear{original_image.format, original_image.w, original_image.h}
        {
        }

        void
        initialize(VkDevice gpu, VkPhysicalDevice hardware)
        {
            VkMemoryPropertyFlags blitted_image_memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                
            // Create linear image
            VkImageCreateInfo image_info = {};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.extent.width = blitted_linear.w;
            image_info.extent.height = blitted_linear.h;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            // TODO: Support cubemaps
            image_info.arrayLayers = 1;
            image_info.format = blitted_linear.format;
            image_info.tiling = VK_IMAGE_TILING_LINEAR;
            image_info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
            image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            // TODO: Possibly add the capability of concurrent sharing mode
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.flags = 0;

            // --- Issue create image call here
            vkCreateImage(gpu, &image_info, nullptr, &blitted_linear.image);
            // ---

            VkMemoryRequirements memory_requirements = {};
            vkGetImageMemoryRequirements(gpu,
                                         blitted_linear.image,
                                         &memory_requirements);

            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = memory_requirements.size;

            VkPhysicalDeviceMemoryProperties gpu_memory_properties;
            vkGetPhysicalDeviceMemoryProperties(hardware,
                                                &gpu_memory_properties);

            int32_t memory_type = 0;
            bool success = 0;
                
            for (; memory_type < gpu_memory_properties.memoryTypeCount; ++memory_type)
            {
                if (memory_requirements.memoryTypeBits & (1 << memory_type)
                    && (gpu_memory_properties.memoryTypes[memory_type].propertyFlags & blitted_image_memory_property) == blitted_image_memory_property)
                {
                    success = 1;
                    break;
                }
            }
            assert(success);
            
            alloc_info.memoryTypeIndex = memory_type;

            vkAllocateMemory(gpu,
                             &alloc_info,
                             nullptr,
                             &blitted_linear.memory);

            vkBindImageMemory(gpu, blitted_linear.image, blitted_linear.memory, 0);

            VkImageViewCreateInfo view_info	= {};
            view_info.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image	= blitted_linear.image;
            // TODO: Support cubemaps
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = blitted_linear.format;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel	= 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            // TODO: support cubemaps
            view_info.subresourceRange.layerCount = 1;

            vkCreateImageView(gpu, &view_info, nullptr, &blitted_linear.view);
        }

        // Just copies the image into the linearly host visible image
        void
        capture(VkImageLayout layout_before_copy,
                VkCommandBuffer command_buffer,
                VkDevice gpu)
        {
            // Need to copy the image to the linear image
            // TODO: Make pipeline barriers more flexible
            VkPipelineStageFlags flags_before = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            VkPipelineStageFlags flags_after = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                
            VkImageMemoryBarrier image_barrier = {};
            image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            image_barrier.oldLayout = layout_before_copy;
            image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            image_barrier.image = original_image.image;
            // TODO: Support depth
            image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_barrier.subresourceRange.baseMipLevel = 0;
            image_barrier.subresourceRange.levelCount = 1;
            image_barrier.subresourceRange.baseArrayLayer = 0;
            image_barrier.subresourceRange.layerCount = 1;
            
            // --- Issue the pre-copy pipeline barrier here on source image
            vkCmdPipelineBarrier(command_buffer,
                                 flags_before,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &image_barrier);
            // ---

            image_barrier.image = blitted_linear.image;
            image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            // --- Issue the pre-copy pipeline barrier here on destination image
            vkCmdPipelineBarrier(command_buffer,
                                 flags_before,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &image_barrier);
            // ---

            VkImageCopy image_copy = {};
            image_copy.srcSubresource.mipLevel = 0;
            image_copy.srcSubresource.layerCount = 1;
            // TODO: Support depth
            image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_copy.srcSubresource.baseArrayLayer = 0;
            image_copy.srcOffset.x = 0;
            image_copy.srcOffset.y = 0;
            image_copy.srcOffset.z = 0;
    
            image_copy.dstSubresource.mipLevel = 0;
            image_copy.dstSubresource.layerCount = 1;
            // TODO: Support depth
            image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_copy.dstSubresource.baseArrayLayer = 0;
            image_copy.dstOffset.x = 0;
            image_copy.dstOffset.y = 0;
            image_copy.dstOffset.z = 0;

            image_copy.extent.width = original_image.w;
            image_copy.extent.height = original_image.h;
            image_copy.extent.depth = 1;

            // --- Issue the copy command here
            vkCmdCopyImage(command_buffer,
                           original_image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           blitted_linear.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &image_copy);
            // ---

            image_barrier.image = original_image.image;
            image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            image_barrier.newLayout = layout_before_copy;

            // --- Issue the post-copy pipeline barrier here on source image
            vkCmdPipelineBarrier(command_buffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 flags_after,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &image_barrier);

            image_barrier.image = blitted_linear.image;
            image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_barrier.newLayout = layout_before_copy;

            // --- Issue the post-copy pipeline barrier here on destination image
            vkCmdPipelineBarrier(command_buffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 flags_after,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &image_barrier);
        }

        void
        begin(VkDevice gpu)
        {
            // --- Start mapping the memory
            VkMemoryRequirements requirements = {};
            vkGetImageMemoryRequirements(gpu, blitted_linear.image, &requirements);
            vkMapMemory(gpu,
                        blitted_linear.memory,
                        0,
                        requirements.size,
                        0,
                        &mapped_data);
        }

        void
        end(VkDevice gpu)
        {
            vkUnmapMemory(gpu, blitted_linear.memory);
        }
            
        // UVS :
        //  (0,0)  --->  (1,0)
        //    |            |
        //    V            V
        //  (0,1)  --->  (1,1)
        vector<float, 0, 1, 2, 3>
        glsl_texture(const vector<float, 0, 1> &uvs, VkDevice gpu) const
        {
            using vec4 = vector<float, 0, 1, 2, 3>;
                
            // In pixel space (ps = pixel space)
            // A prefix with second letter 's' is to denote the coordinate space the variable in
            uint32_t ps_uvs_x = (uint32_t)(uvs.x * (float)blitted_linear.w);
            uint32_t ps_uvs_y = (uint32_t)(uvs.y * (float)blitted_linear.h);

            uint32_t pixel_size;
            switch(blitted_linear.format)
            {
                // TODO: Support all vulkan image formats (there are a ton...)
            case VK_FORMAT_R8G8B8A8_UNORM: case VK_FORMAT_B8G8R8A8_UNORM: {pixel_size = sizeof(uint8_t) * 4; break;}
            case VK_FORMAT_R16G16B16A16_SFLOAT: {pixel_size = sizeof(uint16_t) * 4; break;}
            }

            uint8_t *pixels = (uint8_t *)mapped_data;

            // --- Get internal image layout
            VkImageSubresource subresources {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
            VkSubresourceLayout layout;
            vkGetImageSubresourceLayout(gpu,
                                        blitted_linear.image,
                                        &subresources,
                                        &layout);

            pixels += layout.offset;
            pixels += ps_uvs_y * (layout.rowPitch);
            uint8_t *pixel = ps_uvs_x * pixel_size + pixels;

            vec4 final_color = {};

            switch(blitted_linear.format)
            {
                // R8G8B8A8 and B8G8R8A8 have same swizzle format (? - to research)
            case VK_FORMAT_B8G8R8A8_UNORM: case VK_FORMAT_R8G8B8A8_UNORM:
                {
                    float r = (float)(*(pixel + 2)) / 256.0f;
                    float g = (float)(*(pixel + 1)) / 256.0f;
                    float b = (float)(*(pixel + 0)) / 256.0f;
                    float a = (float)(*(pixel + 3)) / 256.0f;
                    final_color = vec4(r, g, b, a);
                    break;
                }
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                {
                    uint16_t *pixel_16b = (uint16_t *)pixel;
                    float r = detail::float16_to_float32(*(pixel_16b));
                    float g = detail::float16_to_float32(*(pixel_16b + 1));
                    float b = detail::float16_to_float32(*(pixel_16b + 2));
                    float a = detail::float16_to_float32(*(pixel_16b + 3));
                    final_color = vec4(r, g, b, a);
                    break;
                }
            }
            return final_color;
        }
    };
    
}
