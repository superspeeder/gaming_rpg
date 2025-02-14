#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "engine/renderer/vulkan_context.hpp"

namespace engine {
    class Window;

    struct SyncInfo {
        const vk::raii::Semaphore &image_available_semaphore;
        const vk::raii::Semaphore &render_finished_semaphore;
        const vk::raii::Fence &in_flight_fence;
    };

    struct FrameInfo {
        vk::Image image;
        uint32_t image_index;
        uint32_t frame_index;

        vk::Extent2D extent;
        vk::Format format;
        vk::ColorSpaceKHR color_space;

        SyncInfo sync_info;
    };

    class Surface {
    public:
        static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

        Surface(const std::shared_ptr<VulkanContext> &ctx, const Window *window);

        Surface(const Surface &other)                = delete;
        Surface(Surface &&other) noexcept            = default;
        Surface &operator=(const Surface &other)     = delete;
        Surface &operator=(Surface &&other) noexcept = default;

        void recreate_swapchain();

        FrameInfo begin_frame() const;

        void end_frame(const FrameInfo &frame_info);

      private:
        std::shared_ptr<VulkanContext> m_Context;
        const Window                  *m_Window;

        vk::raii::SurfaceKHR m_Surface = nullptr;
        vk::raii::SwapchainKHR m_Swapchain = nullptr;

        vk::SurfaceFormatKHR m_SurfaceFormat;
        vk::PresentModeKHR m_PresentMode;
        vk::Extent2D m_Extent;

        std::vector<vk::Image> m_Images;

        std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
        std::vector<vk::raii::Fence> m_InFlightFences;

        uint32_t m_CurrentFrame = 0;
    };
} // engine
