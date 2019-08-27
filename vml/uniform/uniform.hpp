#pragma once

#define VML_API_SOFTWARE 0
#define VML_API_OPENGL 1
#define VML_API_VULKAN 2

#ifndef VML_API_IN_USE
#define VML_API_IN_USE VML_API_SOFTWARE
#endif

#if (VML_API_IN_USE == VML_API_SOFTWARE)
#include "software.hpp"
#elif (VML_API_IN_USE == VML_API_OPENGL)
#include "opengl.hpp"
#elif (VML_API_IN_USE == VML_API_VULKAN)
#include "vulkan.hpp"
#endif
