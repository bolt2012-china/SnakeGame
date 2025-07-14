#include "StartScreen.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <stdexcept>
#include <variant>     // std::visit
#include <optional>    // std::optional
#include <filesystem> 

using namespace gui;

//  构  造
StartScreen::StartScreen(sf::RenderWindow& win)
: mWindow(win)
, mBgTexture{}                 // 先默认构造纹理
, mBgSprite(mBgTexture)        // ★ Sprite 必须马上绑定纹理
, mFont()
, mTitle(mFont, "", 72)            // 先用空串占位
{
    // 载入字体（SFML-3：openFromFile）
    if (!mFont.openFromFile("assets/fonts/Baloo2.ttf"))
        throw std::runtime_error("Cannot open C:/Windows/Fonts/arial.ttf");

    if (!mBgTexture.loadFromFile("assets/startpage.jpg"))
        throw std::runtime_error("Cannot open assets/startpage.jpg");

    mBgSprite.setTexture(mBgTexture, /*resetRect=*/true); 
    //等比缩放到窗口大小
    sf::Vector2u winSize = mWindow.getSize();
    sf::Vector2u texSize = mBgTexture.getSize();
    mBgSprite.setScale(sf::Vector2f{
        float(winSize.x) / texSize.x,
        float(winSize.y) / texSize.y});

    const float cx = mWindow.getSize().x * 0.85f; 


    //三个按钮
    mButtons.emplace_back(mFont, "Level Mode",   320.f, cx);
    mButtons.emplace_back(mFont, "Portal Mode", 520.f, cx);
    mButtons.emplace_back(mFont, "Points Mode",   720.f, cx);
}


// 运行：渲染并等待用户选择
AppState StartScreen::run()
{
    bool      running    = true;
    AppState  nextState  = AppState::StartMenu;   // 默认回菜单 / 退出由窗口控制

    while (mWindow.isOpen() && running)
    {
        // 事件处理
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
                    if (mButtons[0].isMouseOver(mp)) return AppState::LevelMode;
                    if (mButtons[1].isMouseOver(mp)) return AppState::PortalMode;
                    if (mButtons[2].isMouseOver(mp)) return AppState::ScoreMode;
                }
            }
        }
        mWindow.clear();                 // 颜色随意，反正被背景盖住
        mWindow.draw(mBgSprite);         // ★ 先画背景
        mWindow.draw(mTitle);            // ★ 再画前景
        for (const auto& b : mButtons) mWindow.draw(b.label);
        mWindow.display();
    }

    return mWindow.isOpen() ? nextState : AppState::Exit;
}
