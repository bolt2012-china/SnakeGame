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
            centroid /= static_cast<float>(shape.getPointCount());   // 真正中心
            sf::Vector2f p = shape.getPoint(i);
            p = { centroid.x + (p.x - centroid.x) * factor,
                centroid.y + (p.y - centroid.y) * factor };
            out.push_back(p / cell);                    // 转格坐标
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
        // 1. 背景矩形
        rect.setSize(size);
        rect.setFillColor(fill);
        rect.setPosition(pos);

        // 2. 文本
        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::Black);

        // 3. 计算真正几何中心（兼容描边/下划线带来的偏移）
        auto bounds = label.getLocalBounds();                       // FloatRect
        const sf::Vector2f center = bounds.position + bounds.size * 0.5f;
        label.setOrigin(center);                                    // ★ 新写法

        // 4. 把文字放到按钮几何中心
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

    load(mBufTurn,     "assets/music/snake_move.wav"); //转向
    load(mBufEat,      "assets/music/eat.wav"); //普通食物
    load(mBufEatSpec,  "assets/music/eat.wav"); //特殊食物
    load(mBufLife,     "assets/music/life.wav"); //回血食物
    load(mBufPort,     "assets/music/potral.wav"); //传送门
    load(mBufTunnel,   "assets/music/portral.wav"); //隧道
    load(mBufLoseHP,   "assets/music/loseHp.wav"); //掉血
    load(mBufDeath,    "assets/music/gameover.wav"); //死亡
    mVictoryMusic.openFromFile("assets/music/win.ogg");
    mVictoryMusic.setLooping(false);
    //载入背景图
    if (!mBgTexture.loadFromFile("assets/nnb.gif")) {
        std::cerr << "Cannot open assets/bg.jpg\n";
        // 失败就静默用纯色背景：留空纹理
    } else {
        // 限制背景图只显示在游戏区域（不覆盖侧边栏）
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
    //加载蛇头
    if (!mHeadTexture.loadFromFile("assets/snakehead.jpg")) {
        std::cerr << "Cannot open assets/head.jpg\n";
    } else {
         mHeadSprite.setTexture(mHeadTexture, /*resetRect=*/true);

        // 缩放到与身体方块同宽高（留 1px 网格缝隙）
        float target = float(mCellSize - 1);
        auto  tex    = mHeadTexture.getSize();
        float scale  = target / std::max(tex.x, tex.y);
        mHeadSprite.setScale({scale, scale});

        //让旋转轴在正中央
        mHeadSprite.setOrigin(
            sf::Vector2f(tex.x * 0.5f, tex.y * 0.5f));
    }

    
    // 加载字体
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
    //侧边栏的框
    float sidebarW = 240.f;
    float sidebarH = static_cast<float>(mRows * mCellSize);

    mSidebarBorder.setSize({sidebarW - 2.f, sidebarH - 2.f});
    mSidebarBorder.setPosition({static_cast<float>(mColumns * mCellSize) + 1.f, 1.f});
    mSidebarBorder.setFillColor(sf::Color(51, 48, 46));
    mSidebarBorder.setOutlineThickness(2.f);
    mSidebarBorder.setOutlineColor(sf::Color::White);

    const float centerX  = columns * cellSize + sidebarW * 0.5f;
    /* ——— POINTS ——— */
    mPointsLabel.setString("POINTS");
    mPointsLabel.setCharacterSize(32);
    mPointsLabel.setFillColor(sf::Color::White);
    auto b = mPointsLabel.getLocalBounds();
    mPointsLabel.setOrigin({ b.position.x + b.size.x * 0.5f,
                         b.position.y + b.size.y * 0.5f });
    mPointsLabel.setPosition({centerX, 55.f});

    /* ——— POINTS VALUE ——— */
    mPointsValue.setString("0");
    mPointsValue.setCharacterSize(52);
    mPointsValue.setFillColor(sf::Color::White);
    b = mPointsValue.getLocalBounds();
    mPointsValue.setOrigin({ b.position.x + b.size.x * 0.5f,
                         b.position.y + b.size.y * 0.5f });
    mPointsValue.setPosition({centerX, 110.f});

    /* ——— Lives ——— */
    mLivesText.setString("Lives: 1");
    mLivesText.setCharacterSize(26);
    mLivesText.setFillColor(sf::Color::White);
    b = mLivesText.getLocalBounds();
    mLivesText.setOrigin({ b.position.x + b.size.x * 0.5f,
                       b.position.y + b.size.y * 0.5f });
    mLivesText.setPosition({centerX, 165.f});

    /* ——— Difficulty ——— */
    mDifficultyText.setString("Difficulty: 0");
    mDifficultyText.setCharacterSize(26);
    mDifficultyText.setFillColor(sf::Color::White);
    b = mDifficultyText.getLocalBounds();
    mDifficultyText.setOrigin({ b.position.x + b.size.x * 0.5f,
                            b.position.y + b.size.y * 0.5f });
    mDifficultyText.setPosition({centerX, 200.f});


    // ——— Pause / Resume 按钮 ———
    const float btnW = 180.f, btnH = 60.f;
    // 上移 20px，让按钮更靠上
    mPauseBtn.setSize({btnW, btnH});
    mPauseBtn.setPosition({ float(mColumns * mCellSize + 35),
                        float(mRows    * mCellSize - 120.f) });
    // 默认白底
    mPauseBtn.setFillColor(sf::Color::White);
    // 黑字，居中
    mPauseTxt.setCharacterSize(32);
    mPauseTxt.setFillColor(sf::Color::Black);
    mPauseTxt.setString("Pause");
    // 先按文本自身中心设 origin
    {
        auto r = mPauseTxt.getLocalBounds();
        mPauseTxt.setOrigin({ r.position.x + r.size.x*0.5f,
                          r.position.y + r.size.y*0.5f });
    }
    // 再放到按钮中心
    mPauseTxt.setPosition(
        mPauseBtn.getPosition() + sf::Vector2f(btnW/2.f, btnH/2.f - 10.f)
    );

    

    // —— 暂停层 Home ——
    mHomeBtn.setSize({220.f, 60.f});
    mHomeBtn.setFillColor(sf::Color(60,60,60,220));

    mHomeTxt.setCharacterSize(30);
    mHomeTxt.setString("Home");
    sf::FloatRect hb = mHomeTxt.getLocalBounds();
    mHomeTxt.setOrigin(sf::Vector2f(hb.size.x*0.5f, hb.size.y*0.5f));

    // —— 暂停层 Quit ——
    mPauseQuitBtn.setSize({220.f, 60.f});
    mPauseQuitBtn.setFillColor(sf::Color(60,60,60,220));

    mPauseQuitTxt.setCharacterSize(30);
    mPauseQuitTxt.setString("Quit");
    sf::FloatRect qb = mPauseQuitTxt.getLocalBounds();
    mPauseQuitTxt.setOrigin(sf::Vector2f(qb.size.x*0.5f, qb.size.y*0.5f));


    // 操作说明
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

    // —— 初始化 High Scores 文本 —— 
    // 标题
    mModeHighScores.fill(0);
    mHighScoresTitleText.setFillColor(sf::Color::White);
    mHighScoresTitleText.setString("High Scores");
    mHighScoresTitleText.setPosition({ textX, baseY - 300.f });   

    // 各模式最高分文本
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
    // 生成初始食物
    generateFood();
    // 生成初始障碍物
    generateObstacles();
    
    /* ◆ 菱形 : 4 点曼哈顿菱形   */
    mDiamondBorder.setPointCount(4);
    mDiamondBorder.setPoint(0,{ float(mColumns*mCellSize/2), 0.f});
    mDiamondBorder.setPoint(1,{ float(mColumns*mCellSize), float(mRows*mCellSize/2)});
    mDiamondBorder.setPoint(2,{ float(mColumns*mCellSize/2), float(mRows*mCellSize)});
    mDiamondBorder.setPoint(3,{ 0.f, float(mRows*mCellSize/2)});
    
    constexpr float PI = 3.14159265f;

    // 初始化随机数生成器
    std::srand(static_cast<unsigned>(time(nullptr)));
    currentType = mBorderType::Circle; 
    mNewBoardActivated = false; // Initialize the flag

    // 统一的目标像素大小（留 1px 网格缝隙）
    float target = float(mCellSize - 1);

    // 1) 加载普通食物图标
    if (!mTexFoodNormal.loadFromFile("assets/icons/fries.png"))
        std::cerr << "Failed to load food_normal.png\n";
    mSprFoodNormal.setTexture(mTexFoodNormal, true);
    {
        auto tsz = mTexFoodNormal.getSize();
        float s = target / std::max(tsz.x, tsz.y);
        mSprFoodNormal.setScale({ s, s });
    }

    // 2) 加载回血食物图标
    if (!mTexFoodLife.loadFromFile("assets/icons/heart.png"))
        std::cerr << "Failed to load food_life.png\n";
    mSprFoodLife.setTexture(mTexFoodLife, true);
    {
        auto tsz = mTexFoodLife.getSize();
        float s = target / std::max(tsz.x, tsz.y);
        mSprFoodLife.setScale({s, s});
    }

    // 3) 加载障碍物图标
    if (!mTexObstacle.loadFromFile("assets/icons/obstacle.png"))
        std::cerr << "Failed to load obstacle.png\n";
    mSprObstacle.setTexture(mTexObstacle, true);
    {
        auto tsz = mTexObstacle.getSize();
        float s = (target - 1) / std::max(tsz.x, tsz.y); // 障碍比食物小 1px
        mSprObstacle.setScale({s, s});
    }

    // 4) 分数模式的多彩食物
    std::array<std::string, kNumScoreTypes> paths = {
        "assets/icons/fries.png",
        "assets/icons/pizza.png",
        "assets/icons/ham.png",
        "assets/icons/burger.png"
    };
    // 先清空／预分配，保证后面 emplace_back 时 vector 有空间
    mSprScoreFoods.clear();
    mSprScoreFoods.reserve(kNumScoreTypes);
    for (int i = 0; i < kNumScoreTypes; ++i) {
        if (!mTexScoreFoods[i].loadFromFile(paths[i]))
            std::cerr << "Failed to load " << paths[i] << '\n';
        // 直接用 Texture+resetRect=true 的构造函数创建 Sprite
        sf::Sprite spr(mTexScoreFoods[i]);
        // 计算并设置缩放
        auto tsz = mTexScoreFoods[i].getSize();
        float s = target / std::max(tsz.x, tsz.y);
        spr.setScale({s, s});
        mSprScoreFoods.emplace_back(std::move(spr));
    }
    // 5) 传送门共同图标（入口/出口同用）
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
              "assets/fonts/RussoOne.ttf",   // 字体
              120,
              "assets/music/ready.wav",
              sf::Color(0,0,0,80));      // 音效

    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false; 

    while (mWindow.isOpen()) {
        float dt = clock.restart().asSeconds(); // 每帧更新
        
        processEvents();                    // 主窗口事件

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();          // 处理对话框事件

        
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { // 只在 Playing 时更新蛇
            acc += dt;
            if (acc >= effectiveDelay) { update(); acc -= effectiveDelay; }
                        // —— 紧接着插入胜利判定 —— 
            if (!mVictoryHandled1 && mPoints >= 15) {
                updateHighScores(mPoints, AppState::LevelMode);
                mState = GameState::Victory;
                mVictoryHandled1 = true;
                openVictoryDialog();
                // 只有 Next/Home/Quit 分支才会让 mState 仍旧是 Victory 并 return
                if (mState != GameState::Playing) {
                return mOutcome;
                }
            }
        }

        if (mInvincibleTimer > 0) {
            mInvincibleTimer -= dt;
            mHitEffectTimer = mInvincibleTimer; // 同步闪烁
        }
        // 无敌状态闪烁效果
        if (mHitEffectTimer > 0) {
            visible = static_cast<int>(mHitEffectTimer * 10) % 2 == 0;
            mHitEffectTimer -= dt;
        }

        render();                           // 一直重绘主窗口（静止画面）
    }
    return mOutcome;
}
// In GameSFML.cpp:

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
        sf::VideoMode(sf::Vector2u{600u, 350u}),                           // ← SFML3.0：VideoMode 构造 
        "Victory",                                         // ← 标题，隐式转换为 sf::String
        sf::Style::Titlebar | sf::Style::Close             // ← Style 标志 
    );
    
    mVictoryMusic.stop();  
    mVictoryMusic.play();   
    
    mFont = sf::Font{"assets/fonts/Marker.ttf"};     
    mMessage.setFont(mFont);
    mMessage.setCharacterSize(52u);
    mMessage.setString("You Win!");
    auto mb = mMessage.getLocalBounds();
    mMessage.setOrigin({ mb.size.x * 0.5f, mb.size.y * 0.5f });  // ← FloatRect.size 
    mMessage.setPosition({ 300.f, 100.f });                      // ← 需 Vector2f 
    mMessage.setFillColor(sf::Color::White);
    
    // 4) 使用已有的 makeButton() 创建两个按钮
    sf::Vector2f btnSize{160.f, 60.f};
    float gapX = (600.f - btnSize.x*2)/3.f;
    float y1   = 180.f;
    float y2   = 260.f;

    // 第一排：Resume / Next
    makeButton(mVictoryResumeBtn, mVictoryResumeTxt, mBaloo2Bold,
               "Resume", {gapX,      y1}, btnSize , sf::Color::White);
    makeButton(mVictoryNextBtn,    mVictoryNextTxt,    mBaloo2Bold,
               "Next",   {gapX*2+btnSize.x, y1}, btnSize, sf::Color::White);

    // 第二排：Home / Quit
    makeButton(mVictoryHomeBtn,    mVictoryHomeTxt,    mBaloo2Bold,
               "Home",   {gapX,      y2}, btnSize, sf::Color::White);
    makeButton(mVictoryQuitBtn,    mVictoryQuitTxt,    mBaloo2Bold,
               "Quit",   {gapX*2+btnSize.x, y2}, btnSize, sf::Color::White);

    mNextHover = false;

    // 5) 事件循环：用 SFML3.0 的 variant 事件接口
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
        
            else if (auto* mm = ev.getIf<sf::Event::MouseMoved>())        // ← getIf<T>() 
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
            // 鼠标按下
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
        // 6) 渲染对话
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

        // 关闭窗口 ───────────────────────────────
        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }

        // ───── 键盘 ─────────────────────────────
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

                // 重启
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

                // 退出
                case sf::Keyboard::Scancode::Escape:
                    mWindow.close();
                    break;

                default:
                    break;
            }

            // 同方向键 → 加速
            if (newDir == currentDir) {
                mSnake.setSpeedMultiplier(1.5f);
                if (!mSnake.isAccelerating())
                    mSnake.setAccelerationEffect(true);
            }
            // 改变方向 → 恢复常速
            else if (newDir != currentDir) {
                mSnake.setSpeedMultiplier(1.0f);
                mSnake.changeDirection(newDir);
                playSfx(mBufTurn);
            }
        }

        // ───── 鼠标移动：悬停高亮 ────────────────
        else if (const auto* mv = event.getIf<sf::Event::MouseMoved>()) {
            sf::Vector2f p{ float(mv->position.x), float(mv->position.y) };

            // 侧边栏 Pause / Resume
            bool hoverNow = mPauseBtn.getGlobalBounds().contains(p);
            if (hoverNow != mPauseHover) {
                mPauseHover = hoverNow;
                mPauseBtn.setFillColor(hoverNow
                    ? sf::Color(220,220,220)  // 悬停更亮
                    : sf::Color::White        // 正常白底
                );
            }

            // ★ Paused 时：中央 Home / Quit
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

        //鼠标点击
        else if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button != sf::Mouse::Button::Left) continue;

            sf::Vector2f p{ float(click->position.x), float(click->position.y) };

            //如果已暂停，先检测中央按钮
            if (mState == GameState::Paused) {
                if (mHomeBtn.getGlobalBounds().contains(p)) {
                    mOutcome = AppState::StartMenu;   //通知上层“我要回菜单”
                    mWindow.close();
                    continue;
                }

                if (mPauseQuitBtn.getGlobalBounds().contains(p)) {
                    std::exit(0);             // “Quit” 彻底退出
                }
            }

            //侧边栏 Pause / Resume
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
            /* ① 难度相关的障碍物递增 */
            ++mObstacleCount;
            generateObstacles();

            /* 切换地图  Circle → Diamond → Star */
            mBorderType prev = currentType;
            int phase = (mPoints / 5 - 1) % 2;         // 0,1,2 循环
            currentType =
                (phase == 0) ? mBorderType::Circle  :
                                mBorderType::Diamond ;

            /* 先隐藏新边框，等蛇再次进入中心才激活 */
            if (currentType != prev)
                mNewBoardActivated = false;
        }
        generateFood();
    }

    /* —— 解锁新地图：先弹提示，再激活 —— */
    if (mDifficulty >= 1 && !mNewBoardActivated && isSnakeInCenterArea()) {

        Overlay unlock({mColumns * mCellSize, mRows * mCellSize},
                   "Unlock new maps!",                 // 标题
                   "assets/fonts/Baloo2-Bold.ttf",     // 任取已存在字体
                   72,                                 // 字号
                   "assets/music/unlock.wav",          
                   sf::Color(0, 0, 0, 140));           // 半透黑

        unlock.play(mWindow, 0.5f, [this]{ renderFrame(); });

        mNewBoardActivated = true;   // 提示结束 → 真正切换到新地图
    }

    
    // Check boundary collision based on board state
    bool shouldCheckBoundary = false;
    if (mDifficulty < 1) {
        // Always check boundary for basic board (difficulty 0)
        shouldCheckBoundary = true;
    } else if (mNewBoardActivated) {
        // Check new board boundary when new board is activated
        shouldCheckBoundary = true;
    }
    // If difficulty >= 1 but new board not activated, don't check boundary
    
    if (shouldCheckBoundary && hitBoundary() && mInvincibleTimer <= 0.f) {      
        mSnake.decreaseHitPoints(); // 减少生命值
        if (mSnake.getHitPoints() <= 0) {
            updateHighScores(mPoints, AppState::LevelMode); //更新得分记录
            playSfx(mBufDeath);
            mState = GameState::GameOver;   // 切换状态
            openGameOverDialog();           // 打开对话框
        }
        else {
            playSfx(mBufLoseHP);
            mInvincibleTimer = 1.0f; // 1秒内无敌，且不复位也不变长
        }
    }
    hitObstacles();
}

