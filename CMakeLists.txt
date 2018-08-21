cmake_minimum_required (VERSION 3.0)

project (VML)

if (MSVC)
	add_compile_options(/W4 /permissive- /std:c++17)
else()
	add_compile_options(-Wall -std=c++17)
endif()

add_executable(test_simple test/test_simple.cpp)

add_executable(test_advanced test/catch.hpp test/test_advanced.cpp)

add_executable(test_shaders test/test_shaders.cpp)

SET(ENV{SDLDIR} "${PROJECT_SOURCE_DIR}/test/SDL_app/SDL-1.2.15/")
find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
add_executable(SDL_app test/SDL_app/SDL_app.h test/SDL_app/SDL_app.cpp)
target_link_libraries(SDL_app ${SDL_LIBRARY})

# https://github.com/tcbrindle/sdl2-cmake-scripts
# https://www.reddit.com/r/gamedev/comments/4t07f0/how_do_you_use_findsdlcmake/
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/useful_variables/Logging-Useful-Variables
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")
set(SDL2_PATH "${PROJECT_SOURCE_DIR}/test/SDL2_app/SDL2-2.0.8/")
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
add_executable(SDL2_app test/SDL2_app/SDL2_app.h test/SDL2_app/SDL2_app.cpp)
target_link_libraries(SDL2_app ${SDL2_LIBRARY})