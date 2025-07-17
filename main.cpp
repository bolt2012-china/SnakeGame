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
            nextState = ss.run();          
            break;
        }
        case AppState::LevelMode: {
    window.close();                      
    GameSFML game(COLS, ROWS, cell);    
    AppState result = game.run();

    if (result == AppState::EnergyTrackMode) {
        unlockedEnergyTrack = true;
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::EnergyTrackMode;
    }
    else if (result == AppState::LevelMode) {
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
        unlockedPortal = true;
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::PortalMode;
    }
    else if (result == AppState::EnergyTrackMode) {
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::EnergyTrackMode;
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


case AppState::PortalMode: {
    window.close();
    GameSFML game(COLS, ROWS, cell);
    AppState result = game.runPortalMode();

    if (result == AppState::ScoreMode) {
        unlockedPoints = true;
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::ScoreMode;
    }
    else if (result == AppState::PortalMode) {
        window.create(
            sf::VideoMode({ COLS * cell + SIDE, ROWS * cell }),
            "Snake");
        nextState = AppState::PortalMode;
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
