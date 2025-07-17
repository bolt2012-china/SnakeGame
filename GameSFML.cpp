#include "GameSFML.h"
#include "StartScreen.h" 
#include <SFML/System/Angle.hpp> 
#include <sstream>
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>     
#include <algorithm>  
#include <array>
#include <string> 

#ifndef M_PI             
#   define M_PI 3.14159265358979323846
#endif

const std::array<GameSFML::FoodQuota,4> GameSFML::kFoodQuotas{{
    {sf::Color::Red,    1, 5},
    {sf::Color::Green,  2, 4},
    {sf::Color::Blue,   3, 3},
    {sf::Color::Yellow, 5, 2}
}};

const std::array<std::string, GameSFML::kNumModes> GameSFML::modeNames = {
    "Level Mode",
    "Energy Track",
    "Portal Mode",
    "Points Mode"
};

namespace
{
    static std::vector<sf::Vector2f>
    shrinkPolygon(const sf::ConvexShape& shape, float factor, float cell)
    {
        std::vector<sf::Vector2f> out;
        
        for (std::size_t i = 0; i < shape.getPointCount(); ++i) {
            sf::Vector2f centroid{0.f, 0.f};
            for (std::size_t i = 0; i < shape.getPointCount(); ++i)
                centroid += shape.getPoint(i);
            centroid /= static_cast<float>(shape.getPointCount());  
            sf::Vector2f p = shape.getPoint(i);
            p = { centroid.x + (p.x - centroid.x) * factor,
                centroid.y + (p.y - centroid.y) * factor };
            out.push_back(p / cell);                  
        }
        return out;
    }

    void makeButton(sf::RectangleShape& rect,
                    sf::Text&            label,
                    const sf::Font&      font,
                    const std::string&   text,
                    const sf::Vector2f&  pos,
                    const sf::Vector2f&  size = {100.f, 40.f},
                    const sf::Color&     fill = sf::Color(200, 200, 200))
    {
        rect.setSize(size);
        rect.setFillColor(fill);
        rect.setPosition(pos);

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::Black);

        auto bounds = label.getLocalBounds();                 // FloatRect
        const sf::Vector2f center = bounds.position + bounds.size * 0.5f;
        label.setOrigin(center);                                 

        label.setPosition(pos + size * 0.5f);
    }

}


GameSFML::GameSFML(unsigned int columns,
                   unsigned int rows,
                   unsigned int cellSize)
: mColumns(columns)
, mRows(rows)
, mCellSize(cellSize)
, mWindow(
    sf::VideoMode({ columns * cellSize + 240u,
                    rows    * cellSize }),
    "Snake Game")
, mSnake(columns, rows, 2)
, mFont()
, mBaloo2Bold()
, mRussoOne()
, mGameOverFont()
, mPointsLabel(mBaloo2Bold)
, mPointsValue(mRussoOne)
, mLivesText  (mBaloo2Bold) 
, mDifficultyText(mBaloo2Bold)
, mInstructionText(mFont)
, mUpText(mFont)
, mDownText(mFont)
, mLeftText(mFont)
, mRightText(mFont)
, mRestartTxt(mFont)   
, mQuitTxt  (mFont)
, mHomeDTxt (mFont)    
, mBgSprite(mBgTexture) 
, mHeadTexture{}
, mHeadSprite(mHeadTexture)
, mPauseTxt (mFont)  
, mHomeTxt  (mFont) 
, mPauseQuitTxt (mFont) 
, mHighScoresTitleText{ mBaloo2Bold,  "",  24 }
, mModeHighScoreTexts()  
, mModeHighScores{}
, mGameOverMsg(mGameOverFont)
, mTexFoodNormal()
, mSprFoodNormal(mTexFoodNormal)
, mTexFoodLife()
, mSprFoodLife(mTexFoodLife)
, mTexObstacle()
, mSprObstacle(mTexObstacle)
, mTexScoreFoods()
, mSprScoreFoods()
, mTexPortal()
, mSprPortal(mTexPortal)
, mMessage(mFont)
, mVictoryResumeTxt(mFont)
, mVictoryNextTxt(mFont)
,mVictoryHomeTxt(mFont)
,mVictoryQuitTxt(mFont)
{
    auto load = [](sf::SoundBuffer& buf, const std::string& file){
    if (!buf.loadFromFile(file))
        std::cerr << "Cannot open " << file << '\n';
    };

    load(mBufTurn,     "assets/music/snake_move.wav"); 
    load(mBufEat,      "assets/music/eat.wav");
    load(mBufEatSpec,  "assets/music/eat.wav"); 
    load(mBufLife,     "assets/music/life.wav"); 
    load(mBufPort,     "assets/music/potral.wav"); 
    load(mBufTunnel,   "assets/music/portral.wav"); 
    load(mBufLoseHP,   "assets/music/loseHp.wav"); 
    load(mBufDeath,    "assets/music/gameover.wav");
    mVictoryMusic.openFromFile("assets/music/win.ogg");
    mVictoryMusic.setLooping(false);

    if (!mBgTexture.loadFromFile("assets/nnb.gif")) {
        std::cerr << "Cannot open assets/bg.jpg\n";
    } else {
        unsigned int gameWidth = mColumns * mCellSize;
        unsigned int gameHeight = mRows * mCellSize;

        sf::Vector2u texSize = mBgTexture.getSize();

        mBgSprite.setTextureRect(sf::IntRect{
            sf::Vector2i{0, 0},
            sf::Vector2i{static_cast<int>(texSize.x), static_cast<int>(texSize.y)}
        });
        mBgSprite.setScale(sf::Vector2f(
            float(gameWidth) / texSize.x,
            float(gameHeight) / texSize.y
        ));
        mBgSprite.setPosition(sf::Vector2f(0.f, 0.f));

    }
    if (!mHeadTexture.loadFromFile("assets/snakehead.jpg")) {
        std::cerr << "Cannot open assets/head.jpg\n";
    } else {
         mHeadSprite.setTexture(mHeadTexture, /*resetRect=*/true);
        float target = float(mCellSize - 1);
        auto  tex    = mHeadTexture.getSize();
        float scale  = target / std::max(tex.x, tex.y);
        mHeadSprite.setScale({scale, scale});
        mHeadSprite.setOrigin(
            sf::Vector2f(tex.x * 0.5f, tex.y * 0.5f));
    }

    if (!mFont.openFromFile("assets/fonts/Baloo2-Bold.ttf")) {
        std::cerr << "ERROR: cannot load arial.ttf – check path!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!mBaloo2Bold.openFromFile("assets/fonts/Baloo2-Bold.ttf")) {
        std::cerr << "Cannot open assets/fonts/Orbitron.ttf\n";
        std::exit(EXIT_FAILURE);
    }
    if (!mRussoOne.openFromFile("assets/fonts/RussoOne.ttf")) {
        std::cerr << "Cannot open assets/fonts/RussoOne.ttf\n";
        std::exit(EXIT_FAILURE);
    }
    if (!mGameOverFont.openFromFile("assets/fonts/Nosifer.ttf")) {
        std::cerr << "Cannot open assets/fonts/Nosifer.ttf\n";
        std::exit(EXIT_FAILURE);
    }

    mGameOverMsg.setFont(mGameOverFont);
    mGameOverMsg.setString("Snake is dead.");
    mGameOverMsg.setCharacterSize(52);
    mGameOverMsg.setFillColor(sf::Color::White);
    float sidebarW = 240.f;
    float sidebarH = static_cast<float>(mRows * mCellSize);

    mSidebarBorder.setSize({sidebarW - 2.f, sidebarH - 2.f});
    mSidebarBorder.setPosition({static_cast<float>(mColumns * mCellSize) + 1.f, 1.f});
    mSidebarBorder.setFillColor(sf::Color(51, 48, 46));
    mSidebarBorder.setOutlineThickness(2.f);
    mSidebarBorder.setOutlineColor(sf::Color::White);

    const float centerX  = columns * cellSize + sidebarW * 0.5f;
    //Points
    mPointsLabel.setString("POINTS");
    mPointsLabel.setCharacterSize(32);
    mPointsLabel.setFillColor(sf::Color::White);
    auto b = mPointsLabel.getLocalBounds();
    mPointsLabel.setOrigin({ b.position.x + b.size.x * 0.5f,
                         b.position.y + b.size.y * 0.5f });
    mPointsLabel.setPosition({centerX, 55.f});
    mPointsValue.setString("0");
    mPointsValue.setCharacterSize(52);
    mPointsValue.setFillColor(sf::Color::White);
    b = mPointsValue.getLocalBounds();
    mPointsValue.setOrigin({ b.position.x + b.size.x * 0.5f,
                         b.position.y + b.size.y * 0.5f });
    mPointsValue.setPosition({centerX, 110.f});

    //Lives
    mLivesText.setString("Lives: 1");
    mLivesText.setCharacterSize(26);
    mLivesText.setFillColor(sf::Color::White);
    b = mLivesText.getLocalBounds();
    mLivesText.setOrigin({ b.position.x + b.size.x * 0.5f,
                       b.position.y + b.size.y * 0.5f });
    mLivesText.setPosition({centerX, 165.f});

    //Difficulty
    mDifficultyText.setString("Difficulty: 0");
    mDifficultyText.setCharacterSize(26);
    mDifficultyText.setFillColor(sf::Color::White);
    b = mDifficultyText.getLocalBounds();
    mDifficultyText.setOrigin({ b.position.x + b.size.x * 0.5f,
                            b.position.y + b.size.y * 0.5f });
    mDifficultyText.setPosition({centerX, 200.f});


    //Pause/Resume
    const float btnW = 180.f, btnH = 60.f;
    mPauseBtn.setSize({btnW, btnH});
    mPauseBtn.setPosition({ float(mColumns * mCellSize + 35),
                        float(mRows    * mCellSize - 120.f) });
    mPauseBtn.setFillColor(sf::Color::White);
    mPauseTxt.setCharacterSize(32);
    mPauseTxt.setFillColor(sf::Color::Black);
    mPauseTxt.setString("Pause");
    {
        auto r = mPauseTxt.getLocalBounds();
        mPauseTxt.setOrigin({ r.position.x + r.size.x*0.5f,
                          r.position.y + r.size.y*0.5f });
    }
    mPauseTxt.setPosition(
        mPauseBtn.getPosition() + sf::Vector2f(btnW/2.f, btnH/2.f - 10.f)
    );
    
    // Pause Button
    mHomeBtn.setSize({220.f, 60.f});
    mHomeBtn.setFillColor(sf::Color(60,60,60,220));
    mHomeTxt.setCharacterSize(30);
    mHomeTxt.setString("Home");
    sf::FloatRect hb = mHomeTxt.getLocalBounds();
    mHomeTxt.setOrigin(sf::Vector2f(hb.size.x*0.5f, hb.size.y*0.5f));
    mPauseQuitBtn.setSize({220.f, 60.f});
    mPauseQuitBtn.setFillColor(sf::Color(60,60,60,220));
    mPauseQuitTxt.setCharacterSize(30);
    mPauseQuitTxt.setString("Quit");
    sf::FloatRect qb = mPauseQuitTxt.getLocalBounds();
    mPauseQuitTxt.setOrigin(sf::Vector2f(qb.size.x*0.5f, qb.size.y*0.5f));

    // Instruction
    mInstructionText.setCharacterSize(18);
    float baseY     = sidebarH * 0.5f + 20.f;
    float textX     = float(mColumns * mCellSize + 30);
    mInstructionText.setPosition({ textX, baseY });
    mInstructionText.setString("Instruction\n WASD: Move\nR: Restart\nEsc: Quit");
    mUpText.setCharacterSize(18);
    mUpText.setPosition({ textX, baseY + 60.f });
    mUpText.setString("Up: W");
    mDownText.setCharacterSize(18);
    mDownText.setPosition({ textX, baseY + 90.f });
    mDownText.setString("Down: S");
    mLeftText.setCharacterSize(18);
    mLeftText.setPosition({ textX, baseY + 120.f });
    mLeftText.setString("Left: A");
    mRightText.setCharacterSize(18);
    mRightText.setPosition({ textX, baseY + 150.f });
    mRightText.setString("Right: D");

    //High Scores
    mModeHighScores.fill(0);
    mHighScoresTitleText.setFillColor(sf::Color::White);
    mHighScoresTitleText.setString("High Scores");
    mHighScoresTitleText.setPosition({ textX, baseY - 300.f });   
    mModeHighScoreTexts.reserve(kNumModes);
    for (int i = 0; i < kNumModes; ++i) {
        mModeHighScoreTexts.emplace_back(
            mBaloo2Bold,
            GameSFML::modeNames[i] + ": " + std::to_string(mModeHighScores[i]),
            20
        );
        mModeHighScoreTexts[i].setFillColor(sf::Color::White);
        mModeHighScoreTexts[i].setPosition({ textX - 10.f, baseY - 250.f + i * 40.f });
       
    }
    
    loadHighScores();
    generateFood();
    generateObstacles();
    
    // Diamond shaped map
    mDiamondBorder.setPointCount(4);
    mDiamondBorder.setPoint(0,{ float(mColumns*mCellSize/2), 0.f});
    mDiamondBorder.setPoint(1,{ float(mColumns*mCellSize), float(mRows*mCellSize/2)});
    mDiamondBorder.setPoint(2,{ float(mColumns*mCellSize/2), float(mRows*mCellSize)});
    mDiamondBorder.setPoint(3,{ 0.f, float(mRows*mCellSize/2)});
    
    constexpr float PI = 3.14159265f;

    // Initialize the random number generator
    std::srand(static_cast<unsigned>(time(nullptr)));
    currentType = mBorderType::Circle; 
    mNewBoardActivated = false; // Initialize the flag
    float target = float(mCellSize - 1);
    if (!mTexFoodNormal.loadFromFile("assets/icons/fries.png"))
        std::cerr << "Failed to load food_normal.png\n";
    mSprFoodNormal.setTexture(mTexFoodNormal, true);
    {
        auto tsz = mTexFoodNormal.getSize();
        float s = target / std::max(tsz.x, tsz.y);
        mSprFoodNormal.setScale({ s, s });
    }
    if (!mTexFoodLife.loadFromFile("assets/icons/heart.png"))
        std::cerr << "Failed to load food_life.png\n";
    mSprFoodLife.setTexture(mTexFoodLife, true);
    {
        auto tsz = mTexFoodLife.getSize();
        float s = target / std::max(tsz.x, tsz.y);
        mSprFoodLife.setScale({s, s});
    }
    if (!mTexObstacle.loadFromFile("assets/icons/obstacle.png"))
        std::cerr << "Failed to load obstacle.png\n";
    mSprObstacle.setTexture(mTexObstacle, true);
    {
        auto tsz = mTexObstacle.getSize();
        float s = (target - 1) / std::max(tsz.x, tsz.y); // 障碍比食物小 1px
        mSprObstacle.setScale({s, s});
    }
    std::array<std::string, kNumScoreTypes> paths = {
        "assets/icons/fries.png",
        "assets/icons/pizza.png",
        "assets/icons/ham.png",
        "assets/icons/burger.png"
    };
    mSprScoreFoods.clear();
    mSprScoreFoods.reserve(kNumScoreTypes);
    for (int i = 0; i < kNumScoreTypes; ++i) {
        if (!mTexScoreFoods[i].loadFromFile(paths[i]))
            std::cerr << "Failed to load " << paths[i] << '\n';
        sf::Sprite spr(mTexScoreFoods[i]);
        auto tsz = mTexScoreFoods[i].getSize();
        float s = target / std::max(tsz.x, tsz.y);
        spr.setScale({s, s});
        mSprScoreFoods.emplace_back(std::move(spr));
    }
    if (!mTexPortal.loadFromFile("assets/icons/apple.png"))
        std::cerr << "Failed to load portal.png\n";
    mSprPortal.setTexture(mTexPortal);
    {
        auto tsz = mTexPortal.getSize();
        float s = float(mCellSize - 1) / std::max(tsz.x, tsz.y);
        mSprPortal.setScale({s, s});
    }
}

