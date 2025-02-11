#include "engine_context.hpp"

#include <stdexcept>

namespace engine {
    EngineContext::EngineContext() {}

    std::unique_ptr<Window> EngineContext::createDummyWindow() const {
        throw std::logic_error("Not implemented");
    }
} // engine