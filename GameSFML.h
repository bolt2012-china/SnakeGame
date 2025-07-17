#ifndef GAME_SFML_H
#define GAME_SFML_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include "snake.h"
#include "GameApp.h"  
#include "Overlay.h"          // ★ 新增
#include <algorithm>
#include <vector>


enum class GameState { Playing, Paused, GameOver, Victory };

class GameSFML {
public:
    GameSFML(unsigned int columns, unsigned int rows, unsigned int cellSize);
    AppState run();
    bool hitBoundary();
    bool hitObstacles();
    bool isSnakeInCenterArea(); // 中心判定（用于激活新图）
    AppState runPortalMode();  // TODO
    AppState runScoreMode();  // 分数模式
    AppState runEnergyTrackMode();
    void openVictoryDialog(bool* resumeFlag = nullptr);

    

private:
    
    void drawBackground(); //绘制游戏窗口背景图
    void processEvents();
    void update();
    void updatePortalMode();
    void updateScoreMode();
    void render();
    void renderFrame();
    void renderPortalMode();
    void renderScoreMode();
    void renderBoard();
    void renderNewBoard();
    void renderSnake();
    void renderFood();
    void renderScoreFood();  // 渲染分数模式的多彩食物
    void renderScoreTunnels(); // 渲染分数隧道
    void renderObstacles();
    void renderUI();
    // Gameover页面
    void openGameOverDialog();
    void processDialogEvents();
    void restartGame();

    void generateFood();                // 生成食物
    void generatePairedFood(); //生成传送食物
    void renderPairedFood();   

    void generateScoreFood();  // 生成大量不同颜色的食物
    void generateScoreTunnels(); // 生成分数隧道

    void generateObstacles(); // 生成障碍物
    void generateScoreObstacles(); // 生成障碍物

    bool scoreFoodAt(int x, int y) const; //判断格子是否已被彩色食物占用
   
    

    GameState mState = GameState::Playing;
    
    sf::RectangleShape mSidebarBorder;//侧边栏的框
    sf::RectangleShape mGameBorder; //游戏区域的边框
    enum class mBorderType { Circle, Diamond };
    sf::CircleShape mCircleBorder;
    sf::ConvexShape mDiamondBorder;
    mBorderType currentType;
    static bool pointInPolygon(float px,float py,
                                const std::vector<sf::Vector2f>& poly);
    

    sf::RenderWindow mWindow;
    unsigned int mColumns;
    unsigned int mRows;
    unsigned int mCellSize;
    Snake mSnake;
    SnakeBody mFood;
    int mPoints = 0;
    int mDifficulty = 0;
    float mDelay = 0.2f;
    static constexpr float MIN_DELAY = 0.04f;   // 全局速度下限

    sf::Font mFont;
    sf::Font mBaloo2Bold;
    sf::Font mRussoOne;

    sf::Text mInstructionText;

    sf::Text mPointsLabel; 
    sf::Text mPointsValue;
    sf::Text mLivesText;
    sf::Text mDifficultyText; 
    
    sf::Text mUpText;
    sf::Text mDownText;
    sf::Text mLeftText;
    sf::Text mRightText;

    

    // —— 四种模式的最高分记录 —— 
    enum ModeIndex { LevelModeIdx=0,EnergyTrackModeIdx=1, PortalModeIdx=2, PointsModeIdx=3, };
    static constexpr int kNumModes = 4;
    std::array<int, kNumModes>        mModeHighScores;
    sf::Text                          mHighScoresTitleText;
    std::vector<sf::Text>             mModeHighScoreTexts;
    static const std::array<std::string, kNumModes> modeNames;

    // 分数记录相关
    void updateHighScores(int score, AppState mode);  // 按模式更新最高分
    void loadHighScores();                             // 启动时读取历史
    void saveHighScores();                             // 数组变更时写文件

