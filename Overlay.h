#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>         
#include <functional>
#include <string>

class Overlay {
public:
    Overlay(const sf::Vector2u& winSize,
            const std::string&  text,
            const std::string&  fontPath,
            unsigned int charSize,
            const std::string&  audioPath,
            sf::Color mask = {120, 120, 120, 180});

    void play(sf::RenderWindow& window,
              float  seconds,
              const std::function<void()>& drawBelow);

private:
    sf::RectangleShape mMask;
    sf::Font mFont;
    sf::Text mText;  
    sf::Sound BuffermBuf;     
};