AppState GameSFML::run()
{
    mCurrentMode = AppState::LevelMode;
    Overlay ready({mColumns * mCellSize, mRows * mCellSize},
              "Ready Go !!!",
              "assets/fonts/RussoOne.ttf",   
              120,
              "assets/music/ready.wav",
              sf::Color(0,0,0,80));   

    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false; 

    while (mWindow.isOpen()) {
        float dt = clock.restart().asSeconds();    
        processEvents();                
        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();      
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();
        if (mState == GameState::Playing) { 
            acc += dt;
            if (acc >= effectiveDelay) { update(); acc -= effectiveDelay; }
            if (!mVictoryHandled1 && mPoints >= 15) {
                updateHighScores(mPoints, AppState::LevelMode);
                mState = GameState::Victory;
                mVictoryHandled1 = true;
                openVictoryDialog();
                if (mState != GameState::Playing) {
                return mOutcome;
                }
            }
        }

        if (mInvincibleTimer > 0) {
            mInvincibleTimer -= dt;
            mHitEffectTimer = mInvincibleTimer;
        }
        if (mHitEffectTimer > 0) {
            visible = static_cast<int>(mHitEffectTimer * 10) % 2 == 0;
            mHitEffectTimer -= dt;
        }
        render();                       
    }
    return mOutcome;
}

