#include "surface.hpp"

#include "engine/window.hpp"

namespace engine {
    static vk::Extent2D clamp_extent(const vk::Extent2D &extent, const vk::Extent2D &min, const vk::Extent2D &max) {
        return vk::Extent2D(std::clamp(extent.width, min.width, max.width), std::clamp(extent.height, min.height, max.height));
    }

    vk::PresentModeKHR select_present_mode(const std::vector<vk::PresentModeKHR> &present_modes, bool desireVsync) {
        if (desireVsync) {
            return vk::PresentModeKHR::eFifo;
        }

        bool immediateAvailable = false;

        for (const auto &present_mode : present_modes) {
            if (present_mode == vk::PresentModeKHR::eMailbox) {
                return vk::PresentModeKHR::eMailbox;
            }

            if (present_mode == vk::PresentModeKHR::eImmediate) {
                immediateAvailable = true;
            }
        }

        if (immediateAvailable) {
            return vk::PresentModeKHR::eImmediate;
        }

        // Fall back on fifo
        return vk::PresentModeKHR::eFifo;
    }

    vk::SurfaceFormatKHR select_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats) {
        bool found_rgba_srgb = false;
        for (const auto &format : formats) {
            if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                if (format.format == vk::Format::eB8G8R8A8Srgb) {
                    return format;
                } else if (format.format == vk::Format::eR8G8B8A8Srgb) {
                    found_rgba_srgb = true;
                }
            }
        }

        if (found_rgba_srgb) {
            return vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);
        }

        return formats[0];
    }

    Surface::Surface(const std::shared_ptr<VulkanContext> &ctx, const Window *window) : m_Context(ctx), m_Window(window) {
        m_Surface = window->create_surface_raw(ctx->instance());
        recreate_swapchain();

        m_ImageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_ImageAvailableSemaphores.emplace_back(m_Context->device(), vk::SemaphoreCreateInfo());
            m_RenderFinishedSemaphores.emplace_back(m_Context->device(), vk::SemaphoreCreateInfo());
            m_InFlightFences.emplace_back(m_Context->device(), vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        }
    }

    void Surface::recreate_swapchain() {
        m_Context->device().waitIdle();

        const auto present_modes   = m_Context->physical_device().getSurfacePresentModesKHR(*m_Surface);
        const auto surface_formats = m_Context->physical_device().getSurfaceFormatsKHR(*m_Surface);
        const auto capabilities    = m_Context->physical_device().getSurfaceCapabilitiesKHR(*m_Surface);

        m_Extent = clamp_extent(m_Window->get_inner_size(), capabilities.minImageExtent, capabilities.maxImageExtent);

        m_PresentMode   = select_present_mode(present_modes, false);
        m_SurfaceFormat = select_surface_format(surface_formats);

        uint32_t min_image_count = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && min_image_count > capabilities.maxImageCount) {
            min_image_count = capabilities.maxImageCount;
        }


        vk::SwapchainCreateInfoKHR create_info = {};
        create_info.clipped                    = true;
        create_info.compositeAlpha             = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        create_info.surface                    = m_Surface;
        create_info.minImageCount              = min_image_count;
        create_info.presentMode                = m_PresentMode;
        create_info.preTransform               = capabilities.currentTransform;
        create_info.imageExtent                = m_Extent;
        create_info.imageFormat                = m_SurfaceFormat.format;
        create_info.imageColorSpace            = m_SurfaceFormat.colorSpace;
        create_info.imageUsage                 = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
        create_info.imageArrayLayers           = 1;

        std::vector<uint32_t> queue_family_indices;

        if (m_Context->are_present_render_shared()) {
            create_info.imageSharingMode = vk::SharingMode::eExclusive;
        } else {
            create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            queue_family_indices         = {m_Context->primary_queue_family(), m_Context->present_queue_family()};
        }

        create_info.setQueueFamilyIndices(queue_family_indices);
        if (m_Swapchain != nullptr) {
            create_info.oldSwapchain = *m_Swapchain;
            // TODO: signal to other systems that swapchain is being destroyed somehow. Maybe I should pull in eventpp and use something from that (or I can use some slot-type
            //       mechanic using entt or just eventpp's callbacklist).
        }

        m_Swapchain = vk::raii::SwapchainKHR(m_Context->device(), create_info);
        m_Images    = m_Swapchain.getImages();
    }

    FrameInfo Surface::begin_frame() const {
        auto       _           = m_Context->device().waitForFences(*m_InFlightFences[m_CurrentFrame], true, UINT64_MAX);
        const auto image_index = m_Swapchain.acquireNextImage(UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], nullptr).second;
        m_Context->device().resetFences(*m_InFlightFences[m_CurrentFrame]);

        return {.image      = m_Images[image_index],
                .imageIndex = image_index,
                .frameIndex = m_CurrentFrame,
                .extent     = m_Extent,
                .format     = m_SurfaceFormat.format,
                .colorSpace = m_SurfaceFormat.colorSpace,
                .syncInfo   = SyncInfo{
                      .imageAvailableSemaphore = m_ImageAvailableSemaphores[m_CurrentFrame],
                      .renderFinishedSemaphore = m_RenderFinishedSemaphores[m_CurrentFrame],
                      .inFlightFence           = m_InFlightFences[m_CurrentFrame],
                }};
    }

    void Surface::end_frame(const FrameInfo &frame_info) {
        vk::PresentInfoKHR present_info{};
        present_info.setSwapchains(*m_Swapchain);
        present_info.setWaitSemaphores(*m_RenderFinishedSemaphores[m_CurrentFrame]);
        present_info.setImageIndices(frame_info.imageIndex);

        if (m_Context->queues().present.presentKHR(present_info) == vk::Result::eSuboptimalKHR) {
            recreate_swapchain();
        }
    }
} // namespace engine
