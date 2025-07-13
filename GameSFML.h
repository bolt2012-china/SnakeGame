#ifndef GAME_SFML_H
#define GAME_SFML_H

#include <SFML/Graphics.hpp>
#include <array>
#include "snake.h"

enum class GameState { Playing, GameOver };

class GameSFML {
public:
    GameSFML(unsigned int columns, unsigned int rows, unsigned int cellSize);
    void run();
    bool hitBoundary();
    void runPortalMode();  // TODO
    void runScoreMode () {}  // TODO

private:
    void processEvents();
    void update();
    void updatePortalMode();
    void render();
    void renderPortalMode();
    void renderBoard();
    void renderNewBoard();
    void renderSnake();
    void renderFood();
    void renderUI();
    // Gameover页面
    void openGameOverDialog();
    void processDialogEvents();
    void restartGame();

    //分数记录相关
    void updateHighScores(int score);   // 把本局成绩写入数组并落盘
    void loadHighScores();              // 启动时读取历史
    void saveHighScores();              // 数组变更时写文件

    void generatePortalFood(); // 生成食物
   

    GameState mState = GameState::Playing;
    
    sf::RectangleShape mSidebarBorder;//侧边栏的框
    sf::RectangleShape mGameBorder; //游戏区域的边框
    enum class mBorderType { Circle, Triangle };
    sf::CircleShape mCircleBorder;
    sf::ConvexShape mTriangleBorder;
    mBorderType currentType;


    sf::RenderWindow mWindow;
    unsigned int mColumns;
    unsigned int mRows;
    unsigned int mCellSize;
    Snake mSnake;
    // SnakeBody mFood;
    int mPoints = 0;
    int mDifficulty = 0;
    float mDelay = 0.1f;

    sf::Font mFont;
    sf::Text mPointsText;
    sf::Text mDifficultyText;
    sf::Text mInstructionText;
    
    sf::Text mUpText;
    sf::Text mDownText;
    sf::Text mLeftText;
    sf::Text mRightText;

    sf::Text mLeaderBoardTitleText;
    sf::Text mLeader1Text;
    sf::Text mLeader2Text;
    sf::Text mLeader3Text;

    std::array<int,3> mHighScores;

    // —— Game-Over 对话框相关 ——
    sf::RenderWindow mDialog;           // 默认构造即可
    sf::RectangleShape mRestartBtn;
    sf::RectangleShape mQuitBtn;
    sf::Text        mRestartTxt;
    sf::Text        mQuitTxt;

    SnakeBody mRegularFood;  // 普通食物（红色）
    SnakeBody mPortalFood;   // 传送食物（蓝色）
};

#endif
