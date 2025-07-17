#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include "GameApp.h"


extern bool unlockedLevel;
extern bool unlockedEnergyTrack;
extern bool unlockedPortal;
extern bool unlockedPoints;

namespace gui {

//Button
struct TextButton {
    sf::Text      label;
    sf::FloatRect bounds;
    bool enabled = true;

    TextButton(const sf::Font& font,
               const std::string& text,
               float yPos,
               float xCenter)
        : label(font, text, 48)             
    {
        const auto local = label.getLocalBounds();
        label.setFillColor(sf::Color::White);
        label.setOrigin({local.size.x / 2.f, local.size.y});
        label.setPosition({xCenter, yPos});
        bounds = label.getGlobalBounds();
        setEnabled(true); 
    }

    bool isMouseOver(sf::Vector2f p) const { return bounds.contains(p); }
    void setHover(bool h) {
        label.setFillColor(h ? sf::Color(255,200,50) : sf::Color::White);
    }
     
    void setEnabled(bool e) {
        enabled = e;
        label.setFillColor(enabled
            ? sf::Color::White
            : sf::Color(100,100,100)); 
    }
};

class StartScreen {
public:
    explicit StartScreen(sf::RenderWindow& win);
    AppState run();

private:
    sf::RenderWindow&       mWindow;
    sf::Font                mFont;
    sf::Text                mTitle;         
    std::vector<TextButton> mButtons;
    sf::Texture             mBgTexture; 
    sf::Sprite              mBgSprite;   

    sf::Music               mMusic;
};

} 
