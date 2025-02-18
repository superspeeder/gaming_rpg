#pragma once

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <memory>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/surface.hpp"

namespace engine {
    struct WindowAttributes {
        std::string title;
        glm::uvec2 size;

        bool show_on_create = true;
        bool resizable = false;
    };

    class Window final {
    public:
        explicit Window(const WindowAttributes &window_attributes);

        ~Window();

        [[nodiscard]] vk::raii::SurfaceKHR create_surface_raw(const vk::raii::Instance &instance) const;

        const std::unique_ptr<Surface> &create_surface(const std::shared_ptr<VulkanContext> &vulkan_context);

        [[nodiscard]] const std::unique_ptr<Surface> &get_surface() const;

        [[nodiscard]] vk::Extent2D get_inner_size() const;

        [[nodiscard]] bool is_open() const;

    private:
        GLFWwindow *m_Window;
        std::unique_ptr<Surface> m_Surface;
    };
} // namespace engine
