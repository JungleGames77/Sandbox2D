#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "SaveSystem.hpp"
namespace sandbox {
enum class MenuScreen { Main, Worlds, Naming, Playing };

int menuHit(MenuScreen screen,sf::Vector2f mouse,sf::Vector2u size,std::size_t worldCount,int scroll);
void drawMenu(sf::RenderWindow& window,MenuScreen screen,const sf::Texture* background,
              const sf::Texture* logo,const sf::Font* font,const std::vector<SavedWorldInfo>& worlds,
              int scroll,const sf::String& worldName,const std::string& status);
}
