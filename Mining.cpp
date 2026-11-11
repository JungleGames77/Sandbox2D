#include "GameTypes.hpp"

namespace sandbox {
float miningDuration(Block block, Block equipped) {
    float base = 0.f;
    switch (block) {
        case Leaves: base = 0.22f; break;
        case Grass: base = 0.55f; break;
        case Dirt: base = 0.50f; break;
        case Wood: base = 1.15f; break;
        case Planks: base = 0.85f; break;
        case Stone: base = 1.80f; break;
        case Coal: base = 2.10f; break;
        case Gold: base = 2.50f; break;
        default: return 0.f;
    }

    const bool woodLike = block == Wood || block == Planks || block == Leaves;
    const bool stoneLike = block == Stone || block == Coal || block == Gold;
    float speed = 1.f;
    if (woodLike && equipped == WoodenAxe) speed = 2.8f;
    else if (woodLike && equipped == StoneAxe) speed = 4.2f;
    else if (stoneLike && equipped == WoodenPickaxe) speed = 2.5f;
    else if (stoneLike && equipped == StonePickaxe) speed = 4.0f;
    return base / speed;
}
}
