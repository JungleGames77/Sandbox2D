#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace sandbox {
constexpr int TILE = 32;
constexpr int WORLD_W = 240;
constexpr int WORLD_H = 90;
constexpr unsigned WINDOW_W = 1280;
constexpr unsigned WINDOW_H = 720;
constexpr float PLAYER_W = 24.f;
constexpr float PLAYER_H = 48.f;
constexpr float MOVE_SPEED = 235.f;
constexpr float JUMP_SPEED = 490.f;
constexpr float GRAVITY = 1250.f;
constexpr float REACH = 6.f * TILE;

enum Block : std::uint8_t { Air, Grass, Dirt, Stone, Coal, Gold, Wood, Leaves, Planks, Stick, WoodenAxe, WoodenPickaxe, StonePickaxe, StoneAxe };
using World = std::vector<Block>;
constexpr int HOTBAR_SIZE = 9;
constexpr int STORAGE_SIZE = 27;
constexpr int INVENTORY_SIZE = HOTBAR_SIZE + STORAGE_SIZE;
constexpr int MAX_STACK = 999;
struct ItemStack { Block block = Air; int count = 0; };
using Inventory = std::array<ItemStack, INVENTORY_SIZE>;

struct Player {
    sf::Vector2f position{ 200.f, 100.f };
    sf::Vector2f velocity{ 0.f, 0.f };
    bool grounded = false;
};

using Textures = std::array<sf::Texture, 14>;

bool isTool(Block b);
int maxStack(Block b);
int findStack(const Inventory& inventory, Block block);
int findEmptySlot(const Inventory& inventory);
ItemStack craftingResult(const std::array<ItemStack, 9>& crafting);
int index(int x, int y);
bool inside(int x, int y);
Block get(const World& world, int x, int y);
bool solid(const World& world, int x, int y);
World generateWorld();
sf::FloatRect playerBounds(const Player& player);
bool collides(const World& world, const sf::FloatRect& box);
void moveAxis(Player& player, const World& world, float amount, bool horizontal);
std::filesystem::path assetDirectory();
float miningDuration(Block block, Block equipped);
void drawItemTooltip(sf::RenderWindow& window, const sf::Font& font,
                     Block block, sf::Vector2f mousePosition);
void drawMiningCracks(sf::RenderWindow& window, int x, int y, float progress);
sf::Color blockColor(Block block);
void drawNumber(sf::RenderWindow& window, int value, float right, float bottom, float pixel = 2.f);
void drawBlock(sf::RenderWindow& window, Block block, float x, float y,
               const Textures& textures, const std::array<bool, 14>& loaded);
}
