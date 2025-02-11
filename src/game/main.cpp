#include "game.hpp"

int main() {
    const auto game = std::make_shared<game::Game>();
    engine::run_application(game);
    return 0;
}
