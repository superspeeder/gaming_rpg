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

        virtual std::optional<crash> verify_system() const;

        void run();

        virtual void render_frame(const vk::raii::CommandBuffer &cmd, const FrameInfo &frame_info) = 0;

      private:
        void internal_verify_system() const;
        void build_context();

        void internal_render_frame();


        vk::raii::CommandPool    m_CommandPool    = nullptr;
        vk::raii::CommandBuffers m_CommandBuffers = nullptr;

        std::shared_ptr<EngineContext> m_EngineContext;
    };

    void run(const std::shared_ptr<Application> &app);
} // namespace engine
