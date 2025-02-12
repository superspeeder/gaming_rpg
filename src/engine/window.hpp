#pragma once

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace engine {

    struct WindowAttributes {
        std::string title;
        glm::uvec2 size;

        bool showOnCreate = true;
        bool resizable = false;
    };

    class Window final {
      public:
        explicit Window(const WindowAttributes& windowAttributes);
        ~Window();

        const std::unique_ptr<vk::raii::SurfaceKHR> &createSurface(const std::unique_ptr<vk::raii::Instance> &instance);

      private:
        GLFWwindow *m_Window;

        std::unique_ptr<vk::raii::SurfaceKHR> m_Surface;
    };

} // namespace engine
