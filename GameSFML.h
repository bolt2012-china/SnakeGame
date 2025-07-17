#ifndef GAME_SFML_H
#define GAME_SFML_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include "snake.h"
#include "GameApp.h"  
#include "Overlay.h"        
#include <algorithm>
#include <vector>

enum class GameState { Playing, Paused, GameOver, Victory };

class GameSFML {
public:
    GameSFML(unsigned int columns, unsigned int rows, unsigned int cellSize);
    AppState run();
    AppState runPortalMode();  
    AppState runScoreMode(); 
    AppState runEnergyTrackMode();
    bool hitBoundary();
    bool hitObstacles();
    bool isSnakeInCenterArea(); 
    void openVictoryDialog(bool* resumeFlag = nullptr);

private:
    
    void drawBackground(); 
    void render();
    void renderFrame();
    void renderBoard();
    void renderNewBoard();
    void renderSnake();
    void renderFood();
    void renderPairedFood(); 
    void renderObstacles();
    void renderUI();
    void renderPortalMode();
    void renderScoreFood();
    void renderScoreTunnels(); 
    void renderScoreMode();
    void renderLifeFood(); 
    void renderTracks();
    void renderEnergyMode();

    void processEvents();
    void openGameOverDialog();
    void processDialogEvents();
    void restartGame();

    void update();
    void updatePortalMode();
    void updateScoreMode();
    void updateLifeFood(float dt); 
    void updateEnergyMode();

    void generateFood(); 
    void generateLifeFood();      
    void generatePairedFood(); 
    void generateScoreFood(); 
    void generateScoreTunnels();
    void generateObstacles(); 
    void generateScoreObstacles(); 
    void generateTracks();               
    void feedTracks();                  
    
    void playSfx(const sf::SoundBuffer& buf);
    bool scoreFoodAt(int x, int y) const;  //Check whether the grid is occupied.
    static bool pointInPolygon(float px,float py, const std::vector<sf::Vector2f>& poly); //Auxiliary polygon functions.
    
    sf::RenderWindow mWindow;
    unsigned int mColumns;
    unsigned int mRows;
    unsigned int mCellSize;
    Snake mSnake;

    GameState mState = GameState::Playing;
    
    sf::RectangleShape mSidebarBorder;
    sf::RectangleShape mGameBorder; 
    enum class mBorderType { Circle, Diamond };
    sf::CircleShape mCircleBorder;
    sf::ConvexShape mDiamondBorder;
    mBorderType currentType;

    SnakeBody mFood;
    int mPoints = 0;
    int mDifficulty = 0;
    float mDelay = 0.1f;
    static constexpr float MIN_DELAY = 0.04f;   // Global speed limit

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

    // HighScores 
    enum ModeIndex { LevelModeIdx=0,EnergyTrackModeIdx=1, PortalModeIdx=2, PointsModeIdx=3, };
    static constexpr int kNumModes = 4;
    std::array<int, kNumModes> mModeHighScores;
    sf::Text mHighScoresTitleText;
    std::vector<sf::Text> mModeHighScoreTexts;
    static const std::array<std::string, kNumModes> modeNames;

    void updateHighScores(int score, AppState mode);  
    void loadHighScores();                            
    void saveHighScores();                            

    //Game-Over
    sf::RenderWindow mDialog;        
    sf::RectangleShape mRestartBtn;
    sf::RectangleShape mQuitBtn;
    sf::RectangleShape mHomeDBtn;
    sf::Text mRestartTxt;
    sf::Text mQuitTxt;
    sf::Text mHomeDTxt;
    sf::Text mGameOverMsg;
    sf::Font mGameOverFont;  

    // HP
    float mHitEffectTimer = 0.0f;
    float mInvincibleTimer = 0.0f;
    bool visible = true;

    //Portal Food
    std::array<SnakeBody,2>  mPortals; 

    // Score Mode Food
    struct ColoredFood {
        SnakeBody position;
        sf::Color color;
        int value;  
        int typeIndex; 
    };
    std::vector<ColoredFood> mScoreFoods;

    // Score Mode Tunnel
    struct ScoreTunnel {
        SnakeBody entrance;  
        SnakeBody exit;     
        sf::Color color;    
        int bonusPoints;    
        bool isActive;      
    };
    std::vector<ScoreTunnel> mScoreTunnels;

    std::vector<SnakeBody> mObstacles; 
    int mObstacleCount = 5;
    bool mNewBoardActivated = false; // Flag to track if new board has been rendered

    sf::Texture mBgTexture;  
    sf::Sprite  mBgSprite;   
    sf::Texture mHeadTexture;  
    sf::Sprite  mHeadSprite;

    sf::RectangleShape mPauseBtn; 
    sf::Text mPauseTxt;
    bool mPauseHover = false;
    
    //Pause menu button
    sf::RectangleShape mHomeBtn;
    sf::Text mHomeTxt;
    sf::RectangleShape mPauseQuitBtn;
    sf::Text mPauseQuitTxt;
    bool mOverlayHoverHome   = false;
    bool mOverlayHoverQuit   = false;   

    struct FoodQuota { sf::Color color; int value; int quota; };
    static const std::array<FoodQuota,4> kFoodQuotas;

    //Life Food
    SnakeBody mLifeFood;          
    bool mHasLifeFood = false;
    float mLifeElapsed = 0.f;

    //SFX buffers 
    sf::SoundBuffer mBufTurn, mBufEat, mBufEatSpec,
                mBufLife, mBufPort, mBufTunnel,
                mBufLoseHP, mBufDeath;
    std::vector<sf::Sound> mSounds;

    //EnergyTrack
    struct EnergyTrack {
        std::vector<SnakeBody> cells;   
        std::size_t progress = 0;       
        bool reverse = false;     
    };
    static constexpr int kTrackBonus = 6;
    std::array<EnergyTrack,2> mTracks;  

    //icon
    sf::Texture mTexFoodNormal;
    sf::Sprite  mSprFoodNormal;
    sf::Texture mTexFoodLife;
    sf::Sprite  mSprFoodLife;
    sf::Texture mTexObstacle;
    sf::Sprite  mSprObstacle;
    static constexpr int kNumScoreTypes = 4;
    std::array<sf::Texture, kNumScoreTypes> mTexScoreFoods;
    std::vector<sf::Sprite>  mSprScoreFoods; 
    sf::Texture mTexPortal;
    sf::Sprite mSprPortal;

    //Victory
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
    bool                mNextHover = false;  
    AppState mCurrentMode;
    AppState mOutcome = AppState::Exit;
    bool mVictoryHandled1 = false;
    bool mVictoryHandled2 = false; 
    bool mVictoryHandled3 = false;
    sf::Music       mVictoryMusic;
    
};

#endif
