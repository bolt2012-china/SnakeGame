#include <SFML/Graphics.hpp>
#include <algorithm> 
#include "GameApp.h"
#include "StartScreen.h"
#include "GameSFML.h"

using gui::StartScreen;


int main()
{
    // 计算窗口尺寸、网格参数 
    constexpr unsigned COLS = 40, ROWS = 30, SIDE = 240;
    auto desk = sf::VideoMode::getDesktopMode();
    unsigned deskW = desk.size.x; 
    unsigned deskH = desk.size.y;

    unsigned cell = std::max(10u,
        std::min( (deskW * 3 / 4 - SIDE) / COLS,
                (deskH * 3 / 4)        / ROWS ));
    sf::RenderWindow window(
        sf::VideoMode({ COLS * cell + SIDE,  ROWS * cell }),  
        "Snake");


    AppState state = AppState::StartMenu;

    while (state != AppState::Exit && window.isOpen())
    {
        switch (state)
        {
        case AppState::StartMenu: {
            StartScreen ss(window);
            state = ss.run();                 // 返回下一状态
            break;
        }
        case AppState::LevelMode: {
            window.close();                      // 关掉菜单窗
            GameSFML game(COLS, ROWS, cell);     // 旧构造自行开窗
            AppState result = game.run();
            if (result == AppState::StartMenu) {
                // 重新开一个窗口作欢迎页
                window.create(
                    sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
                    "Snake");
                state = AppState::StartMenu;
            } else {
                state = AppState::Exit; 
            }
            break;
        }
        case AppState::PortalMode: {
            window.close();                      
            GameSFML game(COLS, ROWS, cell);   
            AppState result = game.runPortalMode();  
            if (result == AppState::StartMenu) {
                 window.create(
                    sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
                    "Snake");
                state = AppState::StartMenu;
            } else {
                state = AppState::Exit;
            }
            break;                         
        }
        case AppState::ScoreMode: {
           window.close();                      
            GameSFML game(COLS, ROWS, cell);   
            AppState result = game.runScoreMode();  
            if (result == AppState::StartMenu) {
                 window.create(
                    sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
                    "Snake");
                state = AppState::StartMenu;
            } else {
                state = AppState::Exit;
            }
            break;                                 
        }
        default:
            state = AppState::Exit;
        }
    }
    return 0;
}