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
        try {} catch (vk::OutOfDateKHRError& error) {
            recreate_swapchains();
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