void GameSFML::openVictoryDialog(bool* resumeFlag)
{
    switch (mCurrentMode) {
        case AppState::LevelMode:        unlockedEnergyTrack = true; break;
        case AppState::EnergyTrackMode:  unlockedPortal      = true; break;
        case AppState::PortalMode:       unlockedPoints      = true; break;
        default: break;
    }

    if (mVictoryDialog.isOpen()) return;
    mVictoryDialog.close();
    mVictoryDialog.create(
        sf::VideoMode(sf::Vector2u{600u, 350u}),                     
        "Victory",                                         
        sf::Style::Titlebar | sf::Style::Close         
    );
    
    mVictoryMusic.stop();  
    mVictoryMusic.play();   
    
    mFont = sf::Font{"assets/fonts/Marker.ttf"};     
    mMessage.setFont(mFont);
    mMessage.setCharacterSize(52u);
    mMessage.setString("You Win!");
    auto mb = mMessage.getLocalBounds();
    mMessage.setOrigin({ mb.size.x * 0.5f, mb.size.y * 0.5f });  
    mMessage.setPosition({ 300.f, 100.f });                    
    mMessage.setFillColor(sf::Color::White);

    sf::Vector2f btnSize{160.f, 60.f};
    float gapX = (600.f - btnSize.x*2)/3.f;
    float y1   = 180.f;
    float y2   = 260.f;
    makeButton(mVictoryResumeBtn, mVictoryResumeTxt, mBaloo2Bold,
               "Resume", {gapX,      y1}, btnSize , sf::Color::White);
    makeButton(mVictoryNextBtn,    mVictoryNextTxt,    mBaloo2Bold,
               "Next",   {gapX*2+btnSize.x, y1}, btnSize, sf::Color::White);
    makeButton(mVictoryHomeBtn,    mVictoryHomeTxt,    mBaloo2Bold,
               "Home",   {gapX,      y2}, btnSize, sf::Color::White);
    makeButton(mVictoryQuitBtn,    mVictoryQuitTxt,    mBaloo2Bold,
               "Quit",   {gapX*2+btnSize.x, y2}, btnSize, sf::Color::White);
    mNextHover = false;

    while (mVictoryDialog.isOpen()) {
        while (auto eventOpt = mVictoryDialog.pollEvent())
        {
            auto& ev = *eventOpt;

            if (ev.is<sf::Event::Closed>())
            {
                mOutcome = AppState::StartMenu;
                mVictoryDialog.close();
                return;
            }
            else if (auto* mm = ev.getIf<sf::Event::MouseMoved>())      
            {
                sf::Vector2f mp{ float(mm->position.x), float(mm->position.y) };
                bool now = mVictoryNextBtn.getGlobalBounds().contains(mp);
                if (now != mNextHover)
                {
                    mNextHover = now;
                    mVictoryNextTxt.setFillColor(
                        mNextHover ? sf::Color::Yellow : sf::Color::White
                    );
                }
            }
            else if (auto* mb = ev.getIf<sf::Event::MouseButtonPressed>())
            {
                if (mb->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2f mp{ float(mb->position.x), float(mb->position.y) };

                    // Resume
                    if (mVictoryResumeBtn.getGlobalBounds().contains(mp))
                    {
                        mSnake.setSpeedMultiplier(1.0f);
                        if (resumeFlag) *resumeFlag = true;
                        mState = GameState::Playing;
                        mVictoryDialog.close();
                        break;  
                    }
                    // Next
                    if (mVictoryNextBtn.getGlobalBounds().contains(mp))
                    {
                        switch (mCurrentMode)
                        {
                            case AppState::LevelMode:
                            mOutcome = AppState::EnergyTrackMode; break;
                        case AppState::EnergyTrackMode:
                            mOutcome = AppState::PortalMode;      break;
                        case AppState::PortalMode:
                            mOutcome = AppState::ScoreMode;       break;
                        default:
                            mOutcome = AppState::StartMenu;       break;
                        }
                        mVictoryDialog.close();
                        return;
                    }
                    // Home
                    if (mVictoryHomeBtn.getGlobalBounds().contains(mp))
                    {
                        mVictoryDialog.close();
                        mOutcome = AppState::StartMenu;
                        return;
                    }
                    // Quit
                    if (mVictoryQuitBtn.getGlobalBounds().contains(mp))
                    {
                        mVictoryDialog.close();
                        mOutcome = AppState::Exit;
                        return;
                    }
                }
            }
        }
        mVictoryDialog.clear(sf::Color::Red);
        mVictoryDialog.draw(mMessage);
        mVictoryDialog.draw(mVictoryResumeBtn);
        mVictoryDialog.draw(mVictoryResumeTxt);
        mVictoryDialog.draw(mVictoryNextBtn);
        mVictoryDialog.draw(mVictoryNextTxt);
        mVictoryDialog.draw(mVictoryHomeBtn);
        mVictoryDialog.draw(mVictoryHomeTxt);
        mVictoryDialog.draw(mVictoryQuitBtn);
        mVictoryDialog.draw(mVictoryQuitTxt);
        mVictoryDialog.display();
    }
    mVictoryMusic.stop();
}

void GameSFML::processEvents()
{
    while (auto eventOpt = mWindow.pollEvent()) {
        const auto& event = *eventOpt;
        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }
        else if (auto key = event.getIf<sf::Event::KeyPressed>()) {
            Direction currentDir = mSnake.getDirection();
            Direction newDir     = currentDir;

            switch (key->scancode) {
                case sf::Keyboard::Scancode::W:
                case sf::Keyboard::Scancode::Up:    newDir = Direction::Up;    break;
                case sf::Keyboard::Scancode::S:
                case sf::Keyboard::Scancode::Down:  newDir = Direction::Down;  break;
                case sf::Keyboard::Scancode::A:
                case sf::Keyboard::Scancode::Left:  newDir = Direction::Left;  break;
                case sf::Keyboard::Scancode::D:
                case sf::Keyboard::Scancode::Right: newDir = Direction::Right; break;
                case sf::Keyboard::Scancode::R:
                    mPoints = 0;
                    mSnake.initializeSnake();
                    do {
                        int fx = std::rand() % mColumns;
                        int fy = std::rand() % mRows;
                        mFood  = SnakeBody(fx, fy);
                    } while (mSnake.isPartOfSnake(mFood.getX(), mFood.getY()));
                    mSnake.senseFood(mFood);
                    mDelay       = 0.1f;
                    currentType  = static_cast<mBorderType>(std::rand() % 2);
                    break;
                case sf::Keyboard::Scancode::Escape:
                    mWindow.close();
                    break;
                default:
                    break;
            }
            if (newDir == currentDir) {
                mSnake.setSpeedMultiplier(1.5f);
                if (!mSnake.isAccelerating())
                    mSnake.setAccelerationEffect(true);
            }
            else if (newDir != currentDir) {
                mSnake.setSpeedMultiplier(1.0f);
                mSnake.changeDirection(newDir);
                playSfx(mBufTurn);
            }
        }
        else if (const auto* mv = event.getIf<sf::Event::MouseMoved>()) {
            sf::Vector2f p{ float(mv->position.x), float(mv->position.y) };

            bool hoverNow = mPauseBtn.getGlobalBounds().contains(p);
            if (hoverNow != mPauseHover) {
                mPauseHover = hoverNow;
                mPauseBtn.setFillColor(hoverNow
                    ? sf::Color(220,220,220) 
                    : sf::Color::White    
                );
            }
            if (mState == GameState::Paused) {
                bool hHover = mHomeBtn.getGlobalBounds().contains(p);
                if (hHover != mOverlayHoverHome) {
                    mOverlayHoverHome = hHover;
                    mHomeBtn.setFillColor(hHover ? sf::Color(90,90,90,220)
                                                 : sf::Color(60,60,60,220));
                }

                bool qHover = mPauseQuitBtn.getGlobalBounds().contains(p);
                if (qHover != mOverlayHoverQuit) {
                    mOverlayHoverQuit = qHover;
                    mPauseQuitBtn.setFillColor(qHover ? sf::Color(90,90,90,220)
                                                      : sf::Color(60,60,60,220));
                }
            }
        }
        else if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button != sf::Mouse::Button::Left) continue;
            sf::Vector2f p{ float(click->position.x), float(click->position.y) };
            if (mState == GameState::Paused) {
                if (mHomeBtn.getGlobalBounds().contains(p)) {
                    mOutcome = AppState::StartMenu;  
                    mWindow.close();
                    continue;
                }
                if (mPauseQuitBtn.getGlobalBounds().contains(p)) {
                    std::exit(0);            
                }
            }
            if (mPauseBtn.getGlobalBounds().contains(p)) {
                mState = (mState == GameState::Playing)
                         ? GameState::Paused
                         : GameState::Playing;
            }
        }
    }
}

void GameSFML::update() {
    if (mSnake.moveFoward()) {
        ++mPoints;
        playSfx(mBufEat);
        mDifficulty = mPoints / 5;
        
        if (mPoints % 5 == 0) {
            ++mObstacleCount;
            generateObstacles();
            mBorderType prev = currentType;
            int phase = (mPoints / 5 - 1) % 2;    
            currentType =
                (phase == 0) ? mBorderType::Circle  :
                                mBorderType::Diamond ;
            if (currentType != prev)
                mNewBoardActivated = false;
        }
        generateFood();
    }

    if (mDifficulty >= 1 && !mNewBoardActivated && isSnakeInCenterArea()) {

        Overlay unlock({mColumns * mCellSize, mRows * mCellSize},
                   "Unlock new maps!",                 
                   "assets/fonts/Baloo2-Bold.ttf",     
                   72,                               
                   "assets/music/unlock.wav",          
                   sf::Color(0, 0, 0, 140));          

        unlock.play(mWindow, 0.5f, [this]{ renderFrame(); });
        mNewBoardActivated = true;  
    }

    // Check boundary collision based on board state
    bool shouldCheckBoundary = false;
    if (mDifficulty < 1) {
        shouldCheckBoundary = true;
    } else if (mNewBoardActivated) {
        shouldCheckBoundary = true;
    }
    
    if (shouldCheckBoundary && hitBoundary() && mInvincibleTimer <= 0.f) {      
        mSnake.decreaseHitPoints();
        if (mSnake.getHitPoints() <= 0) {
            updateHighScores(mPoints, AppState::LevelMode); 
            playSfx(mBufDeath);
            mState = GameState::GameOver;   
            openGameOverDialog();          
        }
        else {
            playSfx(mBufLoseHP);
            mInvincibleTimer = 1.0f; 
        }
    }
    hitObstacles();
}

void GameSFML::drawBackground()
{
    if (mBgTexture.getSize().x)   
        mWindow.draw(mBgSprite);
}