    // —— Game-Over 对话框相关 ——
    sf::RenderWindow mDialog;           // 默认构造即可
    sf::RectangleShape mRestartBtn;
    sf::RectangleShape mQuitBtn;
    sf::RectangleShape mHomeDBtn;
    sf::Text        mRestartTxt;
    sf::Text        mQuitTxt;
    sf::Text        mHomeDTxt;
    sf::Text        mGameOverMsg;
    sf::Font        mGameOverFont;  

    // HP
    // 旧的 mHitPointsText 已被 mLivesText 取代
    float mHitEffectTimer = 0.0f;
    float mInvincibleTimer = 0.0f;

    bool visible = true;

    //传送食物
    std::array<SnakeBody,2>  mPortals; 

    // Score Mode 多彩食物
    struct ColoredFood {
        SnakeBody position;
        sf::Color color;
        int value;  // 不同颜色食物的分值
        int typeIndex; 
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

    

    struct FoodQuota { sf::Color color; int value; int quota; };
    static const std::array<FoodQuota,4> kFoodQuotas;

    /* —— Life-Food ——————————————————————— */
    SnakeBody mLifeFood;           // 心形位置
    bool      mHasLifeFood   = false;
    float     mLifeElapsed   = 0.f; // 已存在时间（秒）

    void generateLifeFood();       // 随机刷一颗
    void updateLifeFood(float dt); // 计时 & 过期 / 刷新
    void renderLifeFood();         // 绘制心形

    // ─── SFX buffers ───────────────────────────────────────────
    sf::SoundBuffer mBufTurn, mBufEat, mBufEatSpec,
                mBufLife, mBufPort, mBufTunnel,
                mBufLoseHP, mBufDeath;

    // 动态保存**正在播放**的声音对象
    std::vector<sf::Sound> mSounds;

    // Helper：播放并自动管理声道
    void playSfx(const sf::SoundBuffer& buf);

    /* ─── 能量轨道 ───────────────────────────── */
    struct EnergyTrack {
        std::vector<SnakeBody> cells;   // 顺序路径
        std::size_t progress = 0;       // 已经通过的格数
        bool reverse = false;      // 记录是正向（0→end）还是反向（end→0）
    };
    static constexpr int kTrackBonus = 6;

    std::array<EnergyTrack,2> mTracks;   // 始终两条
    void generateTracks();               // 生成 / 补全
    void feedTracks();                   // 每帧调用
    void renderTracks();                 // 绘制
    void updateEnergyMode();
    void renderEnergyMode();

    // 普通食物
    sf::Texture mTexFoodNormal;
    sf::Sprite  mSprFoodNormal;

    // 回血食物（心形）
    sf::Texture mTexFoodLife;
    sf::Sprite  mSprFoodLife;

    // 障碍物
    sf::Texture mTexObstacle;
    sf::Sprite  mSprObstacle;

    // 分数模式多彩食物（假设有 4 种不同图标）
    static constexpr int kNumScoreTypes = 4;
    std::array<sf::Texture, kNumScoreTypes> mTexScoreFoods;
    std::vector<sf::Sprite>  mSprScoreFoods; 

    sf::Texture              mTexPortal;
    sf::Sprite               mSprPortal;

    sf::RenderWindow    mVictoryDialog;
    sf::Text            mMessage;

    sf::RectangleShape mVictoryResumeBtn;
    sf::Text           mVictoryResumeTxt;

    sf::RectangleShape mVictoryNextBtn;
    sf::Text           mVictoryNextTxt;
    sf::RectangleShape mVictoryHomeBtn;
    sf::Text           mVictoryHomeTxt;

    sf::RectangleShape mVictoryQuitBtn;
    sf::Text           mVictoryQuitTxt;

    bool                mNextHover = false;  // ← 记录鼠标是否悬停在 Next 上

    AppState mCurrentMode;
    AppState mOutcome = AppState::Exit;
    
    bool mVictoryHandled1 = false;
    bool mVictoryHandled2 = false; 
    bool mVictoryHandled3 = false;

    sf::Music       mVictoryMusic;
    
};

#endif
