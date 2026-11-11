#include "GameTypes.hpp"

namespace sandbox {
constexpr std::array<std::array<const char*, 5>, 10> DIGITS{{
    {{"111","101","101","101","111"}},
    {{"010","110","010","010","111"}},
    {{"111","001","111","100","111"}},
    {{"111","001","111","001","111"}},
    {{"101","101","111","001","001"}},
    {{"111","100","111","001","111"}},
    {{"111","100","111","101","111"}},
    {{"111","001","001","001","001"}},
    {{"111","101","111","101","111"}},
    {{"111","101","111","001","111"}}
}};

void drawNumber(sf::RenderWindow& window, int value, float right, float bottom, float pixel) {
    const std::string str = std::to_string(value);
    const float width = static_cast<float>(str.size() * 4 - 1) * pixel;
    float x0 = right - width;
    for (char c : str) {
        const auto& glyph = DIGITS[static_cast<std::size_t>(c - '0')];
        for (int y = 0; y < 5; ++y)
            for (int x = 0; x < 3; ++x)
                if (glyph[y][x] == '1') {
                    sf::RectangleShape shadow({pixel + 0.5f, pixel + 0.5f});
                    shadow.setPosition({x0 + x * pixel + 1.f, bottom - 5 * pixel + y * pixel + 1.f});
                    shadow.setFillColor(sf::Color::Black);
                    window.draw(shadow);
                    sf::RectangleShape dot({pixel, pixel});
                    dot.setPosition({x0 + x * pixel, bottom - 5 * pixel + y * pixel});
                    dot.setFillColor(sf::Color::White);
                    window.draw(dot);
                }
        x0 += 4.f * pixel;
    }
}

sf::Color blockColor(Block block) {
    switch (block) {
        case Grass: return sf::Color(78, 169, 72);
        case Dirt: return sf::Color(133, 91, 57);
        case Stone: return sf::Color(112, 119, 126);
        case Coal: return sf::Color(58, 59, 62);
        case Gold: return sf::Color(228, 193, 47);
        case Wood: return sf::Color(112, 77, 40);
        case Leaves: return sf::Color(75, 157, 68);
        case Planks: return sf::Color(205, 181, 113);
        case Stick: return sf::Color(125, 87, 41);
        case WoodenAxe: return sf::Color(142, 102, 48);
        case WoodenPickaxe: return sf::Color(142, 102, 48);
        case StonePickaxe: case StoneAxe: return sf::Color(155, 155, 155);
        default: return sf::Color::Transparent;
    }
}

void drawMiningCracks(sf::RenderWindow& window, int x, int y, float progress) {
    const float px = x * static_cast<float>(TILE);
    const float py = y * static_cast<float>(TILE);
    const int stage = std::clamp(static_cast<int>(progress * 10.f), 0, 9);
    sf::RectangleShape shade({static_cast<float>(TILE), static_cast<float>(TILE)});
    shade.setPosition({px, py});
    shade.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(18 + stage * 8)));
    window.draw(shade);

    constexpr std::array<std::array<float, 4>, 15> segments{{
        {{16, 15, 12, 11}}, {{16, 15, 21, 11}}, {{16, 15, 17, 21}},
        {{12, 11, 8, 10}}, {{21, 11, 26, 8}}, {{17, 21, 12, 25}},
        {{8, 10, 4, 5}}, {{26, 8, 29, 3}}, {{12, 25, 9, 30}},
        {{17, 21, 24, 24}}, {{24, 24, 29, 29}}, {{12, 11, 13, 4}},
        {{21, 11, 22, 5}}, {{17, 21, 18, 29}}, {{8, 10, 3, 15}}
    }};
    const int visible = std::min(static_cast<int>(segments.size()), 2 + stage * 2);
    for (int i = 0; i < visible; ++i) {
        const auto& a = segments[static_cast<std::size_t>(i)];
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = {px + a[0], py + a[1]};
        line[1].position = {px + a[2], py + a[3]};
        line[0].color = sf::Color(18, 14, 12, 230);
        line[1].color = sf::Color(18, 14, 12, 230);
        window.draw(line);
    }
}

void drawBlock(sf::RenderWindow& window, Block block, float x, float y,
               const Textures& textures, const std::array<bool, 14>& loaded) {
    const auto id = static_cast<std::size_t>(block);
    if (id > 0 && id < textures.size() && loaded[id]) {
        sf::Sprite sprite(textures[id]);
        sprite.setPosition({x, y});
        const auto sz = textures[id].getSize();
        sprite.setScale({static_cast<float>(TILE) / sz.x, static_cast<float>(TILE) / sz.y});
        window.draw(sprite);
    } else {
        sf::RectangleShape tile({static_cast<float>(TILE), static_cast<float>(TILE)});
        tile.setPosition({x, y});
        tile.setFillColor(blockColor(block));
        window.draw(tile);
    }
}
}