void GameSFML::renderFrame() {
    mWindow.clear();                  
    drawBackground();    
    if (mDifficulty == 0) {
        renderBoard(); 
    } else if (mDifficulty >= 1) {
        // Render new board if it has been activated, otherwise continue showing basic board
        if (mNewBoardActivated) {
            renderNewBoard(); 
        } else {
            renderBoard(); // Continue showing basic board until snake enters center
        }
    }
    renderObstacles();
    renderFood();
    renderSnake();
    mWindow.draw(mSidebarBorder);
    renderUI();
    if (mState == GameState::Paused) {
        sf::RectangleShape mask({ float(mColumns*mCellSize),
                              float(mRows   *mCellSize) });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);

        sf::Text paused(mFont, "PAUSED", 64);
        sf::FloatRect b = paused.getLocalBounds();
        paused.setOrigin(sf::Vector2f(b.size.x * 0.5f,
                                b.size.y * 0.5f));
        paused.setPosition(sf::Vector2f(
            float(mColumns * mCellSize) * 0.5f,
            float(mRows    * mCellSize) * 0.5f - 32.f));

        mWindow.draw(paused);

        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        float btnY   = overlayCy + 80.f;
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW = sz.x * 0.5f;
        float spacing   = 20.f;
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;
        
        mHomeBtn .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt.setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        mPauseQuitBtn.setOrigin({ halfW, sz.y * 0.5f });
        mPauseQuitBtn.setPosition({ x2, btnY });
        mPauseQuitTxt.setOrigin({
            mPauseQuitTxt.getLocalBounds().position.x + mPauseQuitTxt.getLocalBounds().size.x * 0.5f,
            mPauseQuitTxt.getLocalBounds().position.y + mPauseQuitTxt.getLocalBounds().size.y * 0.5f
        });
        mPauseQuitTxt.setPosition(mPauseQuitBtn.getPosition());
        mWindow.draw(mPauseQuitBtn);
        mWindow.draw(mPauseQuitTxt);
    }
}

void GameSFML::render()
{
    renderFrame();        
    mWindow.display();    
}

void GameSFML::renderBoard() {
    this->mGameBorder.setPosition(sf::Vector2f(4.f, 4.f));
    this->mGameBorder.setSize({mColumns * mCellSize-12.f, 
                         mRows * mCellSize-12.f});
}

void GameSFML::renderNewBoard() {

    float radius = std::min(mColumns, mRows) * mCellSize / 2.0f;
    this->mCircleBorder.setRadius(radius);
    this->mCircleBorder.setPosition(sf::Vector2f(mColumns * mCellSize / 2.0f - radius, 
                              mRows * mCellSize / 2.0f - radius));
    this->mCircleBorder.setFillColor(sf::Color::Transparent);
    this->mCircleBorder.setOutlineThickness(2.0f);
    this->mCircleBorder.setOutlineColor(sf::Color::White);
    mDiamondBorder.setFillColor(sf::Color::Transparent);
    mDiamondBorder.setOutlineThickness(2.f);
    mDiamondBorder.setOutlineColor(sf::Color::White);
    
    switch(currentType) {
        case mBorderType::Circle: {
            mCircleBorder.setOutlineColor(sf::Color(255,255,255, 80));
            mCircleBorder.setOutlineThickness(10.f);
            mWindow.draw(mCircleBorder);
            mCircleBorder.setOutlineColor(sf::Color(255,255,255, 160));
            mCircleBorder.setOutlineThickness(6.f);
            mWindow.draw(mCircleBorder);
            mCircleBorder.setOutlineColor(sf::Color::White);
            mCircleBorder.setOutlineThickness(3.f);
            mWindow.draw(mCircleBorder);
        } break;

        case mBorderType::Diamond: {
            mDiamondBorder.setOutlineColor(sf::Color(255,255,255, 80));
            mDiamondBorder.setOutlineThickness(10.f);
            mWindow.draw(mDiamondBorder);

            mDiamondBorder.setOutlineColor(sf::Color(255,255,255, 160));
            mDiamondBorder.setOutlineThickness(6.f);
            mWindow.draw(mDiamondBorder);

            mDiamondBorder.setOutlineColor(sf::Color::White);
            mDiamondBorder.setOutlineThickness(3.f);
            mWindow.draw(mDiamondBorder);
        } break;

    }
}

void GameSFML::renderSnake() {
    sf::RectangleShape body({ float(mCellSize - 1), float(mCellSize - 1) });
    body.setFillColor(sf::Color(255, 200, 50));

    const auto& parts = mSnake.getSnake();
    if (parts.empty()) return;

    const SnakeBody& head = parts.front();
    mHeadSprite.setPosition(
        { head.getX() * mCellSize + mCellSize * 0.5f,
        head.getY() * mCellSize + mCellSize * 0.5f });

    switch (mSnake.getDirection()) {
        case Direction::Up:    mHeadSprite.setRotation(sf::degrees(0.f)); break;
        case Direction::Down:  mHeadSprite.setRotation(sf::degrees(180.f)); break;
        case Direction::Left:  mHeadSprite.setRotation(sf::degrees(270.f)); break;
        case Direction::Right: mHeadSprite.setRotation(sf::degrees(90.f)); break;
    }

    mHeadSprite.setColor(mSnake.isAccelerating()
                         ? sf::Color::Red
                         : sf::Color::White);
    mWindow.draw(mHeadSprite);

    for (std::size_t i = 1; i < parts.size(); ++i) {
        const auto& p = parts[i];
        body.setPosition({ float(p.getX() * mCellSize),
                           float(p.getY() * mCellSize) });
        mWindow.draw(body);
    }
}

void GameSFML::generateFood() {
    std::srand(static_cast<unsigned>(time(nullptr)));
    while (true) {
        int fx = std::rand() % mColumns;
        int fy = std::rand() % mRows;

        if (mSnake.isPartOfSnake(fx, fy)) continue;

        bool onObstacle = false;
        for (const auto& obs : mObstacles) {
            if (obs.getX() == fx && obs.getY() == fy) {
                onObstacle = true;
                break;
            }
        }
        if (onObstacle) continue;
        bool valid = true;
        if (mDifficulty == 0) {
            if (fx == 0 || fx == mColumns - 1 || fy == 0 || fy == mRows - 1) {
                continue; // 重新生成坐标
            }
        } else {
            if (currentType == mBorderType::Circle) {
                float centerX = mColumns / 2.0f;
                float centerY = mRows / 2.0f;
                float radius = std::min(mColumns, mRows) / 2.0f;
                float dx = fx + 0.5f - centerX;
                float dy = fy + 0.5f - centerY;
                float distance = std::sqrt(dx*dx + dy*dy);
                if (distance >= radius - 1.0f) valid = false;
            } else if (currentType == mBorderType::Diamond) {
                float cx = mColumns/2.f, cy = mRows/2.f;
                float R  = std::min(mColumns,mRows)/2.f - 1.f; 
                float dx = std::fabs(fx + 0.5f - cx);
                float dy = std::fabs(fy + 0.5f - cy);
                if (dx + dy >  R) valid = false;
            }
        }
        
        if (valid) {
            mFood = SnakeBody(fx, fy);
            break;
        }
    }
    mSnake.senseFood(mFood);
}

void GameSFML::renderFood() {
    mSprFoodNormal.setPosition({
         float(mFood.getX() * mCellSize),
        float(mFood.getY() * mCellSize)
    });
    mWindow.draw(mSprFoodNormal);
}

void GameSFML::renderObstacles() {
    for (auto& obs : mObstacles) {
        mSprObstacle.setPosition({
            obs.getX() * float(mCellSize) + 1.f,
            obs.getY() * float(mCellSize) + 1.f
        });
        mWindow.draw(mSprObstacle);
    }
}

void GameSFML::renderPairedFood()
{
    for (const auto& f : mPortals) {
        mSprPortal.setPosition({float(f.getX() * mCellSize), float(f.getY() * mCellSize)});
        mSprPortal.setTexture(mTexPortal, /*resetRect=*/true);
        mWindow.draw(mSprPortal);
    }
}

void GameSFML::renderScoreFood() {
    for (auto& food : mScoreFoods) {
        int idx = food.typeIndex;
        auto& spr = mSprScoreFoods[idx];
        spr.setPosition({ float(food.position.getX() * mCellSize),
                  float(food.position.getY() * mCellSize) });
        mWindow.draw(spr);

    }
}

void GameSFML::renderUI() {

    mDifficultyText.setString("Difficulty: " + std::to_string(mDifficulty));
    mPointsValue   .setString(std::to_string(mPoints));
    mLivesText     .setString("Lives: "  + std::to_string(mSnake.getHitPoints()));

    mWindow.draw(mUpText);
    mWindow.draw(mDownText);
    mWindow.draw(mLeftText);
    mWindow.draw(mRightText);

    mWindow.draw(mDifficultyText);
    mWindow.draw(mPointsLabel);
    mWindow.draw(mPointsValue);
    mWindow.draw(mLivesText);

    mWindow.draw(mHighScoresTitleText);
    for (auto& txt : mModeHighScoreTexts)
        mWindow.draw(txt);
    
    mPauseTxt.setString(mState == GameState::Paused ? "Resume" : "Pause");
    sf::FloatRect r = mPauseTxt.getLocalBounds();
    mPauseTxt.setOrigin(sf::Vector2f(r.size.x * 0.5f,
                                    r.size.y * 0.5f));
    mWindow.draw(mPauseBtn);
    mWindow.draw(mPauseTxt);

}

