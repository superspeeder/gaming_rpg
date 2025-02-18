//
// Created by andy on 2/15/2025.
//

#include "window_manager.hpp"

#include <ranges>

namespace engine {
    WindowManager::WindowManager() = default;

    void WindowManager::connect_render_context(std::shared_ptr<VulkanContext> vkctx) {
        m_VulkanContext = vkctx;
        for (const auto &window : m_Windows | std::views::values) {
            window->create_surface(m_VulkanContext);
        }
    }

    WindowHandle WindowManager::create_window(const WindowAttributes &window_attributes) {
        static std::size_t index = 0;

        const auto window = std::make_unique<Window>(window_attributes);
        if (m_VulkanContext) {
            window->create_surface(m_VulkanContext);
        }

        WindowHandle handle{window.get(), index++};
        m_Windows.insert(std::make_pair(handle.index, std::move(window)));
        return handle;
    }

} // namespace engine