void GameSFML::drawBackground()
{
    //绘制背景图辅助函数
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
        
        // —— 按钮：白底黑字，水平并列 —— 
        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        // 放在 PAUSED 下方约 80px
        float btnY   = overlayCy + 80.f;
        // 按钮尺寸 & 间隔取自已有按钮
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
       // 左右中心点
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;

       // 样式：白底、黑字
       mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        // Home 按钮
        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        // Quit 按钮
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
    // 绘制游戏区域边界框

    this->mGameBorder.setPosition(sf::Vector2f(4.f, 4.f)); // 左上角对齐
    this->mGameBorder.setSize({mColumns * mCellSize-12.f, 
                         mRows * mCellSize-12.f});
    //this->mGameBorder.setFillColor(sf::Color::Transparent); // 透明填充
    //this->mGameBorder.setOutlineThickness(2.f); // 边框厚度
    //this->mGameBorder.setOutlineColor(sf::Color::White); // 边框颜色
    //this->mWindow.draw(this->mGameBorder);
    // 可绘制网格（可选）
}

void GameSFML::renderNewBoard() {
    // 绘制新的游戏区域边界框
    
     // 圆形边框
    float radius = std::min(mColumns, mRows) * mCellSize / 2.0f;
    this->mCircleBorder.setRadius(radius);
    this->mCircleBorder.setPosition(sf::Vector2f(mColumns * mCellSize / 2.0f - radius, 
                              mRows * mCellSize / 2.0f - radius));
    this->mCircleBorder.setFillColor(sf::Color::Transparent);
    this->mCircleBorder.setOutlineThickness(2.0f);
    this->mCircleBorder.setOutlineColor(sf::Color::White);
    /* ◆ 菱形边框 */
    mDiamondBorder.setFillColor(sf::Color::Transparent);
    mDiamondBorder.setOutlineThickness(2.f);
    mDiamondBorder.setOutlineColor(sf::Color::White);
    
    
    switch(currentType) {
        case mBorderType::Circle: {
            // ① 外层发光晕（最粗最透明）
            mCircleBorder.setOutlineColor(sf::Color(255,255,255, 80));
            mCircleBorder.setOutlineThickness(10.f);
            mWindow.draw(mCircleBorder);
            // ② 中层强化（半透明）
            mCircleBorder.setOutlineColor(sf::Color(255,255,255, 160));
            mCircleBorder.setOutlineThickness(6.f);
            mWindow.draw(mCircleBorder);
            // ③ 内层主描边（纯白实线）
            mCircleBorder.setOutlineColor(sf::Color::White);
            mCircleBorder.setOutlineThickness(3.f);
            mWindow.draw(mCircleBorder);
        } break;

        case mBorderType::Diamond: {
            // 同样思路：外→中→内
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

    //画蛇头
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

    //画蛇身
    for (std::size_t i = 1; i < parts.size(); ++i) {
        const auto& p = parts[i];
        body.setPosition({ float(p.getX() * mCellSize),
                           float(p.getY() * mCellSize) });
        mWindow.draw(body);
    }
}

void GameSFML::generateFood() {
    // 生成初始食物
    std::srand(static_cast<unsigned>(time(nullptr)));
    while (true) {
        int fx = std::rand() % mColumns;
        int fy = std::rand() % mRows;
        
        // 检查是否在蛇体上
        if (mSnake.isPartOfSnake(fx, fy)) continue;
        
        // 检查是否与障碍物重叠
        bool onObstacle = false;
        for (const auto& obs : mObstacles) {
            if (obs.getX() == fx && obs.getY() == fy) {
                onObstacle = true;
                break;
            }
        }
        if (onObstacle) continue;
        
        // 检查是否在有效区域内且不在边界上
        bool valid = true;
        
        if (mDifficulty == 0) {
            // 矩形边界：确保不在边界上
            if (fx == 0 || fx == mColumns - 1 || fy == 0 || fy == mRows - 1) {
                continue; // 重新生成坐标
            }
        } else {
            // 圆形和三角形边界检查
            if (currentType == mBorderType::Circle) {
                float centerX = mColumns / 2.0f;
                float centerY = mRows / 2.0f;
                float radius = std::min(mColumns, mRows) / 2.0f;
                float dx = fx + 0.5f - centerX;
                float dy = fy + 0.5f - centerY;
                float distance = std::sqrt(dx*dx + dy*dy);
                // 确保食物距离边界至少1个单位
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
    // 入口和出口均使用同一个图标
    for (const auto& f : mPortals) {
        mSprPortal.setPosition({
            float(f.getX() * mCellSize),
            float(f.getY() * mCellSize)
        });
        mSprPortal.setTexture(mTexPortal, /*resetRect=*/true);
        mWindow.draw(mSprPortal);
    }
}

void GameSFML::renderScoreFood() {
    for (auto& food : mScoreFoods) {
        // 假设你在 generateScoreFood 时给 ColoredFood 增加了一个类型索引 `food.typeIdx`
        int idx = food.typeIndex;
        auto& spr = mSprScoreFoods[idx];
        spr.setPosition({ float(food.position.getX() * mCellSize),
                  float(food.position.getY() * mCellSize) });
        mWindow.draw(spr);

    }
}

void GameSFML::renderUI() {
    // 动态更新得分和难度
    mDifficultyText.setString("Difficulty: " + std::to_string(mDifficulty));
    mPointsValue   .setString(std::to_string(mPoints));
    mLivesText     .setString("Lives: "  + std::to_string(mSnake.getHitPoints()));

    // 绘制所有 UI 元素
    mWindow.draw(mUpText);
    mWindow.draw(mDownText);
    mWindow.draw(mLeftText);
    mWindow.draw(mRightText);

    mWindow.draw(mDifficultyText);
    mWindow.draw(mPointsLabel);
    mWindow.draw(mPointsValue);
    mWindow.draw(mLivesText);

    // 绘制四模式最高分
    mWindow.draw(mHighScoresTitleText);
    for (auto& txt : mModeHighScoreTexts)
        mWindow.draw(txt);
    
    // 更新按钮文字
    mPauseTxt.setString(mState == GameState::Paused ? "Resume" : "Pause");
    sf::FloatRect r = mPauseTxt.getLocalBounds();
    mPauseTxt.setOrigin(sf::Vector2f(r.size.x * 0.5f,
                                    r.size.y * 0.5f));

    // 绘制按钮
    mWindow.draw(mPauseBtn);
    mWindow.draw(mPauseTxt);

}

void GameSFML::openGameOverDialog()
{
    if (mDialog.isOpen()) return;               // 已打开就别重复建

    // 1) 放大窗口到 
    mDialog.create(
        sf::VideoMode({600u, 350u}), "Game Over",
        sf::Style::Titlebar | sf::Style::Close);
    // —— 2) 提示文字居中（Y 轴上移到窗口 25% 高度） ——
    sf::FloatRect tb = mGameOverMsg.getLocalBounds();
    mGameOverMsg.setOrigin({ tb.size.x * 0.5f, tb.size.y * 0.5f });

    // 直接将 Vector2u 转为 Vector2f
    sf::Vector2f winSize = static_cast<sf::Vector2f>(mDialog.getSize());
    mGameOverMsg.setPosition({ winSize.x * 0.5f, winSize.y * 0.25f });

    // —— 3) 按钮尺寸保持 120×60，均匀分布 —— 
    const sf::Vector2f btnSize{120.f, 60.f};

    // 留出四段间隙（左右各一份 + 两个按钮间各一份）
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
        // evt 是 std::optional<sf::Event>，箭头运算符直接访问内部对象
        if (evt->is<sf::Event::Closed>()) {        // 关闭按钮
            mWindow.close();
            mDialog.close();
            return;
        }
        else if (const auto* mouse = evt->getIf<sf::Event::MouseButtonPressed>()) {
            // 鼠标点击坐标
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

    // 绘制对话框
    mDialog.clear(sf::Color(139,  0,  0));    // 暗红底
    mDialog.draw(mGameOverMsg);               // 白字提示
    // 白底黑字由 makeButton 已经设置
    mDialog.draw(mRestartBtn);    mDialog.draw(mRestartTxt);
    mDialog.draw(mHomeDBtn);      mDialog.draw(mHomeDTxt);
    mDialog.draw(mQuitBtn);       mDialog.draw(mQuitTxt);
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
    currentType = mBorderType::Circle;  // 总是从圆形开始
    mNewBoardActivated = false; // Reset the flag on restart
    mSnake.resetHitPoints(); // 重启时补满生命值

}

void GameSFML::loadHighScores()
{
    std::ifstream in("highscores.txt");
    if (!in) return;  // 文件不存在就跳过

    for (int i = 0; i < kNumModes && in; ++i)
        in >> mModeHighScores[i];
    // 更新显示
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
        // 识别模式索引
    int idx = 0;
    switch (mode) {
        case AppState::LevelMode:        idx = LevelModeIdx;        break;
        case AppState::EnergyTrackMode:  idx = EnergyTrackModeIdx;  break;
        case AppState::PortalMode:       idx = PortalModeIdx;       break;
        case AppState::ScoreMode:        idx = PointsModeIdx;       break;
        default: return;
    }
    // 只在超过历史时更新
    if (score > mModeHighScores[idx]) {
        mModeHighScores[idx] = score;
        saveHighScores();
        // 更新显示文本
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

    if (mDifficulty == 0) {               // 矩形地图：在格子 0 和 最后一格 视为边界
        const auto& snakeCells = mSnake.getSnake();
        if (!snakeCells.empty()) {
            const SnakeBody& head = snakeCells.front();
            int x = head.getX();
            int y = head.getY();
            // 碰撞矩形边框：格子0 或 格子(mColumns-1/mRows-1) 都算死
            bool wallCollision = (x < 0 || x >= static_cast<int>(mColumns) ||
                                y < 0 || y >= static_cast<int>(mRows));
            // 撞到自己也算死
            bool selfCollision = mSnake.hitSelf();
            return wallCollision || selfCollision;
        }
        return false;
    }

    /* —— 新边框 —— */
    const float cx = mColumns * mCellSize * 0.5f;
    const float cy = mRows    * mCellSize * 0.5f;
    const float R  = std::min(mColumns, mRows) * mCellSize * 0.5f;
    const float MARGIN = mCellSize * 0.5f;        // 半格余量
    switch (currentType)
    {
        case mBorderType::Circle: {
            float dist = std::hypot(headX - cx, headY - cy);
            return dist >= R + MARGIN;
        }
        case mBorderType::Diamond: {
            float manhattan = std::fabs(headX - cx) + std::fabs(headY - cy);
            return manhattan >= R + MARGIN;        // 原来用 R，提早了一格
        }
       
    }
    return false;      // 不会走到这里
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
              "assets/fonts/RussoOne.ttf",   // 字体
              80,
              "assets/music/ready.wav");      // 音效

    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false;
    
    mDifficulty = 0;
    
    // 生成初始的传送门食物
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
            // 只在 Playing 时更新蛇
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
        // ——— 胜利判定 ———
        if ( !mVictoryHandled3 && mPoints >= 35) {
            updateHighScores(mPoints, AppState::PortalMode);
            mVictoryHandled3 = true;
            mState = GameState::Victory;
            openVictoryDialog(&resumeFlag);
            if (mState != GameState::Playing)  return mOutcome;
        }
        
        renderPortalMode();                 // 使用传送门模式专用渲染
        
    }
    return mOutcome;
}

void GameSFML::updatePortalMode() {
    // Check if snake touched portal food first
    SnakeBody next = mSnake.createNewHead();          // 预测下一格
    int hit = -1;                                     // 0 表示撞到 A，1 表示 B
    if (next == mPortals[0]) hit = 0;
    else if (next == mPortals[1]) hit = 1;

    if (mHasLifeFood && next == mLifeFood) {
        playSfx(mBufLife);
        if (mSnake.getHitPoints() < 3)  mSnake.increaseHitPoints();
        else mPoints += 5;
        mHasLifeFood = false;
        // 不增长身长，直接继续下面的逻辑
    }


    // ↓ 这里还是“撞到 A 或 B 就进入瞬移流程”
    if (hit != -1) {
        playSfx(mBufPort);
        /*—— 先吃掉 A/B ——*/
        mSnake.senseFood(mPortals[hit]);
        mSnake.moveFoward();           // ⬆︎ 已经会增长 1
        ++mPoints;

        /*—— 传送到另一枚 ——*/
        int other = 1 - hit;
        mSnake.teleportToPosition(
            mPortals[other].getX(), mPortals[other].getY());

        /*—— 吃掉 B/A，再长 1 ——*/
        mSnake.grow();                 // ← 新增：原地再加一节
        ++mPoints;

        generatePairedFood();          // 刷新下一对
        return;                        // 本帧结束
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
        } else {   // 还有命：复活并短暂无敌
            playSfx(mBufLoseHP);
            // bounce: 恢复到撞墙前的格子 → 反向 → 1s 无敌
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
    // —— 暂停蒙版 —— 
    if (mState == GameState::Paused) {
        // 半透明遮罩
        sf::RectangleShape mask({
            float(mColumns * mCellSize),
            float(mRows    * mCellSize)
        });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);

        // PAUSED 文字
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

        // —— 按钮：白底黑字，水平并列 —— 
        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        // 放在 PAUSED 下方约 80px
        float btnY   = overlayCy + 80.f;
        // 按钮尺寸 & 间隔取自已有按钮
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
       // 左右中心点
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;

       // 样式：白底、黑字
       mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        // Home 按钮
        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        // Quit 按钮
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

void GameSFML::generatePairedFood()          // ← 新名字
{
    std::srand(static_cast<unsigned>(time(nullptr)));
    for (int i = 0; i < 2; ++i)              // 连续生成 A、B
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
    // 计算需要生成的障碍物数量
    int need = std::max(0, mObstacleCount - static_cast<int>(mObstacles.size()));
    
    for (int i = 0; i < need; ++i) {
        bool valid = false;
        int ox = 0, oy = 0;
        int attempts = 0;
        const int MAX_ATTEMPTS = 100; // 防止无限循环
        
        while (!valid && attempts < MAX_ATTEMPTS) {
            ox = rand() % mColumns;
            oy = rand() % mRows;
            valid = true;
            attempts++;
            
            // 检查是否与蛇重叠
            if (mSnake.isPartOfSnake(ox, oy)) {
                valid = false;
                continue;
            }
            
            // 检查是否与食物重叠
            if (ox == mFood.getX() && oy == mFood.getY()) {
                valid = false;
                continue;
            }
            
            // 检查是否与其他障碍物重叠
            for (const auto& obs : mObstacles) {
                if (obs.getX() == ox && obs.getY() == oy) {
                    valid = false;
                    break;
                }
            }
            
            // 检查是否与隧道重叠
            if (valid) {
                for (const auto& tunnel : mScoreTunnels) {
                    if ((tunnel.entrance.getX() == ox && tunnel.entrance.getY() == oy) ||
                        (tunnel.exit.getX() == ox && tunnel.exit.getY() == oy)) {
                        valid = false;
                        break;
                    }
                }
            }
            
                        // —— 边界合法性检测 ——  
            if (mDifficulty == 0) {            // 矩形地图
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
    // 计算需要生成的障碍物数量 - 简化版本用于基础游戏板
    int need = std::max(0, mObstacleCount - static_cast<int>(mObstacles.size()));
    
    for (int i = 0; i < need; ++i) {
        int attempts = 0;
        const int MAX_ATTEMPTS = 100; // 防止无限循环
        
        while (attempts < MAX_ATTEMPTS) {
            int ox = rand() % mColumns;
            int oy = rand() % mRows;
            bool valid = true;
            attempts++;
            
            // 检查是否与蛇重叠
            if (mSnake.isPartOfSnake(ox, oy)) {
                valid = false;
            }
            
            // 检查是否与食物重叠
            if (valid && ox == mFood.getX() && oy == mFood.getY()) {
                valid = false;
            }
        
            
            // 检查是否与其他障碍物重叠
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

bool GameSFML::hitObstacles()
{
    // 障碍物碰撞检测
    if (!mObstacles.empty()) { // 确保容器不为空
        const auto& head = mSnake.getSnake().front();

        for (size_t i = 0; i < mObstacles.size(); ++i) {
            const auto& obs = mObstacles[i];

            if (head.getX() == obs.getX() && head.getY() == obs.getY()) {
                if (mSnake.isAccelerating()) {
                    // 加速状态：穿透障碍物
                    mPoints = std::max(0, mPoints - 1);

                    if (mSnake.getLength() > 1) {
                        mSnake.shrink();
                    }

                    // 安全移除障碍物
                    mObstacles.erase(mObstacles.begin() + i);

                    // 补充障碍物（确保不越界）
                    if (mObstacleCount > 0) {
                        generateObstacles();
                    }

                    // 视觉反馈
                    mHitEffectTimer = 0.3f;
                    return true; // 删除后直接返回，避免越界
                } else {
                    // 正常状态：碰撞掉血
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
                    return true; // 处理后直接返回
                }
            }
        }
    }
    return false;
}

void GameSFML::generateScoreFood()           
{
    /* 1️⃣ 统计当前各颜色数量 */
    std::array<int,4> counts{0,0,0,0};
    for (const auto& f : mScoreFoods)
        for (std::size_t i = 0; i < kFoodQuotas.size(); ++i)
            if (f.color == kFoodQuotas[i].color)
                ++counts[i];

    /* 2️⃣ 逐色补足配额 */
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
              "assets/fonts/Honk.ttf",   // 字体
              80,
              "assets/music/ready.ogg");      // 音效

    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    sf::Clock clock;
    float acc = 0.f;
    bool resumeFlag = false;
    
    // 在分数模式中强制使用基础边界（难度0）
    mDifficulty = 0;
    
    // 生成初始的多彩食物
    generateScoreFood();
    
    // 生成分数隧道
    generateScoreTunnels();
    
    while (mWindow.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (resumeFlag) {
            acc = 0.f;
            resumeFlag = false;
        }
        processEvents();                    // 主窗口事件

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();          // 处理对话框事件

        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { // 只在 Playing 时更新蛇
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

        renderScoreMode();                  // 使用分数模式专用渲染
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
        // 不让蛇增长，因此不调用 senseFood()
    }

    // 检查是否进入隧道入口
    for (auto& tunnel : mScoreTunnels) {
        if (tunnel.isActive && 
            newHead.getX() == tunnel.entrance.getX() && 
            newHead.getY() == tunnel.entrance.getY()) {
            
            playSfx(mBufTunnel);
            // 蛇进入隧道，传送到出口
            mSnake.teleportToPosition(tunnel.exit.getX(), tunnel.exit.getY());
            mPoints += tunnel.bonusPoints; // 获得奖励分数
            
            // 隧道使用后暂时失效
            tunnel.isActive = false;
            
            // 难度逐渐增加
            mDifficulty = mPoints / 20;
            return; // 使用隧道后直接返回
        }
    }
    
    // 检查是否吃到彩色食物（在移动之前检查）
    bool ateFood = false;
    for (auto it = mScoreFoods.begin(); it != mScoreFoods.end(); ++it) {
        if (newHead.getX() == it->position.getX() && newHead.getY() == it->position.getY()) {
            mPoints += it->value;  // 根据食物颜色加分
            playSfx(it->value <= 2 ? mBufEat : mBufEatSpec);
            
            // 在移除食物之前，先让蛇感知到食物位置
            mSnake.senseFood(it->position);
            
            mScoreFoods.erase(it); // 移除被吃掉的食物

            generateScoreFood();  
            
            // 现在移动蛇，由于已经感知到食物，它会增长
            mSnake.moveFoward(); // 移动蛇并增长
            
            ateFood = true; //立即补足各颜色配额
            
            
            // 每获得一定分数后重新激活隧道
            if (mPoints % 50 == 0) {
                for (auto& tunnel : mScoreTunnels) {
                    tunnel.isActive = true;
                }
                // 也可以重新生成隧道位置
                generateScoreTunnels();
            }
            
            mDifficulty = mPoints / 20; // 分数模式难度增加更慢
            
            break; // 吃到食物后跳出循环
        }
    }
    
    // 如果没有吃到食物，正常移动（不增长）
    if (!ateFood) {
        // 清除感知到的食物，确保moveFoward()不会让蛇增长
        mSnake.senseFood(SnakeBody(-1, -1)); // 设置为无效位置
        mSnake.moveFoward(); // 现在只会移动，不会增长
    }
    
    // 碰撞检测 - 分数模式使用基础矩形边界
    if (mSnake.checkCollision() && mInvincibleTimer <= 0.f) { 
        mSnake.decreaseHitPoints(); 
        if (mSnake.getHitPoints() <= 0) {    
            updateHighScores(mPoints, AppState::ScoreMode);
            playSfx(mBufDeath);
            mState = GameState::GameOver;
            openGameOverDialog();
        } else {  
            // 还有命：复活并短暂无敌
            playSfx(mBufLoseHP);
            // bounce: 恢复到撞墙前的格子 → 反向 → 1s 无敌
            const auto& snakeVec = mSnake.getSnake();
            if (snakeVec.size() > 1) {
                const SnakeBody& prevHead = snakeVec[1];
                mSnake.teleportToPosition(prevHead.getX(), prevHead.getY());
            }
            mSnake.reverseDirection();
            mInvincibleTimer = 1.0f;
        }
    }
    
    // 障碍物碰撞检测
    hitObstacles();
}

void GameSFML::renderScoreMode() {
    mWindow.clear();
    drawBackground(); 
    
    // 总是使用基础矩形边界在分数模式中
    renderBoard(); 
    
    renderScoreFood();  // 渲染彩色食物
    renderScoreTunnels(); // 渲染分数隧道
    renderLifeFood();
    renderSnake();
    renderObstacles();
    mWindow.draw(mSidebarBorder);
    renderUI();
    // —— 暂停蒙版 —— 
    if (mState == GameState::Paused) {
        // 半透明遮罩
        sf::RectangleShape mask({
            float(mColumns * mCellSize),
            float(mRows    * mCellSize)
        });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);

        // PAUSED 文字
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

        // —— 按钮：白底黑字，水平并列 —— 
        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        // 放在 PAUSED 下方约 80px
        float btnY   = overlayCy + 80.f;
        // 按钮尺寸 & 间隔取自已有按钮
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
       // 左右中心点
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;

       // 样式：白底、黑字
       mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        // Home 按钮
        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        // Quit 按钮
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
    // 清空现有隧道
    mScoreTunnels.clear();
    
    // 生成2-3个隧道
    int tunnelCount = 2 + (std::rand() % 2);
    
    // 定义隧道颜色和奖励分数
    std::vector<std::pair<sf::Color, int>> tunnelTypes = {
        {sf::Color::Cyan, 10},      // 青色隧道：10分
        {sf::Color::Magenta, 15},   // 品红隧道：15分
        {sf::Color(255, 165, 0), 20} // 橙色隧道：20分
    };
    
    std::srand(static_cast<unsigned>(time(nullptr)));
    
    for (int i = 0; i < tunnelCount; ++i) {
        ScoreTunnel tunnel;
        bool validTunnel = false;
        int attempts = 0;
        const int MAX_ATTEMPTS = 50;
        
        while (!validTunnel && attempts < MAX_ATTEMPTS) {
            attempts++;
            
            // 随机生成入口位置（避免边界）
            int entranceX = 3 + std::rand() % (mColumns - 6);
            int entranceY = 3 + std::rand() % (mRows - 6);
            
            // 随机生成出口位置，确保与入口有一定距离，且远离边界
            int exitX, exitY;
            do {
                exitX = 4 + std::rand() % (mColumns - 8);  // 确保出口距离边界至少4个单位
                exitY = 4 + std::rand() % (mRows - 8);     // 确保出口距离边界至少4个单位
            } while (std::abs(exitX - entranceX) < 3 || std::abs(exitY - entranceY) < 3);
            
            tunnel.entrance = SnakeBody(entranceX, entranceY);
            tunnel.exit = SnakeBody(exitX, exitY);
            
            // 检查位置是否有效（不与蛇身、食物、障碍物、其他隧道重叠）
            validTunnel = true;
            
            // 检查与蛇身的重叠
            if (mSnake.isPartOfSnake(entranceX, entranceY) || 
                mSnake.isPartOfSnake(exitX, exitY)) {
                validTunnel = false;
                continue;
            }
            
            // 检查与食物的重叠
            for (const auto& food : mScoreFoods) {
                if ((food.position.getX() == entranceX && food.position.getY() == entranceY) ||
                    (food.position.getX() == exitX && food.position.getY() == exitY)) {
                    validTunnel = false;
                    break;
                }
            }
            
            if (!validTunnel) continue;
            
            // 检查与障碍物的重叠
            for (const auto& obs : mObstacles) {
                if ((obs.getX() == entranceX && obs.getY() == entranceY) ||
                    (obs.getX() == exitX && obs.getY() == exitY)) {
                    validTunnel = false;
                    break;
                }
            }
            
            if (!validTunnel) continue;
            
            // 检查与其他隧道的重叠
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
                // 随机选择隧道类型
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

        /* ③-1 入口：方块 */
        sf::RectangleShape inBlock({float(mCellSize-1),float(mCellSize-1)});
        inBlock.setFillColor(t.color);
        inBlock.setPosition({float(t.entrance.getX()*mCellSize),
                             float(t.entrance.getY()*mCellSize)});
        mWindow.draw(inBlock);

        /* ③-2 出口：圆点 */
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
        /* ① 圆形：与原逻辑一致 */
        case mBorderType::Circle: {
            float dx = headX - cx;
            float dy = headY - cy;
            return std::sqrt(dx*dx + dy*dy) <= 0.7f * R;
        }

        /* ② 菱形（|x-cx|+|y-cy| 判定） */
        case mBorderType::Diamond: {
            float dx = std::fabs(headX - cx);
            float dy = std::fabs(headY - cy);
            return (dx + dy) <= 0.7f * R;
        }

        
    }
    return false;   // 不会到这
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
    } while ( (mSnake.isPartOfSnake(x,y)           ||           // 蛇身
               scoreFoodAt(x,y)                    ||           // 彩食
               (x==mPortals[0].getX()&&y==mPortals[0].getY()) || // 传送食物
               (x==mPortals[1].getX()&&y==mPortals[1].getY())    // …
             ) && attempts<100);

    mLifeFood    = SnakeBody(x,y);
    mHasLifeFood = true;
    mLifeElapsed = 0.f;
}

void GameSFML::updateLifeFood(float dt)
{
    /* 若未存在 —— 有 1/700 概率刷，且命未满 3 */
    if (!mHasLifeFood) {
        if ( (std::rand()%700)==0 && mSnake.getHitPoints()<3 )
            generateLifeFood();
        return;
    }

    /* 已存在：10 s 后闪烁，12 s 自动消失 */
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


/* ───── 通用多边形包含测试（射线奇偶）───── */
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
    // 清理已经停止的声道
    mSounds.erase(std::remove_if(mSounds.begin(), mSounds.end(),
        [](const sf::Sound& s){ return s.getStatus() == sf::Sound::Status::Stopped; }),
        mSounds.end());

    // 新建声道并播放
    mSounds.emplace_back(buf);   // sf::Sound(const SoundBuffer&)
    mSounds.back().play();
}


void GameSFML::generateTracks()
{
    const int Lmin = 10, Lmax = 15;
    // 依次生成每条轨道，确保不与之前的轨道、自身蛇身、食物、障碍重叠
    for (size_t idx = 0; idx < mTracks.size(); ++idx) {
        EnergyTrack& tr = mTracks[idx];
        tr.reverse = false;
        tr.progress = 0;
        std::vector<SnakeBody> cells;

        while (true) {
            cells.clear();
            int len  = Lmin + std::rand() % (Lmax - Lmin + 1);
            int bend = 2    + std::rand() % (len - 4);
            // 随机起点（避开边界、蛇身、当前食物、障碍）
            int x = 1 + std::rand() % (mColumns - 2);
            int y = 1 + std::rand() % (mRows    - 2);
            if (mSnake.isPartOfSnake(x,y)
             || (x == mFood.getX() && y == mFood.getY())
             || std::any_of(mObstacles.begin(), mObstacles.end(),
                 [&](auto& o){ return o.getX()==x && o.getY()==y; }))
                continue;

           // 随机水平 & 垂直方向
            Direction hDir = (std::rand()%2) ? Direction::Left  : Direction::Right;
            Direction vDir = (std::rand()%2) ? Direction::Up    : Direction::Down;

            // 水平段
            int cx = x, cy = y;
            bool fail = false;
            for (int i = 0; i < len - bend; ++i) {
                if (cx <= 0 || cx >= static_cast<int>(mColumns)-1) { fail = true; break; }
                cells.emplace_back(cx, cy);
                cx += (hDir == Direction::Left ? -1 : 1);
            }
            if (fail) continue;
            // 垂直段
            for (int i = 0; i < bend; ++i) {
                if (cy <= 0 || cy >= static_cast<int>(mRows)-1) { fail = true; break; }
                cells.emplace_back(cx, cy);
                cy += (vDir == Direction::Up ? -1 : 1);
            }
            if (fail || static_cast<int>(cells.size()) < Lmin) continue;

            // 最终检查：各格不重叠蛇身、食物、障碍、已生成轨道，以及生命食物
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
                // 已生成轨道
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

            // 通过所有检查，正式写入
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
        if (len == 0 || tr.progress == len) continue; // 空轨或已完成，跳过

        // —— 起点：任意一端都可开始 —— 
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
            // 计算下一步应踩的格子索引
            size_t expected = tr.reverse
                ? (len - 1 - tr.progress)
                :  tr.progress;

            if (head == tr.cells[expected]) {
                ++tr.progress;
            } else {
                // 如果没站在“上一步”格子，则视为脱轨
                size_t prev = tr.reverse
                    ? (len - tr.progress)
                    : (tr.progress - 1);
                if (head != tr.cells[prev]) {
                    tr.progress = 0;
                    tr.reverse  = false;
                }
            }
        }

        // —— 完成轨道奖励并重置 —— 
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
                // 正向：从 cells[0] 向后高亮
                passed = (i < tr.progress);
            } else {
                // 反向：从 cells[len-1] 向前高亮
                passed = (i >= len - tr.progress);
            }
            seg.setFillColor(passed
                ? sf::Color(0,255,255,200)  // 已走高亮
                : sf::Color(0,255,255, 80)); // 未走半透
            seg.setPosition({
                float(tr.cells[i].getX() * mCellSize + 1),
                float(tr.cells[i].getY() * mCellSize + 1)
            });
            mWindow.draw(seg);
        }

    }
        
}

/* ────────────────────────────────────────────────
 *  Energy-Track Mode 入口
 *  ------------------------------------------------
 *  结构对齐其它 runXXXMode()：
 *    1. 统一重置 → 2. “Ready Go” → 3. 生成元素
 *    4. 主循环 { 事件 → 更新(按节拍) → 渲染 } → 5. 返回
 * ────────────────────────────────────────────────*/
AppState GameSFML::runEnergyTrackMode()
{
    /* 1️⃣ 统一重置本局运行时状态（保持和其它模式一致） */
    // 如果已经有公共函数就直接调用 resetRuntimeState();
    mPoints       = 0;
    mDifficulty   = 0;
    mDelay        = 0.2f;           // 见全局常量
    mState        = GameState::Playing;
    mOutcome      = AppState::StartMenu;    // Game-Over 时默认回主菜单
    mCurrentMode = AppState::EnergyTrackMode;
    mSnake.resetToInitial();
    mObstacles.clear(); 
    mHasLifeFood  = false;
    mLifeElapsed = 0.f;  

    /* 2️⃣ “Ready Go !!!” 过场动画 */
    Overlay ready({mColumns * mCellSize, mRows * mCellSize},
                  "Ready Go !!!",
                  "assets/fonts/RussoOne.ttf", 120,
                  "assets/music/ready.wav");
    ready.play(mWindow, 1.f, [this]{ renderFrame(); });

    generateFood();          // 普通食物
    generateObstacles();     // 初始障碍
    generateTracks();        // 两条能量轨道（★ 新增）

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
            updateLifeFood(dt);              // 回血食物计时逻辑
            if (mInvincibleTimer > 0.f) {
                mInvincibleTimer -= dt;
                mHitEffectTimer = mInvincibleTimer;
            }
            acc += dt;
            while (acc >= step)              // 防掉帧漏格
            {
                updateEnergyMode();          // ★ 逻辑核心
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


        /* 4-c. 渲染整帧 */
        renderEnergyMode();

        /* 4-d. Game Over 且对话框已关闭 → 退出循环 */
        if (mState == GameState::GameOver && !mDialog.isOpen())
            break;
    }

    /* 5️⃣ 返回给主程序的下一状态（与其它 run 函数一致） */
    return mOutcome;
}

/* ────────────────────────────────────────
 *  Energy-Track 玩法：一帧完整渲染
 * ────────────────────────────────────────*/
void GameSFML::renderEnergyMode()
{
    mWindow.clear();
    drawBackground();                // 背景（可选）

    renderBoard();                   // 总是矩形边框
    renderTracks();                  // ★ 两条能量轨道
    renderFood();                    // 普通食物
    renderLifeFood();                // 心形回血食物
    renderSnake();                   // 蛇
    renderObstacles();               // 障碍物

    mWindow.draw(mSidebarBorder);    // 右侧 UI 背板
    renderUI();                      // 分数 / 难度 / HP 等
    // —— 暂停蒙版 —— 
    if (mState == GameState::Paused) {
        // 半透明遮罩
        sf::RectangleShape mask({
            float(mColumns * mCellSize),
            float(mRows    * mCellSize)
        });
        mask.setFillColor(sf::Color(0,0,0,120));
        mWindow.draw(mask);

        // PAUSED 文字
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

        // —— 按钮：白底黑字，水平并列 —— 
        float overlayCx = float(mColumns * mCellSize) * 0.5f;
        float overlayCy = float(mRows    * mCellSize) * 0.5f;
        // 放在 PAUSED 下方约 80px
        float btnY   = overlayCy + 80.f;
        // 按钮尺寸 & 间隔取自已有按钮
        sf::Vector2f sz  = mHomeBtn.getSize();    
        float halfW     = sz.x * 0.5f;
        float spacing   = 20.f;
       // 左右中心点
        float x1 = overlayCx - halfW - spacing * 0.5f;
        float x2 = overlayCx + halfW + spacing * 0.5f;

       // 样式：白底、黑字
       mHomeBtn      .setFillColor(sf::Color::White);
        mPauseQuitBtn.setFillColor(sf::Color::White);
        mHomeTxt      .setFillColor(sf::Color::Black);
        mPauseQuitTxt .setFillColor(sf::Color::Black);

        // Home 按钮
        mHomeBtn.setOrigin({ halfW, sz.y * 0.5f });
        mHomeBtn.setPosition({ x1, btnY });
        mHomeTxt.setOrigin({
            mHomeTxt.getLocalBounds().position.x + mHomeTxt.getLocalBounds().size.x * 0.5f,
            mHomeTxt.getLocalBounds().position.y + mHomeTxt.getLocalBounds().size.y * 0.5f
        });
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);

        // Quit 按钮
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


/* ────────────────────────────────────────
 *  Energy-Track 玩法：逻辑主循环
 * ────────────────────────────────────────*/
void GameSFML::updateEnergyMode()
{
    /* 1️⃣ 先让能量轨道组件吃一口 —— 可能结算 +6 分并补轨道 */
    feedTracks();

    /* 2️⃣ 预判下一格：先处理回血食物，避免被蛇身覆盖 */
    SnakeBody next = mSnake.createNewHead();
    if (mHasLifeFood && next == mLifeFood) {
        playSfx(mBufLife);                               // ★ 播音效
        if (mSnake.getHitPoints() < 3)  mSnake.increaseHitPoints();
        else                            mPoints += 5;    // 满血时转 5 分
        mHasLifeFood = false;                            // 移除回血食物
    }

    /* 3️⃣ 正常前进；返回 true 代表吃到了普通食物 */
    if (mSnake.moveFoward()) {
        ++mPoints;                                       // 普通食物 +1
        playSfx(mBufEat);
        generateFood();                                  // 刷新普通食物

        /* 难度 = ⌊分数 / 5⌋；每 5 分加 1 个障碍 */
        mDifficulty       =  mPoints / 5;
        mObstacleCount    =  5 + mDifficulty;
        generateObstacles();
    }

    /* 4️⃣ 碰撞检测 —— 仍采用矩形基本边界 */
    if (mSnake.checkCollision()) {
        mSnake.decreaseHitPoints();
        if (mSnake.getHitPoints() <= 0) {
            updateHighScores(mPoints, AppState::EnergyTrackMode);
            playSfx(mBufDeath);
            mState = GameState::GameOver;
            openGameOverDialog();
        } else {
            playSfx(mBufLoseHP);


            // bounce: 恢复到撞墙前的格子 → 反向 → 1s 无敌
            const auto& snakeVec = mSnake.getSnake();
            if (snakeVec.size() > 1) {
                const SnakeBody& prevHead = snakeVec[1];
                mSnake.teleportToPosition(prevHead.getX(), prevHead.getY());
            }
            mSnake.reverseDirection();
            mInvincibleTimer = 1.0f;
        }
    }

    /* 5️⃣ 障碍物判定（含穿透逻辑） */
    hitObstacles();
}