void GameSFML::openGameOverDialog()
{
    if (mDialog.isOpen()) return;      

    mDialog.create(
        sf::VideoMode({600u, 350u}), "Game Over",
        sf::Style::Titlebar | sf::Style::Close);
    sf::FloatRect tb = mGameOverMsg.getLocalBounds();
    mGameOverMsg.setOrigin({ tb.size.x * 0.5f, tb.size.y * 0.5f });

    sf::Vector2f winSize = static_cast<sf::Vector2f>(mDialog.getSize());
    mGameOverMsg.setPosition({ winSize.x * 0.5f, winSize.y * 0.25f });

    const sf::Vector2f btnSize{120.f, 60.f};
    float gap = (winSize.x - btnSize.x * 3.f) / 4.f;
    float btnY = winSize.y * 0.6f;
    float x1 = gap;
    float x2 = x1 + btnSize.x + gap;
    float x3 = x2 + btnSize.x + gap;

    makeButton(mRestartBtn, mRestartTxt, mGameOverFont, "Restart", { x1, btnY }, btnSize);
    makeButton(mHomeDBtn,  mHomeDTxt,  mGameOverFont, "Home",    { x2, btnY }, btnSize);
    makeButton(mQuitBtn,   mQuitTxt,   mGameOverFont, "Quit",    { x3, btnY }, btnSize);

}

void GameSFML::processDialogEvents()
{
    while (auto evt = mDialog.pollEvent()) {
        // evt 是 std::optional<sf::Event>，
        if (evt->is<sf::Event::Closed>()) { 
            mWindow.close();
            mDialog.close();
            return;
        }
        else if (const auto* mouse = evt->getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f pos{ static_cast<float>(mouse->position.x),
                              static_cast<float>(mouse->position.y) };

            if (mRestartBtn.getGlobalBounds().contains(pos)) {
                restartGame();
                mState = GameState::Playing;
                mDialog.close();
            }
            else if (mHomeDBtn.getGlobalBounds().contains(pos)) {
                mOutcome = AppState::StartMenu;
                mWindow.close();
                mDialog.close();
            }
            else if (mQuitBtn.getGlobalBounds().contains(pos)) {
                mWindow.close();
                mDialog.close();
            }
        }
    }

    mDialog.clear(sf::Color(139,  0,  0));    
    mDialog.draw(mGameOverMsg);          
    mDialog.draw(mRestartBtn);    
    mDialog.draw(mRestartTxt);
    mDialog.draw(mHomeDBtn);      
    mDialog.draw(mHomeDTxt);
    mDialog.draw(mQuitBtn);       
    mDialog.draw(mQuitTxt);
    mDialog.display();
}

void GameSFML::restartGame()
{
    mPoints = 0;
    mDifficulty = 0;
    mDelay = 0.1f;
    mObstacleCount = 5;
    mObstacles.clear();

    mSnake.initializeSnake();
    generateFood();
    generateObstacles();
    currentType = mBorderType::Circle;  
    mNewBoardActivated = false; // Reset the flag on restart
    mSnake.resetHitPoints(); 

}

void GameSFML::loadHighScores()
{
    std::ifstream in("highscores.txt");
    if (!in) return;

    for (int i = 0; i < kNumModes && in; ++i)
        in >> mModeHighScores[i];
    for (int i = 0; i < kNumModes; ++i) {
        mModeHighScoreTexts[i].setString(
            modeNames[i] + ": " + std::to_string(mModeHighScores[i])
        );
    }
}

void GameSFML::saveHighScores()
{
    std::ofstream out("highscores.txt", std::ios::trunc);
    if (!out) {
        std::cerr << "Cannot write highscores.txt" << std::endl;
        return;
    }
    for (int s : mModeHighScores) out << s << '\n';
}

void GameSFML::updateHighScores(int score, AppState mode)
{
    int idx = 0;
    switch (mode) {
        case AppState::LevelMode:        idx = LevelModeIdx;        break;
        case AppState::EnergyTrackMode:  idx = EnergyTrackModeIdx;  break;
        case AppState::PortalMode:       idx = PortalModeIdx;       break;
        case AppState::ScoreMode:        idx = PointsModeIdx;       break;
        default: return;
    }
    if (score > mModeHighScores[idx]) {
        mModeHighScores[idx] = score;
        saveHighScores();
        mModeHighScoreTexts[idx].setString(
           GameSFML::modeNames[idx] + ": " + std::to_string(score)
        );
    }
}

bool GameSFML::hitBoundary()
{
    const auto& snake = mSnake.getSnake();
    if (snake.empty()) return false;

    SnakeBody head = snake.front();
    float headX = head.getX() * mCellSize + mCellSize * 0.5f;
    float headY = head.getY() * mCellSize + mCellSize * 0.5f;

    if (mDifficulty == 0) {            
        const auto& snakeCells = mSnake.getSnake();
        if (!snakeCells.empty()) {
            const SnakeBody& head = snakeCells.front();
            int x = head.getX();
            int y = head.getY();
            bool wallCollision = (x < 0 || x >= static_cast<int>(mColumns) ||
                                y < 0 || y >= static_cast<int>(mRows));
            bool selfCollision = mSnake.hitSelf();
            return wallCollision || selfCollision;
        }
        return false;
    }

    const float cx = mColumns * mCellSize * 0.5f;
    const float cy = mRows    * mCellSize * 0.5f;
    const float R  = std::min(mColumns, mRows) * mCellSize * 0.5f;
    const float MARGIN = mCellSize * 0.5f;   
    switch (currentType)
    {
        case mBorderType::Circle: {
            float dist = std::hypot(headX - cx, headY - cy);
            return dist >= R + MARGIN;
        }
        case mBorderType::Diamond: {
            float manhattan = std::fabs(headX - cx) + std::fabs(headY - cy);
            return manhattan >= R + MARGIN;   
        }
       
    }
    return false;   
}

AppState GameSFML::runPortalMode()
{
    mPoints         = 0;
    mDifficulty     = 0;
    mDelay          = 0.2f;
    mState          = GameState::Playing;
    mInvincibleTimer= 0.f;
    mHitEffectTimer = 0.f;
    mCurrentMode = AppState::PortalMode;
    loadHighScores(); 
    Overlay ready({mColumns * mCellSize, mRows * mCellSize},
              "Ready Go !!!",
              "assets/fonts/RussoOne.ttf",  
              80,
              "assets/music/ready.wav");   

    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false;
    Difficulty = 0;
    generatePairedFood();
    generateObstacles();

    while (mWindow.isOpen()) {
        
        float dt = clock.restart().asSeconds();
        if (resumeFlag) {
            acc = 0.f;
            resumeFlag = false;
        }
        processEvents();                    
        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();         

        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { 
            updateLifeFood(dt);
            if (mInvincibleTimer > 0.f) {
                mInvincibleTimer -= dt;
                mHitEffectTimer = mInvincibleTimer;
            }
            acc += dt;
            while (acc >= effectiveDelay) {
                updatePortalMode();
                acc -= effectiveDelay;
            }
        }

        if ( !mVictoryHandled3 && mPoints >= 35) {
            updateHighScores(mPoints, AppState::PortalMode);
            mVictoryHandled3 = true;
            mState = GameState::Victory;
            openVictoryDialog(&resumeFlag);
            if (mState != GameState::Playing)  return mOutcome;
        }
        
        renderPortalMode();             
        
    }
    return mOutcome;
}

void GameSFML::updatePortalMode() {
    // Check if snake touched portal food first
    SnakeBody next = mSnake.createNewHead();          
    int hit = -1;                                
    if (next == mPortals[0]) hit = 0;
    else if (next == mPortals[1]) hit = 1;

    if (mHasLifeFood && next == mLifeFood) {
        playSfx(mBufLife);
        if (mSnake.getHitPoints() < 3)  mSnake.increaseHitPoints();
        else mPoints += 5;
        mHasLifeFood = false;
    }

    if (hit != -1) {
        playSfx(mBufPort);
        mSnake.senseFood(mPortals[hit]);
        mSnake.moveFoward();          
        ++mPoints;
        int other = 1 - hit;
        mSnake.teleportToPosition( mPortals[other].getX(), mPortals[other].getY());
        mSnake.grow();             
        ++mPoints;
        generatePairedFood();       
        return;                       
    }

    
    if (mSnake.moveFoward()) {
        ++mPoints;
        // In portal mode, difficulty increases slower
        mDifficulty = mPoints / 10; 
        // Generate new food after eating regular food
        generatePairedFood();
    }
    
    // Portal mode uses basic rectangular boundaries only
    if (mSnake.checkCollision() && mInvincibleTimer <= 0.f ) { 
        mSnake.decreaseHitPoints();
        if (mSnake.getHitPoints() <= 0) {
            playSfx(mBufDeath);     
            updateHighScores(mPoints, AppState::PortalMode);
            mState = GameState::GameOver;
            openGameOverDialog();
        } else {  
            playSfx(mBufLoseHP);
            const auto& snakeVec = mSnake.getSnake();
            if (snakeVec.size() > 1) {
                const SnakeBody& prevHead = snakeVec[1];
                mSnake.teleportToPosition(prevHead.getX(), prevHead.getY());
            }
            mSnake.reverseDirection();
            mInvincibleTimer = 1.0f;
        }
    }
    hitObstacles();
}

