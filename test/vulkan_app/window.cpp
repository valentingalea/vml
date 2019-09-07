#include "window.hpp"

void glfw_mouse_position_proc(GLFWwindow *win, double x, double y)
{
    window_data_t *window_data = (window_data_t *)glfwGetWindowUserPointer(win);

    window_data->cursor_pos_x = x;
    window_data->cursor_pos_y = y;
    window_data->cursor_moved = 1;
}

void glfw_mouse_button_proc(GLFWwindow *win, int32_t button, int32_t action, int32_t mods)
{
    window_data_t *window_data = (window_data_t *)glfwGetWindowUserPointer(win);
    if (action == GLFW_PRESS)
    {
        window_data->mouse_buttons[button] = 1;
    }
    else if (action == GLFW_RELEASE)
    {
        window_data->mouse_buttons[button] = 0;
    }
}

void check_if_window_is_open(window_data_t &window, bool32_t &is_running)
{
    is_running = !glfwWindowShouldClose(window.window);
}

void initialize_window(window_data_t *window_data)
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        assert(0);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    window_data->width = 1700;
    window_data->height = 800;
    
    window_data->window = glfwCreateWindow(window_data->width, window_data->height, "VML Debugging Test", nullptr, nullptr);

    if (!window_data->window)
    {
        glfwTerminate();
        assert(0);
    }

    glfwSetWindowUserPointer(window_data->window, window_data);
    glfwSetMouseButtonCallback(window_data->window, glfw_mouse_button_proc);
    glfwSetCursorPosCallback(window_data->window, glfw_mouse_position_proc);
    // Ready to initialize Vulkan API
}

using time_stamp_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

time_stamp_t get_current_time_stamp(void)
{
    return std::chrono::high_resolution_clock::now();
}

float get_time_difference(time_stamp_t &start, time_stamp_t &end)
{
    return std::chrono::duration<double, std::chrono::seconds::period>(end - start).count();
}
