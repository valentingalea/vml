@echo off

C:/VulkanSDK/1.1.108.0/Bin32/glslangValidator.exe -V -o SPV/lp_no_tex_cube.vert.spv lp_no_tex_cube.vert
C:/VulkanSDK/1.1.108.0/Bin32/glslangValidator.exe -V -o SPV/lp_no_tex_cube.geom.spv lp_no_tex_cube.geom	
C:/VulkanSDK/1.1.108.0/Bin32/glslangValidator.exe -V -o SPV/lp_no_tex_cube.frag.spv lp_no_tex_cube.frag

C:/VulkanSDK/1.1.108.0/Bin32/glslangValidator.exe -V -o SPV/deferred_lighting.vert.spv deferred_lighting.vert
C:/VulkanSDK/1.1.108.0/Bin32/glslangValidator.exe -V -o SPV/deferred_lighting.frag.spv deferred_lighting.frag
