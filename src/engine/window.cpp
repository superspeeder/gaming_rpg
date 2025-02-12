#include "window.hpp"

#include "tools.hpp"

namespace engine {
    Window::Window(const WindowAttributes &windowAttributes) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_RESIZABLE, windowAttributes.resizable);
        glfwWindowHint(GLFW_VISIBLE, windowAttributes.showOnCreate);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(static_cast<int>(windowAttributes.size.x), static_cast<int>(windowAttributes.size.y), windowAttributes.title.c_str(), nullptr, nullptr);

        if (!m_Window) {
            throw crash(CrashReason::CriticalFailure, "Failed to create window.");
        }
    }

    const std::unique_ptr<vk::raii::SurfaceKHR> &Window::createSurface(const std::unique_ptr<vk::raii::Instance> &instance) {
        if (!m_Surface) {
            VkSurfaceKHR surface;
            if (const VkResult res = glfwCreateWindowSurface(**instance, m_Window, nullptr, &surface); res != VK_SUCCESS) {
                throw crash(CrashReason::CriticalFailure, "Failed to create window surface (" + vk::to_string(static_cast<vk::Result>(res)) + ").");
            }

            m_Surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, surface);
        }

        return m_Surface;
    }
} // namespace engine
