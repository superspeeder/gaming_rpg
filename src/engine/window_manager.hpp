#pragma once

#include <unordered_map>
#include <memory>

#include "engine/window.hpp"

namespace engine {
    struct WindowHandle {
        Window* window;
        std::size_t index;

        inline Window* operator->() { return window; };
        inline Window* operator->() const { return window; };
    };


    class WindowManager {
      public:
        WindowManager();

        void connect_render_context(std::shared_ptr<VulkanContext> vkctx);

        WindowHandle create_window(const WindowAttributes &window_attributes);

      private:
        void on_window_closed(WindowHandle handle);

        std::shared_ptr<VulkanContext> m_VulkanContext;

        size_t                                              m_WindowCounter = 0;
        std::unordered_map<size_t, std::unique_ptr<Window>> m_Windows;
    };

    inline void WindowManager::on_window_closed(WindowHandle handle) {

    }

} // namespace engine
