#include "application.hpp"

#include <GLFW/glfw3.h>

namespace engine {
    Application::Application() {}

    Application::~Application() {}

    std::optional<crash> Application::verify_system() const {
        return std::nullopt;
    }

    void Application::run() {
        glfwInit();

        internal_verify_system();
        build_context();

        m_Window = std::make_unique<Window>(WindowAttributes{"Hello!", {800, 600}, true, false});
        m_Window->create_surface(m_EngineContext->vulkan());

        m_CommandPool = vk::raii::CommandPool(m_EngineContext->vulkan()->device(), vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_EngineContext->vulkan()->primary_queue_family()));
        m_CommandBuffers = vk::raii::CommandBuffers(m_EngineContext->vulkan()->device(), vk::CommandBufferAllocateInfo(*m_CommandPool, vk::CommandBufferLevel::ePrimary, Surface::MAX_FRAMES_IN_FLIGHT));
    }

    void Application::internal_verify_system() const {
        if (!glfwVulkanSupported()) {
            throw crash(CrashReason::UnsupportedSystem, "System unsupported! Your GPU must support Vulkan and your system must have Vulkan drivers installed for it.");
        }

        if (auto r = verify_system(); r.has_value()) {
            throw r.value();
        }
    }

    void Application::build_context() {
        m_EngineContext = EngineContext::create();
    }

    void Application::internal_render_frame() {
        try {
            const auto frame_info = m_Window->get_surface()->begin_frame();

            const auto &cmd = m_CommandBuffers[frame_info.frame_index];
            cmd.reset();
            cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

            render_frame(cmd, frame_info);

            cmd.end();

            vk::SemaphoreSubmitInfo ia_sem{*frame_info.sync_info.image_available_semaphore, 0, vk::PipelineStageFlagBits2::eTopOfPipe};
            vk::SemaphoreSubmitInfo rf_sem{*frame_info.sync_info.render_finished_semaphore, 0, vk::PipelineStageFlagBits2::eBottomOfPipe};
            vk::CommandBufferSubmitInfo cbsi{*cmd, 0};

            vk::SubmitInfo2 si{{}, ia_sem, cbsi, rf_sem};
            m_EngineContext->vulkan()->queues().primary.main.submit2(si, frame_info.sync_info.in_flight_fence);

            m_Window->get_surface()->end_frame(frame_info);


        } catch (vk::OutOfDateKHRError& error) {
            m_Window->get_surface()->recreate_swapchain();
        }
    }

    void run(const std::shared_ptr<Application> &app) {
        try {
            app->run();
        } catch (crash& c) {
            spdlog::critical(c.what());
            error_popup(c.message);
            std::exit(-1);
        } catch (std::exception& e) {
            spdlog::critical(e.what());
            error_popup(e.what());
            std::exit(-1);
        }
    }
} // namespace engine
