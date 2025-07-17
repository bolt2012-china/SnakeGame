#include "Overlay.h"
#include <stdexcept>

Overlay::Overlay(const sf::Vector2u& winSize,
                 const std::string&  text,
                 const std::string&  fontPath,
                 unsigned int        charSize,
                 const std::string&  audioPath,
                 sf::Color           maskColor)
: mMask   {sf::Vector2f(static_cast<float>(winSize.x),
                        static_cast<float>(winSize.y))},
  mFont   {},                                  
  mText   {mFont, text, charSize},             
  mBuf    {}
{

    if (!mFont.openFromFile(fontPath))
        throw std::runtime_error("Cannot open font: " + fontPath);

    mText.setFont(mFont);                      
    mText.setFillColor(sf::Color::White);

    const auto bounds = mText.getLocalBounds(); 
    mText.setOrigin({bounds.size.x * 0.5f,
                     bounds.size.y * 0.5f});
    mText.setPosition({winSize.x * 0.5f,
                       winSize.y * 0.5f});

    mMask.setFillColor(maskColor);

    mBuf.loadFromFile(audioPath);       
}

void Overlay::play(sf::RenderWindow& window,
                   float             seconds,
                   const std::function<void()>& drawBelow)
{
    if (!window.isOpen()) return;

    drawBelow();                 
    window.draw(mMask);
    window.draw(mText);
    window.display();

    sf::Sound sound(mBuf);     
    sound.play();

    
    sf::Clock timer;
    while (timer.getElapsedTime().asSeconds() < seconds && window.isOpen())
    {
        while (const auto ev = window.pollEvent())
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                return;
            }

        sf::sleep(sf::milliseconds(10));  
    }
}