void GameSFML::renderPortalMode() {
    mWindow.clear();
    drawBackground(); 
    // Always use basic rectangular board in portal mode
    renderBoard(); // This renders the basic rectangular border
    renderPairedFood();  // Renders both regular and portal food
    renderLifeFood();
    renderObstacles();
    renderSnake();
    mWindow.draw(mSidebarBorder);
    renderUI();

    if (mState == GameState::Paused) {
        sf::RectangleShape mask({
            float(mColumns * mCellSize),
            float(mRows    * mCellSize)
        });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);

        sf::Text paused(mFont, "PAUSED", 64);
        auto pb = paused.getLocalBounds();
        paused.setOrigin({
            pb.position.x + pb.size.x*0.5f,
            pb.position.y + pb.size.y*0.5f
        });
        paused.setPosition({
            float(mColumns * mCellSize)*0.5f,
            float(mRows    * mCellSize)*0.5f - 32.f
        });
        mWindow.draw(paused);

        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        float btnY   = overlayCy + 80.f;
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;
        
        mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);
        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);
        mPauseQuitBtn.setOrigin({ halfW, sz.y * 0.5f });
        mPauseQuitBtn.setPosition({ x2, btnY });
        mPauseQuitTxt.setOrigin({
            mPauseQuitTxt.getLocalBounds().position.x + mPauseQuitTxt.getLocalBounds().size.x * 0.5f,
            mPauseQuitTxt.getLocalBounds().position.y + mPauseQuitTxt.getLocalBounds().size.y * 0.5f
        });
        mPauseQuitTxt.setPosition(mPauseQuitBtn.getPosition());
        mWindow.draw(mPauseQuitBtn);
        mWindow.draw(mPauseQuitTxt);
    }
    mWindow.display();
}

void GameSFML::generatePairedFood()      
{
    std::srand(static_cast<unsigned>(time(nullptr)));
    for (int i = 0; i < 2; ++i)           
    {
        while (true) {
            int x = 1 + std::rand() % (mColumns - 2);
            int y = 1 + std::rand() % (mRows    - 2);

            bool overlapSnake = mSnake.isPartOfSnake(x,y);
            bool overlapOther = (i==1) && (x==mPortals[0].getX() && y==mPortals[0].getY());

            if (!overlapSnake && !overlapOther) {
                mPortals[i] = SnakeBody(x,y);
                break;
            }
        }
    }
}

void GameSFML::generateObstacles() {
    int need = std::max(0, mObstacleCount - static_cast<int>(mObstacles.size()));
    
    for (int i = 0; i < need; ++i) {
        bool valid = false;
        int ox = 0, oy = 0;
        int attempts = 0;
        const int MAX_ATTEMPTS = 100;
        
        while (!valid && attempts < MAX_ATTEMPTS) {
            ox = rand() % mColumns;
            oy = rand() % mRows;
            valid = true;
            attempts++;

            if (mSnake.isPartOfSnake(ox, oy)) {
                valid = false;
                continue;
            }
            if (ox == mFood.getX() && oy == mFood.getY()) {
                valid = false;
                continue;
            }
            for (const auto& obs : mObstacles) {
                if (obs.getX() == ox && obs.getY() == oy) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                for (const auto& tunnel : mScoreTunnels) {
                    if ((tunnel.entrance.getX() == ox && tunnel.entrance.getY() == oy) ||
                        (tunnel.exit.getX() == ox && tunnel.exit.getY() == oy)) {
                        valid = false;
                        break;
                    }
                }
            }
            
            if (mDifficulty == 0) {          
                if (ox == 0 || ox == mColumns-1 ||
                    oy == 0 || oy == mRows-1)   valid = false;
            } else {
                float cx = mColumns / 2.0f;
                float cy = mRows    / 2.0f;
                float R  = std::min(mColumns, mRows) / 2.0f;

                if (currentType == mBorderType::Circle) {
                    float dx = ox + 0.5f - cx;
                    float dy = oy + 0.5f - cy;
                    if (std::sqrt(dx*dx + dy*dy) > R - 0.5f) valid = false;
                }
                else if (currentType == mBorderType::Diamond) {
                    float dx = std::fabs(ox + 0.5f - cx);
                    float dy = std::fabs(oy + 0.5f - cy);
                    if (dx + dy > R - 0.5f) valid = false;
                }
            }

        }
        if (valid) {
            mObstacles.push_back(SnakeBody(ox, oy));
        }
    }
}

void GameSFML::generateScoreObstacles()
{

    int need = std::max(0, mObstacleCount - static_cast<int>(mObstacles.size()));
    
    for (int i = 0; i < need; ++i) {
        int attempts = 0;
        const int MAX_ATTEMPTS = 100;
        
        while (attempts < MAX_ATTEMPTS) {
            int ox = rand() % mColumns;
            int oy = rand() % mRows;
            bool valid = true;
            attempts++;
            if (mSnake.isPartOfSnake(ox, oy)) {
                valid = false;
                if (valid && ox == mFood.getX() && oy == mFood.getY()) {
                    valid = false;
                }
                if (valid) {
                    for (const auto& obs : mObstacles) {
                        if (obs.getX() == ox && obs.getY() == oy) {
                        valid = false;
                        break;
                        }
                    }
                }
            
                if (valid) {
                    mObstacles.push_back(SnakeBody(ox, oy));
                    break;
                }
            }
        }
    }
}

bool GameSFML::hitObstacles()
{
    if (!mObstacles.empty()) { 
        const auto& head = mSnake.getSnake().front();

        for (size_t i = 0; i < mObstacles.size(); ++i) {
            const auto& obs = mObstacles[i];

            if (head.getX() == obs.getX() && head.getY() == obs.getY()) {
                if (mSnake.isAccelerating()) {
                    mPoints = std::max(0, mPoints - 1);
                    if (mSnake.getLength() > 1) {
                        mSnake.shrink();
                    }
                    mObstacles.erase(mObstacles.begin() + i);
                    if (mObstacleCount > 0) {
                        generateObstacles();
                    }
                    mHitEffectTimer = 0.3f;
                    return true; 
                } else {
                    mSnake.decreaseHitPoints();
                    if (mSnake.getHitPoints() <= 0) {
                        playSfx(mBufDeath);
                        mState = GameState::GameOver;
                        openGameOverDialog();
                    } else {
                        playSfx(mBufLoseHP);
                        mInvincibleTimer = 2.0f;
                        mObstacles.erase(mObstacles.begin() + i);
                    }
                    return true; 
                }
            }
        }
    }
    return false;
}

void GameSFML::generateScoreFood()           
{
    std::array<int,4> counts{0,0,0,0};
    for (const auto& f : mScoreFoods)
        for (std::size_t i = 0; i < kFoodQuotas.size(); ++i)
            if (f.color == kFoodQuotas[i].color)
                ++counts[i];
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (std::size_t i = 0; i < kFoodQuotas.size(); ++i)
    {
        while (counts[i] < kFoodQuotas[i].quota)
        {
            int x, y;
            do {
                x = 1 + std::rand() % (mColumns - 2);
                y = 1 + std::rand() % (mRows    - 2);
            } while (mSnake.isPartOfSnake(x,y) || scoreFoodAt(x,y));

            mScoreFoods.push_back({{x,y},
                                    kFoodQuotas[i].color,
                                    kFoodQuotas[i].value,
                                    int(i) 
                                });
            ++counts[i];
        }
    }
}

AppState GameSFML::runScoreMode()
{
    mPoints         = 0;
    mDifficulty     = 0;
    mDelay          = 0.2f;
    mState          = GameState::Playing;
    mInvincibleTimer= 0.f;
    mHitEffectTimer = 0.f;
    loadHighScores(); 
   
    Overlay ready({mColumns * mCellSize, mRows * mCellSize},
              "Ready Go !!!",
              "assets/fonts/Honk.ttf",
              80,
              "assets/music/ready.ogg");

    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false;
    mDifficulty = 0;
    generateScoreFood();
    generateScoreTunnels();
    while (mWindow.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (resumeFlag) {
            acc = 0.f;
            resumeFlag = false;
        }
        processEvents();                   

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();       
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) {
            updateLifeFood(dt);
            if (mInvincibleTimer > 0.f) {
                mInvincibleTimer -= dt;
                mHitEffectTimer = mInvincibleTimer;
            }
            acc += dt;
            while (acc >= effectiveDelay) {
                updateScoreMode();
                acc -= effectiveDelay;
            }
        }

        renderScoreMode();             
    }
    return mOutcome;
}

void GameSFML::updateScoreMode() {
    SnakeBody newHead = mSnake.createNewHead();
    
    if (mHasLifeFood &&
        newHead.getX()==mLifeFood.getX() && newHead.getY()==mLifeFood.getY())
    {
        if (mSnake.getHitPoints() < 3)  mSnake.increaseHitPoints();
        else mPoints += 5;
        mHasLifeFood = false;
    }

    for (auto& tunnel : mScoreTunnels) {
        if (tunnel.isActive && 
            newHead.getX() == tunnel.entrance.getX() && 
            newHead.getY() == tunnel.entrance.getY()) {
            playSfx(mBufTunnel);
            mSnake.teleportToPosition(tunnel.exit.getX(), tunnel.exit.getY());
            mPoints += tunnel.bonusPoints;
            tunnel.isActive = false;
            mDifficulty = mPoints / 20;
            return; 
        }
    }
    bool ateFood = false;
    for (auto it = mScoreFoods.begin(); it != mScoreFoods.end(); ++it) {
        if (newHead.getX() == it->position.getX() && newHead.getY() == it->position.getY()) {
            mPoints += it->value; 
            playSfx(it->value <= 2 ? mBufEat : mBufEatSpec);
            mSnake.senseFood(it->position);
            mScoreFoods.erase(it); 
            generateScoreFood();  
            mSnake.moveFoward(); 
            ateFood = true; 
    
            if (mPoints % 50 == 0) {
                for (auto& tunnel : mScoreTunnels) {
                    tunnel.isActive = true;
                }
                generateScoreTunnels();
            }

            mDifficulty = mPoints / 20; 
            break; 
        }
    }
    
  
    if (!ateFood) {  
        mSnake.senseFood(SnakeBody(-1, -1)); 
        mSnake.moveFoward();
    }
    
    if (mSnake.checkCollision() && mInvincibleTimer <= 0.f) { 
        mSnake.decreaseHitPoints(); 
        if (mSnake.getHitPoints() <= 0) {    
            updateHighScores(mPoints, AppState::ScoreMode);
            playSfx(mBufDeath);
            mState = GameState::GameOver;
            openGameOverDialog();
        } else {  
            playSfx(mBufLoseHP);
            const auto& snakeVec = mSnake.getSnake();
            if (snakeVec.size() > 1) {
                const SnakeBody& prevHead = snakeVec[1];
                mSnake.teleportToPosition(prevHead.getX(), prevHead.getY());
            }
            mSnake.reverseDirection();
            mInvincibleTimer = 1.0f;
        }
    }
    hitObstacles();
}

