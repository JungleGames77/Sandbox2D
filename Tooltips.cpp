#include "GameTypes.hpp"

namespace sandbox {
namespace {
struct Info { const char* name; const char* purpose; const char* detail; };
Info itemInfo(Block block) {
    switch (block) {
    case Grass: return {"Grass Block", "Building and landscaping block", "Hand mining time: 0.55 s"};
    case Dirt: return {"Dirt", "Used for building and landscaping", "Hand mining time: 0.50 s"};
    case Stone: return {"Stone", "Used to craft stone tools", "Stone pickaxe mining speed: 4x"};
    case Coal: return {"Coal Ore", "Resource for future crafting recipes", "Pickaxes mine this block faster"};
    case Gold: return {"Gold Ore", "Resource for future crafting recipes", "Pickaxes mine this block faster"};
    case Wood: return {"Wood Log", "Crafts into 4 wooden planks", "Axes chop this block faster"};
    case Leaves: return {"Leaves", "Decorative building block", "Breaks quickly by hand"};
    case Planks: return {"Wooden Planks", "Used to craft sticks and tools", "2 planks craft 4 sticks"};
    case Stick: return {"Stick", "Used to craft axes and pickaxes", "Crafted from 2 wooden planks"};
    case WoodenAxe: return {"Wooden Axe", "Chops wood and planks faster", "Mining speed: 2.8x | Not stackable"};
    case WoodenPickaxe: return {"Wooden Pickaxe", "Mines stone and ores faster", "Mining speed: 2.5x | Not stackable"};
    case StonePickaxe: return {"Stone Pickaxe", "Mines stone and ores faster", "Mining speed: 4x | Not stackable"};
    case StoneAxe: return {"Stone Axe", "Chops wood and planks faster", "Mining speed: 4.2x | Not stackable"};
    default: return {"", "", ""};
    }
}
sf::String utf8(const char* value) {
    const std::string bytes(value);
    return sf::String::fromUtf8(bytes.begin(), bytes.end());
}
}
void drawItemTooltip(sf::RenderWindow& window, const sf::Font& font,
                     Block block, sf::Vector2f mousePosition) {
    if (block == Air) return;
    const Info info = itemInfo(block);
    const auto makeText = [&](const char* value, unsigned size, sf::Color color) {
        sf::Text text(font, utf8(value), size);
        text.setFillColor(color);
        return text;
    };
    auto name = makeText(info.name, 18, sf::Color(255, 231, 116));
    auto purpose = makeText(info.purpose, 15, sf::Color(238, 238, 238));
    auto detail = makeText(info.detail, 14, sf::Color(172, 204, 255));
    const float width = std::max({name.getLocalBounds().size.x,
                                  purpose.getLocalBounds().size.x,
                                  detail.getLocalBounds().size.x}) + 28.f;
    const float height = 93.f;
    const auto size = window.getSize();
    const float x = std::clamp(mousePosition.x + 17.f, 4.f,
                               std::max(4.f, static_cast<float>(size.x) - width - 6.f));
    const float y = std::clamp(mousePosition.y - height - 12.f, 4.f,
                               std::max(4.f, static_cast<float>(size.y) - height - 6.f));
    sf::RectangleShape shadow({width + 4.f, height + 4.f});
    shadow.setPosition({x + 3.f, y + 3.f});
    shadow.setFillColor(sf::Color(0, 0, 0, 110));
    window.draw(shadow);
    sf::RectangleShape background({width, height});
    background.setPosition({x, y});
    background.setFillColor(sf::Color(17, 12, 28, 244));
    background.setOutlineThickness(2.f);
    background.setOutlineColor(sf::Color(90, 66, 155));
    window.draw(background);
    name.setPosition({x + 12.f, y + 8.f});
    purpose.setPosition({x + 12.f, y + 36.f});
    detail.setPosition({x + 12.f, y + 62.f});
    window.draw(name); window.draw(purpose); window.draw(detail);
}
}
