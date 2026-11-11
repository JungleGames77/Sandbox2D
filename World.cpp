#include "GameTypes.hpp"

namespace sandbox {
int index(int x, int y) { return y * WORLD_W + x; }

bool inside(int x, int y) { return x >= 0 && x < WORLD_W && y >= 0 && y < WORLD_H; }

Block get(const World& world, int x, int y) {
    if (x < 0 || x >= WORLD_W || y >= WORLD_H) return Stone;
    if (y < 0) return Air;
    return world[index(x, y)];
}

bool solid(const World& world, int x, int y) { const Block block = get(world, x, y);
    return block != Air && block != Wood && block != Leaves && block != Stick && !isTool(block); }

World generateWorld() {
    World world(WORLD_W * WORLD_H, Air);
    std::mt19937 rng(2026);
    std::uniform_int_distribution<int> jitter(-1, 1);
    int ground = 32;
    for (int x = 0; x < WORLD_W; ++x) {
        if (x % 3 == 0) ground = std::clamp(ground + jitter(rng), 24, 39);
        const int height = std::clamp(ground + static_cast<int>(std::sin(x * 0.085f) * 3.f), 20, 45);
        for (int y = height; y < WORLD_H; ++y) {
            world[index(x, y)] = (y == height) ? Grass : (y < height + 5 ? Dirt : Stone);
        }
    }

    std::uniform_real_distribution<float> chance(0.f, 1.f);
    for (int y = 0; y < WORLD_H; ++y) {
        for (int x = 0; x < WORLD_W; ++x) {
            if (world[index(x, y)] != Stone) continue;
            const float roll = chance(rng);
            if (y >= 45 && roll < 0.012f) world[index(x, y)] = Gold;
            else if (y >= 30 && roll < 0.045f) world[index(x, y)] = Coal;
        }
    }

    std::uniform_int_distribution<int> treeGap(7, 13);
    std::uniform_int_distribution<int> treeHeight(4, 7);
    for (int x = 12 + treeGap(rng); x < WORLD_W - 4; x += treeGap(rng)) {
        int surface = 0;
        while (surface < WORLD_H && world[index(x, surface)] == Air) ++surface;
        if (surface < 10 || surface >= WORLD_H || world[index(x, surface)] != Grass) continue;
        const int height = treeHeight(rng);
        const int top = surface - height;
        if (top - 3 < 0) continue;

        bool clear = true;
        for (int yy = top - 2; yy < surface && clear; ++yy)
            for (int xx = x - 3; xx <= x + 3; ++xx)
                if (!inside(xx, yy) || world[index(xx, yy)] != Air) { clear = false; break; }
        if (!clear) continue;
        for (int yy = surface - 1; yy >= top; --yy) world[index(x, yy)] = Wood;

        for (int dy = -2; dy <= 1; ++dy) {
            const int radius = (dy == -2) ? 1 : (dy == 1 ? 2 : 3);
            for (int dx = -radius; dx <= radius; ++dx) {
                const int xx = x + dx, yy = top + dy;
                if (inside(xx, yy) && world[index(xx, yy)] == Air)
                    world[index(xx, yy)] = Leaves;
            }
        }
    }
    return world;
}
}
