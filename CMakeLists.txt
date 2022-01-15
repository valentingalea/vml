cmake_minimum_required (VERSION 3.0.2)

project (VML)

if (MSVC)
	add_compile_options(/W4 /permissive- /std:c++17 /fp:fast /fp:except-)
	if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		add_compile_options(/O2 /Ob2 /Oi /Ot /Oy /Gy /GR- /arch:AVX /GL)
		#set(${CMAKE_EXE_LINKER_FLAGS} "/LTCG /DEBUG:NONE")
	endif()
else()
	add_compile_options(-Wall -std=c++17)
endif()

add_executable(test_simple test/test_simple.cpp)

add_executable(test_advanced test/catch.hpp test/test_advanced.cpp)

add_executable(test_shaders test/test_shaders.cpp)

SET(ENV{SDLDIR} "${PROJECT_SOURCE_DIR}/test/SDL_app/SDL-1.2.15/")
find_package(SDL)
include_directories(${SDL_INCLUDE_DIR})
add_executable(SDL_app test/SDL_app/SDL_app.cpp)
target_link_libraries(SDL_app ${SDL_LIBRARY})
target_compile_definitions(SDL_app PRIVATE SCR_W8=240 SCR_H8=240)
