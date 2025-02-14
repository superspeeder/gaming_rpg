#pragma once

#include "engine/application.hpp"

namespace game {

    class Game : public engine::Application {
      public:
        Game();
        ~Game() override = default;

        std::optional<engine::crash> verify_system() const override;
    };

} // namespace game
