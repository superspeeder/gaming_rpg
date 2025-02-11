#pragma once

#include <optional>
#include <string>

#include "engine/tools.hpp"
#include "engine/window.hpp"

#include <spdlog/spdlog.h>

#include "engine/engine_context.hpp"

namespace engine {

    class Application {
      public:
        Application();
        virtual ~Application();

        virtual std::optional<crash> verifySystem() const;

        void run();

      private:
        void internalVerifySystem() const;
        void buildContext();


        std::shared_ptr<EngineContext> m_EngineContext;
    };

    void run(const std::shared_ptr<Application> &app);
} // namespace engine
