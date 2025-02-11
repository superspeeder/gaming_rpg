#include "game.hpp"

int main() {
    const auto game = std::make_shared<game::Game>();
    engine::run(game);
    return 0;
}
