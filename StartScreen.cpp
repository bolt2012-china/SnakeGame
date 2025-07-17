#include "StartScreen.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <stdexcept>
#include <variant>    
#include <optional>  
#include <filesystem> 

using namespace gui;

StartScreen::StartScreen(sf::RenderWindow& win)
: mWindow(win)
, mBgTexture{}               
, mBgSprite(mBgTexture)       
, mFont()
, mTitle(mFont, "", 72)      
, mMusic()
{
    if (!mFont.openFromFile("assets/fonts/Baloo2.ttf"))
        throw std::runtime_error("Cannot open C:/Windows/Fonts/arial.ttf");

    if (!mBgTexture.loadFromFile("assets/startpage.jpg"))
        throw std::runtime_error("Cannot open assets/startpage.jpg");

    if (!mMusic.openFromFile("assets/music/wel.ogg"))
        throw std::runtime_error("Cannot open assets/music/wel.ogg");
    
    mMusic.setLooping(true);       
    mBgSprite.setTexture(mBgTexture, /*resetRect=*/true); 
    sf::Vector2u winSize = mWindow.getSize();
    sf::Vector2u texSize = mBgTexture.getSize();
    mBgSprite.setScale(sf::Vector2f{
        float(winSize.x) / texSize.x,
        float(winSize.y) / texSize.y});

    const float cx = mWindow.getSize().x * 0.85f; 

    mButtons.emplace_back(mFont, "Level Mode", 320.f, cx);
    mButtons.emplace_back(mFont, "Energy Track", 460.f, cx);
    mButtons.emplace_back(mFont, "Portal Mode", 600.f, cx);
    mButtons.emplace_back(mFont, "Points Mode", 740.f, cx);
    
    mButtons[0].setEnabled(unlockedLevel);
    mButtons[1].setEnabled(unlockedEnergyTrack);
    mButtons[2].setEnabled(unlockedPortal);
    mButtons[3].setEnabled(unlockedPoints);

}

AppState StartScreen::run()
{
    mMusic.play();      
    bool      running    = true;
    AppState  nextState  = AppState::StartMenu;
    while (mWindow.isOpen() && running)
    {
        while (auto evt = mWindow.pollEvent()) {
            if (evt->is<sf::Event::Closed>()) {
                mWindow.close();
                return AppState::Exit;
            }else if (const auto* move = evt->getIf<sf::Event::MouseMoved>()) {
                sf::Vector2f mp{ static_cast<float>(move->position.x),
                            static_cast<float>(move->position.y) };
                for (auto& b : mButtons) b.setHover(b.isMouseOver(mp));
            }
            else if (const auto* click = evt->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mp{ static_cast<float>(click->position.x),
                             static_cast<float>(click->position.y) };
                    if (mButtons[0].isMouseOver(mp)  && mButtons[0].enabled) return AppState::LevelMode;
                    if (mButtons[1].isMouseOver(mp) && mButtons[1].enabled) return AppState::EnergyTrackMode;
                    if (mButtons[2].isMouseOver(mp) && mButtons[2].enabled) return AppState::PortalMode;
                    if (mButtons[3].isMouseOver(mp)  && mButtons[3].enabled) return AppState::ScoreMode;

                }
            }
        }
        mWindow.clear();               
        mWindow.draw(mBgSprite);       
        mWindow.draw(mTitle);         
        for (const auto& b : mButtons) mWindow.draw(b.label);
        mWindow.display();
    }

    mMusic.stop();
    return mWindow.isOpen() ? nextState : AppState::Exit;
}
