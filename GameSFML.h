#ifndef GAME_SFML_H
#define GAME_SFML_H

#include <SFML/Graphics.hpp>
#include <array>
#include "snake.h"
#include "GameApp.h"   


enum class GameState { Playing, Paused, GameOver };

class GameSFML {
public:
    GameSFML(unsigned int columns, unsigned int rows, unsigned int cellSize);
    AppState run();
    bool hitBoundary();
    bool hitObstacles();
    bool isSnakeInCenterArea(); // Helper function to check if snake is in center area
    void runPortalMode();  // TODO
    void runScoreMode();  // 分数模式

private:
    void drawBackground(); //绘制游戏窗口背景图
    void processEvents();
    void update();
    void updatePortalMode();
    void updateScoreMode();
    void render();
    void renderPortalMode();
    void renderScoreMode();
    void renderBoard();
    void renderNewBoard();
    void renderSnake();
    void renderFood();
    void renderPSFood();
    void renderScoreFood();  // 渲染分数模式的多彩食物
    void renderScoreTunnels(); // 渲染分数隧道
    void renderObstacles();
    void renderUI();
    // Gameover页面
    void openGameOverDialog();
    void processDialogEvents();
    void restartGame();

    //分数记录相关
    void updateHighScores(int score);   // 把本局成绩写入数组并落盘
    void loadHighScores();              // 启动时读取历史
    void saveHighScores();              // 数组变更时写文件

    void generateFood();                // 生成食物
    void generatePortalFood(); // 生成RB食物
    void generateScoreFood();  // 生成大量不同颜色的食物
    void generateScoreTunnels(); // 生成分数隧道

    void generateObstacles(); // 生成障碍物
    void generateScoreObstacles(); // 生成障碍物
   

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
    SnakeBody mFood;
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
    sf::RectangleShape mHomeDBtn;
    sf::Text        mRestartTxt;
    sf::Text        mQuitTxt;
    sf::Text        mHomeDTxt;

    // HP
    sf::Text mHitPointsText;
    float mHitEffectTimer = 0.0f;
    float mInvincibleTimer = 0.0f;

    bool visible = true;

    SnakeBody mRegularFood;  // 普通食物（红色）
    SnakeBody mPortalFood;   // 传送食物（蓝色）

    // Score Mode 多彩食物
    struct ColoredFood {
        SnakeBody position;
        sf::Color color;
        int value;  // 不同颜色食物的分值
    };
    std::vector<ColoredFood> mScoreFoods; // 分数模式的多彩食物数组

    // Score Mode 隧道
    struct ScoreTunnel {
        SnakeBody entrance;  // 入口位置
        SnakeBody exit;      // 出口位置
        sf::Color color;     // 隧道颜色
        int bonusPoints;     // 通过隧道获得的奖励分数
        bool isActive;       // 隧道是否激活
    };
    std::vector<ScoreTunnel> mScoreTunnels; // 分数模式的隧道数组

    std::vector<SnakeBody> mObstacles; // 障碍物数组
    int mObstacleCount = 5; // 初始障碍物数量
    bool mNewBoardActivated = false; // Flag to track if new board has been rendered

    sf::Texture mBgTexture;   // 背景图
    sf::Sprite  mBgSprite;    // 背景图

    sf::Texture mHeadTexture;  // 蛇头
    sf::Sprite  mHeadSprite;   // 蛇头

    sf::RectangleShape mPauseBtn;   // 侧边栏暂停按钮
    sf::Text mPauseTxt;
    bool               mPauseHover = false;
    
    //暂停菜单按钮
    sf::RectangleShape mHomeBtn;
    sf::Text           mHomeTxt;

    sf::RectangleShape mPauseQuitBtn;
    sf::Text           mPauseQuitTxt;

    bool mOverlayHoverHome   = false;
    bool mOverlayHoverQuit   = false;   

    AppState mOutcome = AppState::Exit;

};

#endif
