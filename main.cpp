#include <SFML/Graphics.hpp>
#include <algorithm> 
#include "GameApp.h"
#include "StartScreen.h"
#include "GameSFML.h"

using gui::StartScreen;


bool unlockedLevel        = true;
bool unlockedEnergyTrack  = false;
bool unlockedPortal       = false;
bool unlockedPoints       = false;



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
    AppState  nextState = AppState::StartMenu; 

    while (state != AppState::Exit && window.isOpen())
    {
        switch (state)
        {
        case AppState::StartMenu: {
            StartScreen ss(window);
            nextState = ss.run();               // 返回下一状态
            break;
        }
        case AppState::LevelMode: {
    window.close();                      
    GameSFML game(COLS, ROWS, cell);    
    AppState result = game.run();

    if (result == AppState::EnergyTrackMode) {
        // 1) 解锁下一关
        unlockedEnergyTrack = true;
        // 2) 先重建窗口
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        // 3) 切换到下一关
        nextState = AppState::EnergyTrackMode;
    }
    else if (result == AppState::LevelMode) {
        // Resume 本关，同样要重建窗口
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::LevelMode;
    }
    else if (result == AppState::StartMenu) {
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::StartMenu;
    }
    else {
        nextState = AppState::Exit;
    }
    break;
}

case AppState::EnergyTrackMode: {
    window.close();
    GameSFML game(COLS, ROWS, cell);
    AppState result = game.runEnergyTrackMode();

    if (result == AppState::PortalMode) {
        // Next：解锁下一关并进入 PortalMode
        unlockedPortal = true;
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::PortalMode;
    }
    else if (result == AppState::EnergyTrackMode) {
        // Resume：重新在 EnergyTrackMode 中继续
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::EnergyTrackMode;
    }
    else if (result == AppState::StartMenu) {
        // Home
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::StartMenu;
    }
    else {
        // Quit
        nextState = AppState::Exit;
    }
    break;
}


case AppState::PortalMode: {
    window.close();
    GameSFML game(COLS, ROWS, cell);
    AppState result = game.runPortalMode();

    if (result == AppState::ScoreMode) {
        // Next：解锁下一关并进入 ScoreMode
        unlockedPoints = true;
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::ScoreMode;
    }
    else if (result == AppState::PortalMode) {
        // Resume：重新在 PortalMode 中继续
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::PortalMode;
    }
    else if (result == AppState::StartMenu) {
        // Home
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::StartMenu;
    }
    else {
        // Quit
        nextState = AppState::Exit;
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
                nextState = AppState::StartMenu;
            } else {
                nextState = AppState::Exit;
            }
            break;                                 
        }

        default:
            nextState = AppState::Exit;
        }
        state = nextState;
    }
    return 0;
}