void GameSFML::renderScoreMode() {
    mWindow.clear();
    drawBackground(); 

    renderBoard(); 
    renderScoreFood(); 
    renderScoreTunnels(); 
    renderLifeFood();
    renderSnake();
    renderObstacles();
    mWindow.draw(mSidebarBorder);
    renderUI();

    if (mState == GameState::Paused) {
        sf::RectangleShape mask({
            float(mColumns * mCellSize),
            float(mRows    * mCellSize)
        });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);

        sf::Text paused(mFont, "PAUSED", 64);
        auto pb = paused.getLocalBounds();
        paused.setOrigin({
            pb.position.x + pb.size.x*0.5f,
            pb.position.y + pb.size.y*0.5f
        });
        paused.setPosition({
            float(mColumns * mCellSize)*0.5f,
            float(mRows    * mCellSize)*0.5f - 32.f
        });
        mWindow.draw(paused);
        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        float btnY   = overlayCy + 80.f;
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;

        mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        mPauseQuitBtn.setOrigin({ halfW, sz.y * 0.5f });
        mPauseQuitBtn.setPosition({ x2, btnY });
        mPauseQuitTxt.setOrigin({
            mPauseQuitTxt.getLocalBounds().position.x + mPauseQuitTxt.getLocalBounds().size.x * 0.5f,
            mPauseQuitTxt.getLocalBounds().position.y + mPauseQuitTxt.getLocalBounds().size.y * 0.5f
        });
        mPauseQuitTxt.setPosition(mPauseQuitBtn.getPosition());
        mWindow.draw(mPauseQuitBtn);
        mWindow.draw(mPauseQuitTxt);
    }
    mWindow.display();
}

void GameSFML::generateScoreTunnels() {
    mScoreTunnels.clear();
    int tunnelCount = 2 + (std::rand() % 2);
    std::vector<std::pair<sf::Color, int>> tunnelTypes = {
        {sf::Color::Cyan, 10},      
        {sf::Color::Magenta, 15},  
        {sf::Color(255, 165, 0), 20}
    };
    
    std::srand(static_cast<unsigned>(time(nullptr)));
    
    for (int i = 0; i < tunnelCount; ++i) {
        ScoreTunnel tunnel;
        bool validTunnel = false;
        int attempts = 0;
        const int MAX_ATTEMPTS = 50;
        
        while (!validTunnel && attempts < MAX_ATTEMPTS) {
            attempts++;
            int entranceX = 3 + std::rand() % (mColumns - 6);
            int entranceY = 3 + std::rand() % (mRows - 6);

            int exitX, exitY;
            do {
                exitX = 4 + std::rand() % (mColumns - 8);  
                exitY = 4 + std::rand() % (mRows - 8);   
            } while (std::abs(exitX - entranceX) < 3 || std::abs(exitY - entranceY) < 3);
            
            tunnel.entrance = SnakeBody(entranceX, entranceY);
            tunnel.exit = SnakeBody(exitX, exitY);
            validTunnel = true;

            if (mSnake.isPartOfSnake(entranceX, entranceY) || 
                mSnake.isPartOfSnake(exitX, exitY)) {
                validTunnel = false;
                continue;
            }

            for (const auto& food : mScoreFoods) {
                if ((food.position.getX() == entranceX && food.position.getY() == entranceY) ||
                    (food.position.getX() == exitX && food.position.getY() == exitY)) {
                    validTunnel = false;
                    break;
                }
            }
            
            if (!validTunnel) continue;
            for (const auto& obs : mObstacles) {
                if ((obs.getX() == entranceX && obs.getY() == entranceY) ||
                    (obs.getX() == exitX && obs.getY() == exitY)) {
                    validTunnel = false;
                    break;
                }
            }
            
            if (!validTunnel) continue;
            for (const auto& existingTunnel : mScoreTunnels) {
                if ((existingTunnel.entrance.getX() == entranceX && existingTunnel.entrance.getY() == entranceY) ||
                    (existingTunnel.entrance.getX() == exitX && existingTunnel.entrance.getY() == exitY) ||
                    (existingTunnel.exit.getX() == entranceX && existingTunnel.exit.getY() == entranceY) ||
                    (existingTunnel.exit.getX() == exitX && existingTunnel.exit.getY() == exitY)) {
                    validTunnel = false;
                    break;
                }
            }
            
            if (validTunnel) {
                int typeIndex = std::rand() % tunnelTypes.size();
                tunnel.color = tunnelTypes[typeIndex].first;
                tunnel.bonusPoints = tunnelTypes[typeIndex].second;
                tunnel.isActive = true;
                
                mScoreTunnels.push_back(tunnel);
            }
        }
    }
}

void GameSFML::renderScoreTunnels()
{
    for (const auto& t : mScoreTunnels)
    {
        if (!t.isActive) continue;

        sf::RectangleShape inBlock({float(mCellSize-1),float(mCellSize-1)});
        inBlock.setFillColor(t.color);
        inBlock.setPosition({float(t.entrance.getX()*mCellSize),
                             float(t.entrance.getY()*mCellSize)});
        mWindow.draw(inBlock);

        sf::CircleShape outDot(float(mCellSize-1)/2.f);
        outDot.setFillColor(t.color);
        outDot.setPosition({float(t.exit.getX()*mCellSize),
                            float(t.exit.getY()*mCellSize)});
        mWindow.draw(outDot);

    }
}


bool GameSFML::isSnakeInCenterArea()
{
    const auto& snakeVec = mSnake.getSnake();
    if (snakeVec.empty()) return false;

    const SnakeBody& head = snakeVec.front();
    float headX = head.getX() + 0.5f;
    float headY = head.getY() + 0.5f;

    float cx = mColumns / 2.0f;
    float cy = mRows    / 2.0f;
    float R  = std::min(mColumns, mRows) / 2.0f;   // 外接半径

    switch (currentType)
    {

        case mBorderType::Circle: {
            float dx = headX - cx;
            float dy = headY - cy;
            return std::sqrt(dx*dx + dy*dy) <= 0.7f * R;
        }
        case mBorderType::Diamond: {
            float dx = std::fabs(headX - cx);
            float dy = std::fabs(headY - cy);
            return (dx + dy) <= 0.7f * R;
        }
    }
    return false;   
}


bool GameSFML::scoreFoodAt(int x, int y) const
{
    for (const auto& f : mScoreFoods)
        if (f.position.getX() == x && f.position.getY() == y)
            return true;
    return false;
}

void GameSFML::generateLifeFood()
{
    int x,y;  int attempts=0;
    do {
        x = 1 + std::rand() % (mColumns-2);
        y = 1 + std::rand() % (mRows   -2);
        ++attempts;
    } while ( (mSnake.isPartOfSnake(x,y)           ||         
               scoreFoodAt(x,y)                    ||        
               (x==mPortals[0].getX()&&y==mPortals[0].getY()) || 
               (x==mPortals[1].getX()&&y==mPortals[1].getY())   
             ) && attempts<100);

    mLifeFood    = SnakeBody(x,y);
    mHasLifeFood = true;
    mLifeElapsed = 0.f;
}

void GameSFML::updateLifeFood(float dt)
{
    if (!mHasLifeFood) {
        if ( (std::rand()%700)==0 && mSnake.getHitPoints()<3 )
            generateLifeFood();
        return;
    }
    mLifeElapsed += dt;
    if (mLifeElapsed > 12.f)          mHasLifeFood = false;
}

void GameSFML::renderLifeFood() {
    if (!mHasLifeFood) return;
    mSprFoodLife.setPosition({
        float(mLifeFood.getX() * mCellSize),
        float(mLifeFood.getY() * mCellSize)
    });
    mWindow.draw(mSprFoodLife);
}

bool GameSFML::pointInPolygon(float px,float py,
                              const std::vector<sf::Vector2f>& poly){
    bool c = false;
    for (std::size_t i=0,j=poly.size()-1; i<poly.size(); j=i++)
    {
        const auto& pi = poly[i];
        const auto& pj = poly[j];
        bool intersect = ((pi.y>py) != (pj.y>py)) &&
            (px < (pj.x-pi.x)*(py-pi.y)/(pj.y-pi.y)+pi.x);
        if(intersect) c = !c;
    }
    return c;
}


