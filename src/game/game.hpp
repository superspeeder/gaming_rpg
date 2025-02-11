#pragma once

#include "engine/application.hpp"

namespace game {

    class Game : public engine::Application {
      public:
        Game();
        ~Game() override = default;

        std::optional<engine::crash> verifySystem() const override;
    };

} // namespace game
