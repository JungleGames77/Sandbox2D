#include "GameTypes.hpp"

namespace sandbox {
sf::FloatRect playerBounds(const Player& player) {
    return {{player.position.x, player.position.y}, {PLAYER_W, PLAYER_H}};
}

bool collides(const World& world, const sf::FloatRect& box) {
    const int left = static_cast<int>(std::floor(box.position.x / TILE));
    const int right = static_cast<int>(std::floor((box.position.x + box.size.x - 0.01f) / TILE));
    const int top = static_cast<int>(std::floor(box.position.y / TILE));
    const int bottom = static_cast<int>(std::floor((box.position.y + box.size.y - 0.01f) / TILE));
    for (int y = top; y <= bottom; ++y)
        for (int x = left; x <= right; ++x)
            if (solid(world, x, y)) return true;
    return false;
}

void moveAxis(Player& player, const World& world, float amount, bool horizontal) {
    if (amount == 0.f) return;

    const int steps = std::max(1, static_cast<int>(std::ceil(std::abs(amount) / 4.f)));
    const float step = amount / steps;
    for (int i = 0; i < steps; ++i) {
        if (horizontal) player.position.x += step;
        else player.position.y += step;
        if (collides(world, playerBounds(player))) {
            if (horizontal) {
                player.position.x -= step;
                player.velocity.x = 0.f;
            } else {
                player.position.y -= step;
                if (step > 0.f) player.grounded = true;
                player.velocity.y = 0.f;
            }
            break;
        }
    }
}
}
