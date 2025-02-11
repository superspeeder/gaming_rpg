#pragma once

#include <optional>
#include <string>

#include "engine/tools.hpp"
#include "engine/window.hpp"
#include <spdlog/spdlog.h>

namespace engine {

    class Application {
      public:
        Application();
        virtual ~Application();

        virtual std::optional<crash> verify_system() const;

        void run();

      private:
        void _verify_system() const;
    };

    void run_application(const std::shared_ptr<Application> &app);
} // namespace engine
