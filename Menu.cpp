#include "Menu.hpp"
#include <algorithm>
#include <string>
namespace sandbox {
namespace {
struct Rect{float x,y,w,h;};
Rect button(int index,sf::Vector2u size,MenuScreen screen){
    const float w=std::min(470.f,size.x*0.8f),h=44.f,gap=9.f;
    const float top=screen==MenuScreen::Main||screen==MenuScreen::Naming?size.y*0.55f:size.y*0.32f;
    return {(size.x-w)*0.5f,top+index*(h+gap),w,h};
}
void text(sf::RenderWindow& window,const sf::Font& font,const sf::String& value,unsigned sz,sf::Color color,float x,float y){
    sf::Text t(font,value,sz);t.setFillColor(color);auto b=t.getLocalBounds();t.setOrigin({b.position.x+b.size.x*0.5f,b.position.y+b.size.y*0.5f});t.setPosition({x,y});window.draw(t);
}
void drawButton(sf::RenderWindow& window,const sf::Font& font,const sf::String& label,Rect r,bool hover){
    sf::RectangleShape shadow({r.w,r.h});shadow.setPosition({r.x+3,r.y+3});shadow.setFillColor(sf::Color(0,0,0,170));window.draw(shadow);
    sf::RectangleShape b({r.w,r.h});b.setPosition({r.x,r.y});b.setFillColor(hover?sf::Color(155,155,155):sf::Color(100,100,100));b.setOutlineThickness(2);b.setOutlineColor(sf::Color(25,25,25));window.draw(b);
    text(window,font,label,20,sf::Color::White,r.x+r.w/2,r.y+r.h/2);
}
}
int menuHit(MenuScreen screen,sf::Vector2f mouse,sf::Vector2u size,std::size_t worldCount,int scroll){
    if(screen==MenuScreen::Playing)return -1;
    int count=screen==MenuScreen::Main?2:screen==MenuScreen::Naming?2:static_cast<int>(std::min<std::size_t>(worldCount-static_cast<std::size_t>(std::min<int>(scroll,static_cast<int>(worldCount))),5))+2;
    for(int i=0;i<count;++i){auto r=button(i,size,screen);if(mouse.x>=r.x&&mouse.x<r.x+r.w&&mouse.y>=r.y&&mouse.y<r.y+r.h)return i;}
    return -1;
}
void drawMenu(sf::RenderWindow& window,MenuScreen screen,const sf::Texture* background,
              const sf::Texture* logo,const sf::Font* font,const std::vector<SavedWorldInfo>& worlds,
              int scroll,const sf::String& worldName,const std::string& status){
    const auto size=window.getSize();window.setView(sf::View(sf::FloatRect({0.f,0.f},{float(size.x),float(size.y)})));
    window.clear(sf::Color(49,42,34));
    if(background){sf::Sprite sprite(*background);auto ts=background->getSize();sprite.setScale({float(size.x)/ts.x,float(size.y)/ts.y});window.draw(sprite);}
    sf::RectangleShape dim({float(size.x),float(size.y)});dim.setFillColor(sf::Color(0,0,0,90));window.draw(dim);
    if(logo){sf::Sprite sprite(*logo);auto ts=logo->getSize();float scale=std::min({size.x*.78f/ts.x,size.y*.28f/ts.y,1.6f});sprite.setScale({scale,scale});sprite.setPosition({(size.x-ts.x*scale)*.5f,size.y*.06f});window.draw(sprite);}
    if(!font){window.display();return;}
    const auto mouse=sf::Mouse::getPosition(window);
    int hover=menuHit(screen,{float(mouse.x),float(mouse.y)},size,worlds.size(),scroll);
    if(screen==MenuScreen::Main){
        drawButton(window,*font,"Singleplayer",button(0,size,screen),hover==0);
        drawButton(window,*font,"Quit Game",button(1,size,screen),hover==1);
    }else if(screen==MenuScreen::Worlds){
        text(window,*font,"Select World",28,sf::Color::White,size.x*.5f,size.y*.27f);
        drawButton(window,*font,"Create New World",button(0,size,screen),hover==0);
        int visible=std::min<int>(5,static_cast<int>(worlds.size())-scroll);
        for(int i=0;i<visible;++i){
            const auto& name=worlds[scroll+i].name;
            sf::String label=sf::String::fromUtf8(name.begin(),name.end());
            if(label.getSize()>26)label=label.substring(0,26)+"...";
            drawButton(window,*font,label,button(i+1,size,screen),hover==i+1);
        }
        drawButton(window,*font,"Back",button(visible+1,size,screen),hover==visible+1);
        if(worlds.empty())text(window,*font,"No saved worlds yet",16,sf::Color(210,210,210),size.x*.5f,size.y*.73f);
        else text(window,*font,"Mouse wheel: scroll worlds",15,sf::Color(210,210,210),size.x*.5f,size.y*.80f);
    }else if(screen==MenuScreen::Naming){
        text(window,*font,"Create New World",28,sf::Color::White,size.x*.5f,size.y*.25f);
        text(window,*font,"World Name:",19,sf::Color::White,size.x*.5f,size.y*.31f);
        sf::RectangleShape input({std::min(470.f,size.x*.8f),48.f});input.setPosition({(size.x-input.getSize().x)*.5f,size.y*.35f});input.setFillColor(sf::Color(20,20,20,220));input.setOutlineThickness(2);input.setOutlineColor(sf::Color::White);window.draw(input);
        auto display=worldName;display+="_";
        text(window,*font,display,21,sf::Color::White,size.x*.5f,size.y*.35f+24.f);
        drawButton(window,*font,"Create World",button(0,size,screen),hover==0);
        drawButton(window,*font,"Back",button(1,size,screen),hover==1);
    }
    if(!status.empty())text(window,*font,sf::String::fromUtf8(status.begin(),status.end()),16,sf::Color(255,170,130),size.x*.5f,size.y*.89f);
    text(window,*font,"Sandbox2D v0.7.1",15,sf::Color(220,220,220),size.x*.5f,size.y-22.f);
    window.display();
}
}