void GameSFML::playSfx(const sf::SoundBuffer& buf)
{
    mSounds.erase(std::remove_if(mSounds.begin(), mSounds.end(),
        [](const sf::Sound& s){ return s.getStatus() == sf::Sound::Status::Stopped; }),
        mSounds.end());

    mSounds.emplace_back(buf);   // sf::Sound(const SoundBuffer&)
    mSounds.back().play();
}


void GameSFML::generateTracks()
{
    const int Lmin = 10, Lmax = 15;
    for (size_t idx = 0; idx < mTracks.size(); ++idx) {
        EnergyTrack& tr = mTracks[idx];
        tr.reverse = false;
        tr.progress = 0;
        std::vector<SnakeBody> cells;

        while (true) {
            cells.clear();
            int len  = Lmin + std::rand() % (Lmax - Lmin + 1);
            int bend = 2    + std::rand() % (len - 4);
            int x = 1 + std::rand() % (mColumns - 2);
            int y = 1 + std::rand() % (mRows    - 2);
            if (mSnake.isPartOfSnake(x,y)
             || (x == mFood.getX() && y == mFood.getY())
             || std::any_of(mObstacles.begin(), mObstacles.end(),
                 [&](auto& o){ return o.getX()==x && o.getY()==y; }))
                continue;

            Direction hDir = (std::rand()%2) ? Direction::Left  : Direction::Right;
            Direction vDir = (std::rand()%2) ? Direction::Up    : Direction::Down;

            int cx = x, cy = y;
            bool fail = false;
            for (int i = 0; i < len - bend; ++i) {
                if (cx <= 0 || cx >= static_cast<int>(mColumns)-1) { fail = true; break; }
                cells.emplace_back(cx, cy);
                cx += (hDir == Direction::Left ? -1 : 1);
            }
            if (fail) continue;
            for (int i = 0; i < bend; ++i) {
                if (cy <= 0 || cy >= static_cast<int>(mRows)-1) { fail = true; break; }
                cells.emplace_back(cx, cy);
                cy += (vDir == Direction::Up ? -1 : 1);
            }
            if (fail || static_cast<int>(cells.size()) < Lmin) continue;

            bool overlap = false;
            for (auto& c : cells) {
                if (mSnake.isPartOfSnake(c.getX(), c.getY())
                 || (c.getX()==mFood.getX() && c.getY()==mFood.getY())
                 || (mHasLifeFood && c.getX()==mLifeFood.getX() && c.getY()==mLifeFood.getY())
                 || std::any_of(mObstacles.begin(), mObstacles.end(),
                     [&](auto& o){ return o.getX()==c.getX() && o.getY()==c.getY(); }))
                {
                    overlap = true; break;
                }
                for (size_t j = 0; j < idx; ++j) {
                    for (auto& oc : mTracks[j].cells) {
                        if (oc.getX()==c.getX() && oc.getY()==c.getY()) {
                            overlap = true; break;
                        }
                    }
                    if (overlap) break;
                }
                if (overlap) break;
            }
            if (overlap) continue;

            tr.cells = std::move(cells);
            break;
        }
    }
}

void GameSFML::feedTracks()
{
    const SnakeBody& head = mSnake.getSnake().front();
    for (auto& tr : mTracks) {
        size_t len = tr.cells.size();
        if (len == 0 || tr.progress == len) continue; 

        if (tr.progress == 0) {
            if (head == tr.cells[0]) {
                tr.reverse = false;
                tr.progress = 1;
            } else if (head == tr.cells[len-1]) {
                tr.reverse = true;
                tr.progress = 1;
            }
        }
        else {
            size_t expected = tr.reverse
                ? (len - 1 - tr.progress)
                :  tr.progress;

            if (head == tr.cells[expected]) {
                ++tr.progress;
            } else {
                size_t prev = tr.reverse
                    ? (len - tr.progress)
                    : (tr.progress - 1);
                if (head != tr.cells[prev]) {
                    tr.progress = 0;
                    tr.reverse  = false;
                }
            }
        }

        if (tr.progress == len) {
            mPoints += kTrackBonus;
            generateTracks();
        }
    }

}


void GameSFML::renderTracks()
{
    sf::RectangleShape seg({float(mCellSize-3), float(mCellSize-3)});
    for (const auto& tr : mTracks){
        size_t len = tr.cells.size();
        for (size_t i = 0; i < len; ++i) {
            bool passed;
            if (!tr.reverse) {
                passed = (i < tr.progress);
            } else {
                passed = (i >= len - tr.progress);
            }
            seg.setFillColor(passed
                ? sf::Color(0,255,255,200)  
                : sf::Color(0,255,255, 80)); 
            seg.setPosition({
                float(tr.cells[i].getX() * mCellSize + 1),
                float(tr.cells[i].getY() * mCellSize + 1)
            });
            mWindow.draw(seg);
        }

    }
        
}

AppState GameSFML::runEnergyTrackMode()
{
   
    mPoints       = 0;
    mDifficulty   = 0;
    mDelay        = 0.2f;          
    mState        = GameState::Playing;
    mOutcome      = AppState::StartMenu;   
    mCurrentMode = AppState::EnergyTrackMode;
    mSnake.resetToInitial();
    mObstacles.clear(); 
    mHasLifeFood  = false;
    mLifeElapsed = 0.f;  

    Overlay ready({mColumns * mCellSize, mRows * mCellSize},
                  "Ready Go !!!",
                  "assets/fonts/RussoOne.ttf", 120,
                  "assets/music/ready.wav");
    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    generateFood();          
    generateObstacles();    
    generateTracks();       

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false;

    while (mWindow.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (resumeFlag) {
            acc = 0.f;
            resumeFlag = false;
        }
        processEvents();
        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();
        
        float step = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing)
        {
            updateLifeFood(dt);          
            if (mInvincibleTimer > 0.f) {
                mInvincibleTimer -= dt;
                mHitEffectTimer = mInvincibleTimer;
            }
            acc += dt;
            while (acc >= step)         
            {
                updateEnergyMode();         
                acc -= step;
            }
        }
        if (!mVictoryHandled2 && mPoints >= 25) {
            updateHighScores(mPoints, AppState::EnergyTrackMode);
            mVictoryHandled2 = true;
            mState = GameState::Victory;
            openVictoryDialog(&resumeFlag); 
            openVictoryDialog();
            if (mState != GameState::Playing)  return mOutcome;
        }

        renderEnergyMode();
        if (mState == GameState::GameOver && !mDialog.isOpen())
            break;
    }
    return mOutcome;
}

void GameSFML::renderEnergyMode()
{
    mWindow.clear();
    drawBackground();                
    renderBoard();                 
    renderTracks();              
    renderFood();              
    renderLifeFood();        
    renderSnake();           
    renderObstacles();        

    mWindow.draw(mSidebarBorder);   
    renderUI();                    
    if (mState == GameState::Paused) {
        sf::RectangleShape mask({
            float(mColumns * mCellSize),
            float(mRows    * mCellSize)
        });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);
        sf::Text paused(mFont, "PAUSED", 64);
        auto pb = paused.getLocalBounds();
        paused.setOrigin({
            pb.position.x + pb.size.x*0.5f,
            pb.position.y + pb.size.y*0.5f
        });
        paused.setPosition({
            float(mColumns * mCellSize)*0.5f,
            float(mRows    * mCellSize)*0.5f - 32.f
        });
        mWindow.draw(paused);

        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        float btnY   = overlayCy + 80.f;
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;
        
        mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        mPauseQuitBtn.setOrigin({ halfW, sz.y * 0.5f });
        mPauseQuitBtn.setPosition({ x2, btnY });
        mPauseQuitTxt.setOrigin({
            mPauseQuitTxt.getLocalBounds().position.x + mPauseQuitTxt.getLocalBounds().size.x * 0.5f,
            mPauseQuitTxt.getLocalBounds().position.y + mPauseQuitTxt.getLocalBounds().size.y * 0.5f
        });
        mPauseQuitTxt.setPosition(mPauseQuitBtn.getPosition());
        mWindow.draw(mPauseQuitBtn);
        mWindow.draw(mPauseQuitTxt);
    }
    mWindow.display();
}

void GameSFML::updateEnergyMode()
{  
    feedTracks();
    SnakeBody next = mSnake.createNewHead();
    if (mHasLifeFood && next == mLifeFood) {
        playSfx(mBufLife);                            
        if (mSnake.getHitPoints() < 3)  mSnake.increaseHitPoints();
        else                            mPoints += 5;   
        mHasLifeFood = false;                         
    }

    if (mSnake.moveFoward()) {
        ++mPoints;                               
        playSfx(mBufEat);
        generateFood();                    
        mDifficulty       =  mPoints / 5;
        mObstacleCount    =  5 + mDifficulty;
        generateObstacles();
    }

    if (mSnake.checkCollision()) {
        mSnake.decreaseHitPoints();
        if (mSnake.getHitPoints() <= 0) {
            updateHighScores(mPoints, AppState::EnergyTrackMode);
            playSfx(mBufDeath);
            mState = GameState::GameOver;
            openGameOverDialog();
        } else {
            playSfx(mBufLoseHP);
            const auto& snakeVec = mSnake.getSnake();
            if (snakeVec.size() > 1) {
                const SnakeBody& prevHead = snakeVec[1];
                mSnake.teleportToPosition(prevHead.getX(), prevHead.getY());
            }
            mSnake.reverseDirection();
            mInvincibleTimer = 1.0f;
        }
    }
    hitObstacles();
}
