#include "game.hpp"

namespace game {
    Game::Game() {}

    std::optional<engine::crash> Game::verifySystem() const {
        return std::nullopt;
    }
} // game