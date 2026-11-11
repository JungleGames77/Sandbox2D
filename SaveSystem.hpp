#pragma once
#include "GameTypes.hpp"
#include <string>
#include <vector>
namespace sandbox {
struct SavedWorldInfo { std::string id, name; };
struct WorldSave {
    World world;
    Player player;
    Inventory inventory{};
    std::array<ItemStack,9> crafting{};
    ItemStack cursor{};
    int selectedSlot=0;
    float cameraZoom=1.f;
};
std::vector<SavedWorldInfo> listWorlds();
std::string createWorldId();
bool saveWorld(const std::string& id,const std::string& name,const WorldSave& state);
bool loadWorld(const std::string& id,std::string& name,WorldSave& state);
}
