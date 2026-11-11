#include "GameTypes.hpp"

namespace sandbox {
bool isTool(Block b) { return b == WoodenAxe || b == WoodenPickaxe || b == StonePickaxe || b == StoneAxe; }

int maxStack(Block b) { return isTool(b) ? 1 : MAX_STACK; }

int findStack(const Inventory& inventory, Block block) {
    for (int i = 0; i < INVENTORY_SIZE; ++i)
        if (inventory[i].block == block && inventory[i].count > 0 && inventory[i].count < maxStack(block)) return i;
    return -1;
}

int findEmptySlot(const Inventory& inventory) {
    for (int i = 0; i < INVENTORY_SIZE; ++i)
        if (inventory[i].count == 0) return i;
    return -1;
}
ItemStack craftingResult(const std::array<ItemStack, 9>& crafting) {
        int occupied = 0;
        int logSlot = -1;
        for (int i = 0; i < 9; ++i) {
            if (crafting[i].count <= 0) continue;
            ++occupied;
            if (crafting[i].block == Wood) logSlot = i;
        }
        if (occupied == 1 && logSlot >= 0) return {Planks, 4};
        if (occupied == 2) {
            for (int row = 0; row < 2; ++row) {
                for (int col = 0; col < 3; ++col) {
                    const int a = row * 3 + col;
                    const int b = (row + 1) * 3 + col;
                    if (crafting[a].block == Planks && crafting[a].count > 0 &&
                        crafting[b].block == Planks && crafting[b].count > 0)
                        return {Stick, 4};
                }
            }
        }

        if (occupied == 5) {
            const auto matches = [&](const std::array<Block, 9>& pattern) -> bool {
                for (int i = 0; i < 9; ++i) {
                    const Block actual = crafting[i].count > 0 ? crafting[i].block : Air;
                    if (actual != pattern[i]) return false;
                }
                return true;
            };
            for (Block material : {Planks, Stone}) {
                const Block axe = material == Planks ? WoodenAxe : StoneAxe;
                const Block pickaxe = material == Planks ? WoodenPickaxe : StonePickaxe;
                if (matches({material, material, material, Air, Stick, Air, Air, Stick, Air}))
                    return {pickaxe, 1};
                if (matches({material, material, Air, material, Stick, Air, Air, Stick, Air}) ||
                    matches({Air, material, material, Air, Stick, material, Air, Stick, Air}))
                    return {axe, 1};
            }
        }
        return {};
    }

}
