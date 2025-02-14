#include "game.hpp"

namespace game {
    Game::Game() {}

    std::optional<engine::crash> Game::verify_system() const {
        return std::nullopt;
    }
} // game