#include "engine/window.hpp"

#include "tools.hpp"

namespace engine {
    Window::Window(const WindowAttributes &window_attributes) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_RESIZABLE, window_attributes.resizable);
        glfwWindowHint(GLFW_VISIBLE, window_attributes.show_on_create);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(static_cast<int>(window_attributes.size.x), static_cast<int>(window_attributes.size.y), window_attributes.title.c_str(), nullptr, nullptr);

        if (!m_Window) {
            throw crash(CrashReason::CriticalFailure, "Failed to create window.");
        }
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    vk::raii::SurfaceKHR Window::create_surface_raw(const vk::raii::Instance &instance) const {
        VkSurfaceKHR surface;
        if (const VkResult res = glfwCreateWindowSurface(*instance, m_Window, nullptr, &surface); res != VK_SUCCESS) {
            throw crash(CrashReason::CriticalFailure, "Failed to create window surface (" + vk::to_string(static_cast<vk::Result>(res)) + ").");
        }

        return vk::raii::SurfaceKHR(instance, surface);
    }

    const std::unique_ptr<Surface> & Window::create_surface(const std::shared_ptr<VulkanContext> &vulkan_context) {
        m_Surface = std::make_unique<Surface>(vulkan_context, this);
        return m_Surface;
    }

    const std::unique_ptr<Surface> & Window::get_surface() const {
        return m_Surface;
    }

    vk::Extent2D Window::get_inner_size() const {
        int w,h;
        glfwGetFramebufferSize(m_Window, &w, &h);
        return vk::Extent2D(w, h);
    }
} // namespace engine
