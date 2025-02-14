#include "engine_context.hpp"

#include <stdexcept>

namespace engine {
    EngineContext::EngineContext() = default;

    void EngineContext::init() {
        m_VulkanContext = VulkanContext::create(shared_from_this());
    }

    std::unique_ptr<Window> EngineContext::create_dummy_window() const {
        return std::make_unique<Window>(WindowAttributes{"", {1, 1}, false, false});
    }

    const DebugSettings &EngineContext::debug_settings() const {
        return m_DebugSettings;
    }
} // engine
