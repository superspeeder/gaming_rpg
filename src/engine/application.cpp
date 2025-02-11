#include "application.hpp"

#include <GLFW/glfw3.h>

namespace engine {
    Application::Application() {}

    Application::~Application() {}

    std::optional<crash> Application::verifySystem() const {
        return std::nullopt;
    }

    void Application::run() {
        internalVerifySystem();
        buildContext();
    }

    void Application::internalVerifySystem() const {
        if (!glfwVulkanSupported()) {
            throw crash(CrashReason::UnsupportedSystem, "System unsupported! Your GPU must support Vulkan and your system must have Vulkan drivers installed for it.");
        }

        if (auto r = verifySystem(); r.has_value()) {
            throw r.value();
        }
    }

    void Application::buildContext() {
        m_EngineContext = std::make_shared<EngineContext>();
    }

    void run(const std::shared_ptr<Application> &app) {
        try {
            app->run();
        } catch (crash& c) {
            spdlog::critical(c.what());
            errorPopup(c.message);
            std::exit(-1);
        } catch (std::exception& e) {
            spdlog::critical(e.what());
            errorPopup(e.what());
            std::exit(-1);
        }
    }
} // namespace engine
