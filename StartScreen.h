#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "GameApp.h"

namespace gui {

//Button
struct TextButton {
    sf::Text      label;
    sf::FloatRect bounds;

    TextButton(const sf::Font& font,
               const std::string& text,
               float yPos,
               float xCenter)
        : label(font, text, 48)                 // (font, string, size)
    {
        const auto local = label.getLocalBounds();
        label.setFillColor(sf::Color::White);
        label.setOrigin({local.size.x / 2.f, local.size.y});
        label.setPosition({xCenter, yPos});
        bounds = label.getGlobalBounds();
    }

    bool isMouseOver(sf::Vector2f p) const { return bounds.contains(p); }
    void setHover(bool h) {
        label.setFillColor(h ? sf::Color(255,200,50) : sf::Color::White);
    }
};

class StartScreen {
public:
    explicit StartScreen(sf::RenderWindow& win);
    AppState run();

private:
    sf::RenderWindow&       mWindow;
    sf::Font                mFont;
    sf::Text                mTitle;          // 用带-font 构造
    std::vector<TextButton> mButtons;
};

} 
