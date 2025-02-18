#pragma once

#include "engine/application.hpp"

namespace game {

    class Game : public engine::Application {
      public:
        Game();
        ~Game() override = default;

        [[nodiscard]] std::optional<engine::crash> verify_system() const override;

        void render_frame(const vk::raii::CommandBuffer &cmd, const engine::FrameInfo &frame_info) override;
    };

} // namespace game
