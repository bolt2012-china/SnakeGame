#include "StartScreen.h"
#include <stdexcept>
#include <variant>     // std::visit
#include <optional>    // std::optional

using namespace gui;

//  构  造
StartScreen::StartScreen(sf::RenderWindow& win)
: mWindow(win)
, mFont()
, mTitle(mFont, "", 72)            // 先用空串占位
{
    // 载入字体（SFML-3：openFromFile）
    // if (!mFont.openFromFile("C:/Windows/Fonts/arial.ttf"))
    //     throw std::runtime_error("Cannot open C:/Windows/Fonts/arial.ttf");
    if (!mFont.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
        throw std::runtime_error("Cannot open /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    // 配置标题
    mTitle.setString("S N A K E");
    mTitle.setFont(mFont);
    mTitle.setFillColor(sf::Color::Green);

    const float cx = mWindow.getSize().x * 0.5f;
    const auto  local = mTitle.getLocalBounds();
    mTitle.setOrigin({ local.size.x / 2.f, local.size.y });
    mTitle.setPosition({ cx, 80.f });

    //三个按钮
    mButtons.emplace_back(mFont, "Level Mode",   220.f, cx);
    mButtons.emplace_back(mFont, "Portal Mode", 320.f, cx);
    mButtons.emplace_back(mFont, "Points Mode",   420.f, cx);
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
            }
            else if (const auto* move = evt->getIf<sf::Event::MouseMoved>()) {
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


        mWindow.clear(sf::Color::Black);
        mWindow.draw(mTitle);
        for (const auto& b : mButtons) mWindow.draw(b.label);
        mWindow.display();
    }

    return mWindow.isOpen() ? nextState : AppState::Exit;
}
