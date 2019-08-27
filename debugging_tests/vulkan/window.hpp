#pragma once

#include <chrono>

/* --------------------------------- WINDOW CODE --------------------------------- */
using bool32_t = uint32_t;
using time_stamp_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct window_data_t
{
    class GLFWwindow *window;
    float cursor_pos_x, cursor_pos_y;
    bool32_t cursor_moved = 0;

    static constexpr uint32_t MAX_BUTTONS = 8;
    bool mouse_buttons[MAX_BUTTONS];

    int32_t width;
    int32_t height;
};

void glfw_mouse_position_proc(GLFWwindow *win, double x, double y);
void glfw_mouse_button_proc(GLFWwindow *win, int32_t button, int32_t action, int32_t mods);
void check_if_window_is_open(window_data_t &window, bool32_t &is_running);
time_stamp_t get_current_time_stamp(void);
float get_time_difference(time_stamp_t &start, time_stamp_t &end);

void initialize_window(window_data_t *window_data);
