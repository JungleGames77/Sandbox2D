#include "Menu.hpp"
#include "SaveSystem.hpp"
#include "GameTypes.hpp"

using namespace sandbox;

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Sandbox2D | v0.7.1");
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(60);
    Textures textures;
    std::array<bool, 14> loaded{};
    const std::array<std::string, 14> names{"", "Grass.PNG", "Dirt.PNG", "Stone.PNG", "Coat.PNG", "Gold.PNG", "tree.png", "Leaves.PNG", "Wood.PNG", "Stick.png", "Wooden_Axe.png", "Wooden_Pick.png", "Stone_Pickax.png", "Stone_Axe.png"};
    const auto assets = assetDirectory();
    for (std::size_t i = 1; i < names.size(); ++i) {
        loaded[i] = textures[i].loadFromFile((assets / names[i]).string());
        if (loaded[i]) textures[i].setSmooth(false);
    }
    std::array<sf::Texture, 5> steveTextures;
    std::array<bool, 5> steveLoaded{};
    for (std::size_t i = 0; i < steveTextures.size(); ++i) {
        steveLoaded[i] = steveTextures[i].loadFromFile(
            (assets / ("Steve_" + std::to_string(i) + ".png")).string());
        if (steveLoaded[i]) steveTextures[i].setSmooth(false);
    }
    sf::Texture saveIconTexture;
    const bool saveIconLoaded = saveIconTexture.loadFromFile((assets / "SaveIcon.png").string());
    if (saveIconLoaded) saveIconTexture.setSmooth(false);
    sf::Texture crosshairTexture;
    const bool crosshairLoaded = crosshairTexture.loadFromFile((assets / "Crosshair.png").string());
    if (crosshairLoaded) crosshairTexture.setSmooth(false);

    sf::Font tooltipFont;
    bool tooltipFontLoaded = false;
    for (const char* path : {"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"}) {
        if (tooltipFont.openFromFile(path)) { tooltipFontLoaded = true; break; }
    }
    sf::Texture menuBackgroundTexture;
    sf::Texture menuLogoTexture;
    const bool menuBackgroundLoaded = menuBackgroundTexture.loadFromFile((assets / "MenuDirt.png").string());
    const bool menuLogoLoaded = menuLogoTexture.loadFromFile((assets / "MenuLogo.png").string());
    MenuScreen menuScreen = MenuScreen::Main;
    std::vector<SavedWorldInfo> savedWorlds = listWorlds();
    int worldScroll = 0;
    sf::String typedWorldName = "New World";
    std::string currentWorldId, currentWorldName, saveStatus;
    sf::Clock autoSaveClock;
    sf::Clock saveIndicatorClock;
    bool showAutoSaveIndicator = false;
    bool hasWorld = false;
    sf::Clock tooltipClock;
    int lastHoverArea = -1;
    int lastHoverSlot = -1;
    Block lastHoverBlock = Air;
    float walkTimer = 0.f;
    bool facingRight = true;
    World world = generateWorld();
    Player player;
    const int spawnX = 7;
    int surface = 0;
    while (surface < WORLD_H && get(world, spawnX, surface) == Air) ++surface;
    const sf::Vector2f spawn{spawnX * static_cast<float>(TILE), surface * static_cast<float>(TILE) - PLAYER_H - 2.f};
    player.position = spawn;
    sf::View camera(sf::FloatRect({0.f, 0.f}, {static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H)}));

    float cameraZoom = 1.f;
    const auto updateCameraSize = [&]() {
        const auto size = window.getSize();
        camera.setSize({size.x * cameraZoom, size.y * cameraZoom});
    };

    sf::View uiView(sf::FloatRect({0.f, 0.f},
        {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)}));
    sf::Clock clock;
    Inventory inventory{};

    int selectedSlot = 0;
    Block selected = Air;
    bool inventoryOpen = false;
    ItemStack cursorStack{};
    std::array<ItemStack, 9> crafting{};

    bool jumpHeld = false;
    bool leftHeld = false;
    bool rightHeld = false;
    int miningX = -1;
    int miningY = -1;
    Block miningBlock = Air;
    float miningProgress = 0.f;
    const auto resetMining = [&]() {
        miningX = -1; miningY = -1; miningBlock = Air; miningProgress = 0.f;
    };
    const auto saveCurrentWorld = [&]() {
        if (!hasWorld || currentWorldId.empty()) return true;
        WorldSave state;
        state.world=world; state.player=player; state.inventory=inventory;
        state.crafting=crafting; state.cursor=cursorStack;
        state.selectedSlot=selectedSlot; state.cameraZoom=cameraZoom;
        const bool ok=saveWorld(currentWorldId,currentWorldName,state);
        if (!ok) saveStatus="ERROR: Could not save world!";
        else {saveStatus.clear();autoSaveClock.restart();}
        return ok;
    };
    const auto startNewWorld = [&]() {
        const auto utf8=typedWorldName.toUtf8();
        std::string name(utf8.begin(),utf8.end());
        if(name.empty() || name.size()>160){saveStatus="Enter a world name (max 160 bytes)";return;}
        const auto id=createWorldId();
        if(id.empty()){saveStatus="Could not create world ID";return;}
        world=generateWorld();player=Player{};
        int newSurface=0;
        while(newSurface<WORLD_H && get(world,spawnX,newSurface)==Air)++newSurface;
        player.position={spawnX*float(TILE),newSurface*float(TILE)-PLAYER_H-2.f};
        inventory={};crafting={};cursorStack={};selectedSlot=0;selected=Air;
        inventoryOpen=false;cameraZoom=1.f;updateCameraSize();resetMining();
        currentWorldId=id;currentWorldName=name;hasWorld=true;
        if(!saveCurrentWorld()){hasWorld=false;return;}
        savedWorlds=listWorlds();worldScroll=0;
        menuScreen=MenuScreen::Playing;clock.restart();
    };
    const auto openSavedWorld = [&](int i) {
        if(i<0||i>=static_cast<int>(savedWorlds.size()))return;
        WorldSave state;std::string name;
        if(!loadWorld(savedWorlds[i].id,name,state)){saveStatus="ERROR: Cannot load world";return;}
        world=std::move(state.world);player=state.player;inventory=state.inventory;
        crafting=state.crafting;cursorStack=state.cursor;selectedSlot=state.selectedSlot;
        cameraZoom=state.cameraZoom;updateCameraSize();resetMining();
        currentWorldId=savedWorlds[i].id;currentWorldName=name;
        inventoryOpen=false;hasWorld=true;saveStatus.clear();
        menuScreen=MenuScreen::Playing;autoSaveClock.restart();clock.restart();
    };

    while (window.isOpen()) {
        const float dt = std::min(clock.restart().asSeconds(), 1.f / 30.f);
        while (const auto event = window.pollEvent()) {
            if (menuScreen != MenuScreen::Playing) {
                if(event->is<sf::Event::Closed>()){window.close();continue;}
                if(const auto* resized=event->getIf<sf::Event::Resized>()){
                    if(resized->size.x>0&&resized->size.y>0){updateCameraSize();uiView.setSize({float(resized->size.x),float(resized->size.y)});uiView.setCenter({resized->size.x*.5f,resized->size.y*.5f});}
                }
                if(const auto* wheel=event->getIf<sf::Event::MouseWheelScrolled>()){
                    if(menuScreen==MenuScreen::Worlds){
                        worldScroll=std::clamp(worldScroll+(wheel->delta<0?1:-1),0,std::max(0,static_cast<int>(savedWorlds.size())-5));
                    }
                }
                if(const auto* key=event->getIf<sf::Event::KeyPressed>()){
                    if(key->code==sf::Keyboard::Key::Escape){
                        if(menuScreen==MenuScreen::Naming)menuScreen=MenuScreen::Worlds;
                        else if(menuScreen==MenuScreen::Worlds)menuScreen=MenuScreen::Main;
                    }
                    if(menuScreen==MenuScreen::Naming){
                        if(key->code==sf::Keyboard::Key::Backspace && !typedWorldName.isEmpty())typedWorldName.erase(typedWorldName.getSize()-1);
                        if(key->code==sf::Keyboard::Key::Enter)startNewWorld();
                    }
                }
                if(const auto* typed=event->getIf<sf::Event::TextEntered>()){
                    if(menuScreen==MenuScreen::Naming&&typed->unicode>=32&&typed->unicode!=127&&typedWorldName.getSize()<36){
                        typedWorldName+=typed->unicode;
                    }
                }
                if(const auto* click=event->getIf<sf::Event::MouseButtonPressed>()){
                    if(click->button==sf::Mouse::Button::Left){
                        const int hit=menuHit(menuScreen,{float(click->position.x),float(click->position.y)},window.getSize(),savedWorlds.size(),worldScroll);
                        if(menuScreen==MenuScreen::Main){
                            if(hit==0){savedWorlds=listWorlds();worldScroll=0;saveStatus.clear();menuScreen=MenuScreen::Worlds;}
                            else if(hit==1)window.close();
                        }else if(menuScreen==MenuScreen::Worlds){
                            const int visible=std::min<int>(5,static_cast<int>(savedWorlds.size())-worldScroll);
                            if(hit==0){typedWorldName="New World";saveStatus.clear();menuScreen=MenuScreen::Naming;}
                            else if(hit>=1&&hit<=visible)openSavedWorld(worldScroll+hit-1);
                            else if(hit==visible+1)menuScreen=MenuScreen::Main;
                        }else if(menuScreen==MenuScreen::Naming){
                            if(hit==0)startNewWorld();
                            else if(hit==1)menuScreen=MenuScreen::Worlds;
                        }
                    }
                }
                continue;
            }

            if (event->is<sf::Event::Closed>()) {saveCurrentWorld();window.close();}
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                if (resized->size.x > 0 && resized->size.y > 0) {
                    updateCameraSize();
                    uiView.setSize({static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)});
                    uiView.setCenter({resized->size.x * 0.5f, resized->size.y * 0.5f});
                }
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    if (inventoryOpen) { if (cursorStack.count == 0) inventoryOpen = false; }
                    else { if(saveCurrentWorld()){menuScreen = MenuScreen::Main;resetMining();} }
                }

                if (key->code == sf::Keyboard::Key::Up) {
                    cameraZoom = std::max(0.5f, cameraZoom / 1.15f);
                    updateCameraSize();
                }
                if (key->code == sf::Keyboard::Key::Down) {
                    cameraZoom = std::min(2.5f, cameraZoom * 1.15f);
                    updateCameraSize();
                }
                if (key->code == sf::Keyboard::Key::E) {
                    if (!inventoryOpen || cursorStack.count == 0) inventoryOpen = !inventoryOpen;
                }
                const std::array<sf::Keyboard::Key, 9> hotkeys{
                    sf::Keyboard::Key::Num1, sf::Keyboard::Key::Num2,
                    sf::Keyboard::Key::Num3, sf::Keyboard::Key::Num4,
                    sf::Keyboard::Key::Num5, sf::Keyboard::Key::Num6,
                    sf::Keyboard::Key::Num7, sf::Keyboard::Key::Num8,
                    sf::Keyboard::Key::Num9
                };
                for (std::size_t i = 0; i < hotkeys.size(); ++i)
                    if (key->code == hotkeys[i]) selectedSlot = static_cast<int>(i);
            }
            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->wheel == sf::Mouse::Wheel::Vertical && wheel->delta != 0.f) {
                    const int steps = std::max(1, static_cast<int>(std::abs(wheel->delta)));
                    const int direction = wheel->delta > 0.f ? -1 : 1;
                    for (int n = 0; n < steps; ++n)
                        selectedSlot = (selectedSlot + direction + HOTBAR_SIZE) % HOTBAR_SIZE;
                }
            }
        }
        if (menuScreen != MenuScreen::Playing) {
            if (!window.isOpen()) break;
            drawMenu(window, menuScreen,
                menuBackgroundLoaded ? &menuBackgroundTexture : nullptr,
                menuLogoLoaded ? &menuLogoTexture : nullptr,
                tooltipFontLoaded ? &tooltipFont : nullptr,
                savedWorlds,worldScroll,typedWorldName,saveStatus);
            clock.restart();
            continue;
        }
        if (autoSaveClock.getElapsedTime().asSeconds() >= 30.f) {
            if (saveCurrentWorld()) {
                showAutoSaveIndicator = true;
                saveIndicatorClock.restart();
            }
        }
        selected = inventory[selectedSlot].count > 0 ? inventory[selectedSlot].block : Air;
        if (!window.isOpen()) break;

        const bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        const bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
        const bool jump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
                          sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        player.velocity.x = inventoryOpen ? 0.f : (static_cast<int>(moveRight) - static_cast<int>(moveLeft)) * MOVE_SPEED;
        if (moveRight != moveLeft) facingRight = moveRight;
        if (player.grounded && moveRight != moveLeft) walkTimer += dt;
        else walkTimer = 0.f;
        if (!inventoryOpen && jump && !jumpHeld && player.grounded) {
            player.velocity.y = -JUMP_SPEED;
            player.grounded = false;
        }
        jumpHeld = jump;
        player.velocity.y = std::min(player.velocity.y + GRAVITY * dt, 900.f);
        moveAxis(player, world, player.velocity.x * dt, true);
        player.grounded = false;
        moveAxis(player, world, player.velocity.y * dt, false);
        if (player.position.y > WORLD_H * TILE + 100.f) {
            player.position = spawn;
            player.velocity = {0.f, 0.f};
        }

        const sf::Vector2f viewSize = camera.getSize();
        const float halfX = viewSize.x / 2.f;
        const float halfY = viewSize.y / 2.f;
        const float worldPixelWidth = WORLD_W * static_cast<float>(TILE);
        const float worldPixelHeight = WORLD_H * static_cast<float>(TILE);
        const float cx = std::clamp(player.position.x + PLAYER_W / 2.f, halfX, std::max(halfX, worldPixelWidth - halfX));
        const float cy = std::clamp(player.position.y + PLAYER_H / 2.f, halfY, std::max(halfY, worldPixelHeight - halfY));
        camera.setCenter({cx, cy});
        window.setView(camera);

        const sf::Vector2f mouseWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        const int mx = static_cast<int>(std::floor(mouseWorld.x / TILE));
        const int my = static_cast<int>(std::floor(mouseWorld.y / TILE));
        const float dx = mouseWorld.x - (player.position.x + PLAYER_W / 2.f);
        const float dy = mouseWorld.y - (player.position.y + PLAYER_H / 2.f);
        const bool inReach = dx * dx + dy * dy <= REACH * REACH;
        const bool left = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        const bool right = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);

        if (inventoryOpen) {
            const auto winSize = window.getSize();
            constexpr float cell = 42.f, panelW = 9.f * cell + 28.f;
            constexpr float panelH = 412.f;
            const float px = (static_cast<float>(winSize.x) - panelW) * 0.5f;
            const float py = (static_cast<float>(winSize.y) - panelH) * 0.5f;
            const auto mouse = sf::Mouse::getPosition(window);
            const float rx = static_cast<float>(mouse.x) - px;
            const float ry = static_cast<float>(mouse.y) - py;
            auto hit = [&](float x, float y, float w, float h) {
                return rx >= x && rx < x + w && ry >= y && ry < y + h;
            };
            int clicked = -1;
            int craftClicked = -1;
            bool outputClicked = false;

            for (int row = 0; row < 3; ++row)
                for (int col = 0; col < 3; ++col)
                    if (hit(18.f + col * cell, 24.f + row * cell, 38.f, 38.f))
                        craftClicked = row * 3 + col;
            outputClicked = hit(panelW - 78.f, 57.f, 42.f, 42.f);
            for (int row = 0; row < 3; ++row)
                for (int col = 0; col < 9; ++col)
                    if (hit(14.f + col * cell, 195.f + row * cell, 38.f, 38.f))
                        clicked = HOTBAR_SIZE + row * 9 + col;
            for (int col = 0; col < 9; ++col)
                if (hit(14.f + col * cell, 350.f, 38.f, 38.f)) clicked = col;

            if (outputClicked && ((left && !leftHeld) || (right && !rightHeld))) {
                const ItemStack result = craftingResult(crafting);
                if (result.count > 0 &&
                    (cursorStack.count == 0 || cursorStack.block == result.block) &&
                    cursorStack.count <= maxStack(result.block) - result.count) {
                    if (cursorStack.count == 0) cursorStack.block = result.block;
                    cursorStack.count += result.count;

                    for (auto& ingredient : crafting) {
                        if (ingredient.count > 0) {
                            --ingredient.count;
                            if (ingredient.count == 0) ingredient = {};
                        }
                    }
                }
            }
            ItemStack* target = nullptr;
            if (clicked >= 0) target = &inventory[clicked];
            else if (craftClicked >= 0) target = &crafting[craftClicked];
            if (target && left && !leftHeld) {
                if (cursorStack.count > 0 && target->count > 0 && target->block == cursorStack.block) {
                    const int moved = std::min(cursorStack.count, maxStack(target->block) - target->count);
                    target->count += moved; cursorStack.count -= moved;
                    if (cursorStack.count == 0) cursorStack = {};
                } else std::swap(cursorStack, *target);
            }
            if (target && right && !rightHeld) {
                if (cursorStack.count > 0 && (target->count == 0 || target->block == cursorStack.block) && target->count < maxStack(cursorStack.block)) {
                    target->block = cursorStack.block; ++target->count;
                    --cursorStack.count; if (cursorStack.count == 0) cursorStack = {};
                } else if (cursorStack.count == 0 && target->count > 0) {
                    cursorStack.block = target->block;
                    cursorStack.count = (target->count + 1) / 2;
                    target->count -= cursorStack.count;
                    if (target->count == 0) *target = {};
                }
            }
        } else {

            const Block targetBlock = (inside(mx, my) && inReach) ? get(world, mx, my) : Air;
            if (!left || targetBlock == Air) {
                resetMining();
            } else {
                if (mx != miningX || my != miningY || targetBlock != miningBlock) {
                    miningX = mx; miningY = my;
                    miningBlock = targetBlock;
                    miningProgress = 0.f;
                }
                miningProgress += dt / miningDuration(miningBlock, selected);
                if (miningProgress >= 1.f) {

                    int slot = findStack(inventory, miningBlock);
                    if (slot < 0) slot = findEmptySlot(inventory);
                    if (slot >= 0) {
                        if (inventory[slot].count == 0) inventory[slot].block = miningBlock;
                        ++inventory[slot].count;
                        world[index(miningX, miningY)] = Air;
                    }
                    resetMining();
                }
            }
            if (inside(mx, my) && inReach && right && !rightHeld &&
                selected != Air && selected != Stick && !isTool(selected) &&
                inventory[selectedSlot].count > 0 && get(world, mx, my) == Air) {
                world[index(mx, my)] = selected;
                if (collides(world, playerBounds(player))) {
                    world[index(mx, my)] = Air;
                } else {
                    --inventory[selectedSlot].count;
                    if (inventory[selectedSlot].count == 0) inventory[selectedSlot].block = Air;
                }
            }
        }
        if (inventoryOpen) resetMining();
        leftHeld = left;
        rightHeld = right;

        window.clear(sf::Color(123, 194, 240));
        const sf::Vector2f topLeft = camera.getCenter() - camera.getSize() / 2.f;
        const int firstX = std::max(0, static_cast<int>(std::floor(topLeft.x / TILE)) - 1);
        const int lastX = std::min(WORLD_W - 1, static_cast<int>(std::ceil((topLeft.x + viewSize.x) / TILE)) + 1);
        const int firstY = std::max(0, static_cast<int>(std::floor(topLeft.y / TILE)) - 1);
        const int lastY = std::min(WORLD_H - 1, static_cast<int>(std::ceil((topLeft.y + viewSize.y) / TILE)) + 1);

        for (int y = firstY; y <= lastY; ++y)
            for (int x = firstX; x <= lastX; ++x) {
                const Block block = get(world, x, y);
                if (block == Wood || block == Leaves)
                    drawBlock(window, block, x * static_cast<float>(TILE), y * static_cast<float>(TILE), textures, loaded);
            }
        for (int y = firstY; y <= lastY; ++y)
            for (int x = firstX; x <= lastX; ++x) {
                const Block block = get(world, x, y);
                if (block != Air && block != Wood && block != Leaves)
                    drawBlock(window, block, x * static_cast<float>(TILE), y * static_cast<float>(TILE), textures, loaded);
            }

        if (!inventoryOpen && miningBlock != Air && miningProgress > 0.f &&
            inside(miningX, miningY) && get(world, miningX, miningY) == miningBlock)
            drawMiningCracks(window, miningX, miningY, miningProgress);

        int frame = 0;
        if (player.grounded && std::abs(player.velocity.x) > 0.1f)
            frame = 1 + (static_cast<int>(walkTimer / 0.13f) % 4);
        if (steveLoaded[frame]) {
            sf::Sprite steve(steveTextures[frame]);
            constexpr float spriteH = 62.f;
            const auto size = steveTextures[frame].getSize();
            steve.setOrigin({size.x / 2.f, static_cast<float>(size.y)});
            steve.setPosition({player.position.x + PLAYER_W / 2.f,
                               player.position.y + PLAYER_H});
            steve.setScale({(facingRight ? 1.f : -1.f) * spriteH / 550.f,
                            spriteH / 550.f});
            window.draw(steve);
        } else {
            sf::RectangleShape character({PLAYER_W, PLAYER_H});
            character.setPosition(player.position);
            character.setFillColor(sf::Color(244, 187, 65));
            window.draw(character);
        }
        if (!inventoryOpen && inside(mx, my) && inReach) {
            sf::RectangleShape highlight({static_cast<float>(TILE), static_cast<float>(TILE)});
            highlight.setPosition({mx * static_cast<float>(TILE), my * static_cast<float>(TILE)});
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineThickness(-2.f);
            highlight.setOutlineColor(sf::Color::White);
            window.draw(highlight);
        }

        const sf::Vector2u actualSize = window.getSize();
        if (actualSize.x == 0 || actualSize.y == 0) continue;
        if (uiView.getSize().x != static_cast<float>(actualSize.x) ||
            uiView.getSize().y != static_cast<float>(actualSize.y)) {
            uiView.setSize({static_cast<float>(actualSize.x), static_cast<float>(actualSize.y)});
            uiView.setCenter({actualSize.x * 0.5f, actualSize.y * 0.5f});
        }
        window.setView(uiView);
        constexpr float slotStep = 48.f;

        const sf::Vector2u windowSize = window.getSize();
        constexpr float barWidth = HOTBAR_SIZE * slotStep + 6.f;
        constexpr float barHeight = 52.f;
        constexpr float bottomMargin = 14.f;
        const float barX = (static_cast<float>(windowSize.x) - barWidth) / 2.f;
        const float barY = static_cast<float>(windowSize.y) - barHeight - bottomMargin;

        if (!inventoryOpen) {
            sf::RectangleShape bar({barWidth, barHeight});
        bar.setPosition({barX, barY});
        bar.setFillColor(sf::Color(188, 188, 188, 245));
        window.draw(bar);
        for (int i = 0; i < HOTBAR_SIZE; ++i) {
            const float sx = barX + 7.f + i * slotStep;
            const float sy = barY + 6.f;
            sf::RectangleShape slot({36.f, 36.f});
            slot.setPosition({sx, sy});
            slot.setFillColor(sf::Color(117, 117, 117));
            slot.setOutlineThickness(i == selectedSlot ? 3.f : 1.f);
            slot.setOutlineColor(i == selectedSlot ? sf::Color::White : sf::Color(65, 65, 65));
            window.draw(slot);
            const Block block = inventory[i].block;
            const int count = inventory[i].count;
            if (count == 0 || block == Air) continue;
            if (loaded[static_cast<std::size_t>(block)]) {
                sf::Sprite preview(textures[static_cast<std::size_t>(block)]);
                preview.setPosition({sx + 2.f, sy + 2.f});
                const auto sz = textures[static_cast<std::size_t>(block)].getSize();
                preview.setScale({32.f / sz.x, 32.f / sz.y});
                window.draw(preview);
            } else {
                sf::RectangleShape preview({32.f, 32.f});
                preview.setPosition({sx + 2.f, sy + 2.f});
                sf::Color color = blockColor(block);
                preview.setFillColor(color);
                window.draw(preview);
            }
            if (count > 0 && !isTool(block)) drawNumber(window, count, sx + 35.f, sy + 34.f);
        }
        }
        if (inventoryOpen) {
            constexpr float cell = 42.f, panelW = 9.f * cell + 28.f;
            constexpr float panelH = 412.f;
            const float px = (static_cast<float>(windowSize.x) - panelW) * 0.5f;
            const float py = (static_cast<float>(windowSize.y) - panelH) * 0.5f;
            sf::RectangleShape dim({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
            dim.setFillColor(sf::Color(0, 0, 0, 100)); window.draw(dim);
            sf::RectangleShape panel({panelW, panelH});
            panel.setPosition({px, py});
            panel.setFillColor(sf::Color(188, 188, 188, 245));
            panel.setOutlineThickness(4.f);
            panel.setOutlineColor(sf::Color(44, 44, 44)); window.draw(panel);
            const auto drawItem = [&](const ItemStack& item, float sx, float sy) {
                if (item.count <= 0 || item.block == Air) return;
                if (loaded[static_cast<std::size_t>(item.block)]) {
                    sf::Sprite sprite(textures[static_cast<std::size_t>(item.block)]);
                    sprite.setPosition({sx + 3.f, sy + 3.f});
                    const auto size = textures[static_cast<std::size_t>(item.block)].getSize();
                    sprite.setScale({32.f / size.x, 32.f / size.y}); window.draw(sprite);
                } else {
                    sf::RectangleShape icon({32.f, 32.f}); icon.setPosition({sx + 3.f, sy + 3.f});
                    icon.setFillColor(blockColor(item.block)); window.draw(icon);
                }
                if (!isTool(item.block)) drawNumber(window, item.count, sx + 36.f, sy + 36.f);
            };
            const auto drawSlot = [&](const ItemStack& item, float sx, float sy) {
                sf::RectangleShape border({38.f, 38.f});
                border.setPosition({sx, sy});
                border.setFillColor(sf::Color(117, 117, 117));
                border.setOutlineThickness(2.f);
                border.setOutlineColor(sf::Color(65, 65, 65)); window.draw(border);
                drawItem(item, sx, sy);
            };

            for (int row = 0; row < 3; ++row)
                for (int col = 0; col < 3; ++col)
                    drawSlot(crafting[row * 3 + col], px + 18.f + col * cell, py + 24.f + row * cell);
            sf::RectangleShape arrowBody({40.f, 9.f});
            arrowBody.setPosition({px + 181.f, py + 73.f});
            arrowBody.setFillColor(sf::Color(130, 130, 130)); window.draw(arrowBody);
            sf::ConvexShape arrowTip(3);
            arrowTip.setPoint(0, {px + 221.f, py + 62.f});
            arrowTip.setPoint(1, {px + 239.f, py + 77.f});
            arrowTip.setPoint(2, {px + 221.f, py + 92.f});
            arrowTip.setFillColor(sf::Color(130, 130, 130)); window.draw(arrowTip);
            drawSlot(craftingResult(crafting), px + panelW - 78.f, py + 57.f);

            for (int row = 0; row < 3; ++row)
                for (int col = 0; col < 9; ++col)
                    drawSlot(inventory[HOTBAR_SIZE + row * 9 + col], px + 14.f + col * cell, py + 195.f + row * cell);
            for (int col = 0; col < 9; ++col)
                drawSlot(inventory[col], px + 14.f + col * cell, py + 350.f);
            if (cursorStack.count > 0 && cursorStack.block != Air) {
                const auto mouse = sf::Mouse::getPosition(window);
                const float sx = static_cast<float>(mouse.x) - 16.f;
                const float sy = static_cast<float>(mouse.y) - 16.f;
                if (loaded[static_cast<std::size_t>(cursorStack.block)]) {
                    sf::Sprite sprite(textures[static_cast<std::size_t>(cursorStack.block)]);
                    sprite.setPosition({sx, sy});
                    const auto size = textures[static_cast<std::size_t>(cursorStack.block)].getSize();
                    sprite.setScale({32.f / size.x, 32.f / size.y}); window.draw(sprite);
                } else {
                    sf::RectangleShape icon({32.f, 32.f}); icon.setPosition({sx, sy});
                    icon.setFillColor(blockColor(cursorStack.block)); window.draw(icon);
                }
                if (!isTool(cursorStack.block)) drawNumber(window, cursorStack.count, sx + 34.f, sy + 32.f);
            }
        }

        int hoverArea = -1;
        int hoverSlot = -1;
        ItemStack hovered{};
        const auto mousePosition = sf::Mouse::getPosition(window);
        const float hx = static_cast<float>(mousePosition.x);
        const float hy = static_cast<float>(mousePosition.y);
        const auto over = [&](float x, float y, float w, float h) {
            return hx >= x && hx < x + w && hy >= y && hy < y + h;
        };
        if (window.hasFocus() && cursorStack.count == 0) {
            if (inventoryOpen) {
                constexpr float cell = 42.f;
                constexpr float panelW = 9.f * cell + 28.f;
                constexpr float panelH = 412.f;
                const float px = (static_cast<float>(windowSize.x) - panelW) * 0.5f;
                const float py = (static_cast<float>(windowSize.y) - panelH) * 0.5f;
                for (int row = 0; row < 3; ++row)
                    for (int col = 0; col < 3; ++col) {
                        const int i = row * 3 + col;
                        if (over(px + 18.f + col * cell, py + 24.f + row * cell, 38.f, 38.f)) {
                            hoverArea = 2; hoverSlot = i; hovered = crafting[i];
                        }
                    }
                if (over(px + panelW - 78.f, py + 57.f, 42.f, 42.f)) {
                    hoverArea = 3; hoverSlot = 0; hovered = craftingResult(crafting);
                }
                for (int row = 0; row < 3; ++row)
                    for (int col = 0; col < 9; ++col) {
                        const int i = HOTBAR_SIZE + row * 9 + col;
                        if (over(px + 14.f + col * cell, py + 195.f + row * cell, 38.f, 38.f)) {
                            hoverArea = 1; hoverSlot = i; hovered = inventory[i];
                        }
                    }
                for (int col = 0; col < HOTBAR_SIZE; ++col)
                    if (over(px + 14.f + col * cell, py + 350.f, 38.f, 38.f)) {
                        hoverArea = 1; hoverSlot = col; hovered = inventory[col];
                    }
            } else {
                for (int col = 0; col < HOTBAR_SIZE; ++col)
                    if (over(barX + 7.f + col * slotStep, barY + 6.f, 36.f, 36.f)) {
                        hoverArea = 0; hoverSlot = col; hovered = inventory[col];
                    }
            }
        }
        const Block hoverBlock = hovered.count > 0 ? hovered.block : Air;
        if (hoverArea != lastHoverArea || hoverSlot != lastHoverSlot ||
            hoverBlock != lastHoverBlock || hoverBlock == Air || left || right) {
            tooltipClock.restart();
        }
        lastHoverArea = hoverArea;
        lastHoverSlot = hoverSlot;
        lastHoverBlock = hoverBlock;
        if (tooltipFontLoaded && hoverBlock != Air &&
            tooltipClock.getElapsedTime().asSeconds() >= 3.f) {
            drawItemTooltip(window, tooltipFont, hoverBlock, {hx, hy});
        }

        window.setMouseCursorVisible(inventoryOpen);
        if (!inventoryOpen && window.hasFocus()) {
            const auto m = sf::Mouse::getPosition(window);
            if (m.x >= 0 && m.y >= 0 && m.x < static_cast<int>(windowSize.x) &&
                m.y < static_cast<int>(windowSize.y)) {
                if (isTool(selected) && loaded[static_cast<std::size_t>(selected)]) {
                    sf::Sprite toolCursor(textures[static_cast<std::size_t>(selected)]);
                    const auto sz = textures[static_cast<std::size_t>(selected)].getSize();
                    if (sz.x > 0 && sz.y > 0) {
                        constexpr float iconSize = 38.f;
                        toolCursor.setScale({iconSize / static_cast<float>(sz.x),
                                             iconSize / static_cast<float>(sz.y)});
                        toolCursor.setPosition({static_cast<float>(m.x) - iconSize * 0.5f,
                                                static_cast<float>(m.y) - iconSize * 0.5f});
                        window.draw(toolCursor);
                    }
                } else if (crosshairLoaded) {
                    sf::Sprite cursor(crosshairTexture);
                    cursor.setScale({3.f, 3.f});
                    cursor.setPosition({static_cast<float>(m.x) - 7.5f,
                                        static_cast<float>(m.y) - 7.5f});
                    window.draw(cursor);
                } else {
                    sf::RectangleShape h({13.f, 2.f});
                    h.setPosition({static_cast<float>(m.x) - 6.f, static_cast<float>(m.y) - 1.f});
                    h.setFillColor(sf::Color::White); window.draw(h);
                    sf::RectangleShape v({2.f, 13.f});
                    v.setPosition({static_cast<float>(m.x) - 1.f, static_cast<float>(m.y) - 6.f});
                    v.setFillColor(sf::Color::White); window.draw(v);
                }
            }
        }

        if (showAutoSaveIndicator) {
            const float elapsed = saveIndicatorClock.getElapsedTime().asSeconds();
            if (elapsed >= 2.4f) {
                showAutoSaveIndicator = false;
            } else if (std::fmod(elapsed, 0.6f) < 0.42f) {
                const float iconSize = 43.f;
                const float cx = static_cast<float>(windowSize.x) - 52.f;
                const float top = 15.f;
                if (saveIconLoaded) {
                    sf::Sprite icon(saveIconTexture);
                    const auto size = saveIconTexture.getSize();
                    if (size.x && size.y) {
                        icon.setScale({iconSize / static_cast<float>(size.x),
                                       iconSize / static_cast<float>(size.y)});
                        icon.setPosition({cx - iconSize * 0.5f, top});
                        window.draw(icon);
                    }
                }
                if (tooltipFontLoaded) {
                    sf::Text label(tooltipFont, "Autosaving...", 12);
                    label.setFillColor(sf::Color::White);
                    label.setOutlineColor(sf::Color(0, 0, 0, 210));
                    label.setOutlineThickness(1.f);
                    const float width = label.getLocalBounds().size.x;
                    label.setPosition({cx - width * 0.5f, top + iconSize + 4.f});
                    window.draw(label);
                }
            }
        }
        window.display();
    }
    saveCurrentWorld();
    return 0;
}
