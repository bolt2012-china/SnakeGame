#include "GameSFML.h"
#include <SFML/System/Angle.hpp> 
#include <sstream>
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>     
#include <algorithm>   

#ifndef M_PI             
#   define M_PI 3.14159265358979323846
#endif

GameSFML::GameSFML(unsigned int columns,
                   unsigned int rows,
                   unsigned int cellSize)
: mColumns(columns)
, mRows(rows)
, mCellSize(cellSize)
, mWindow(
    sf::VideoMode({ columns * cellSize + 200u,
                    rows    * cellSize }),
    "Snake Game")
, mSnake(columns, rows, 2)
, mFont()
, mPointsText(mFont)
, mDifficultyText(mFont)
, mInstructionText(mFont)
, mUpText(mFont)
, mDownText(mFont)
, mLeftText(mFont)
, mRightText(mFont)
, mLeaderBoardTitleText(mFont)
, mLeader1Text(mFont)
, mLeader2Text(mFont)
, mLeader3Text(mFont)
, mHitPointsText(mFont)  
, mRestartTxt(mFont)   
, mQuitTxt  (mFont)    
, mHighScores{0,0,0}
, mBgSprite(mBgTexture) 
, mHeadTexture{}
, mHeadSprite(mHeadTexture)
, mPauseTxt (mFont)  
, mHomeTxt  (mFont) 
, mPauseQuitTxt (mFont) 
{
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

    loadHighScores();
    // 加载字体
    if (!mFont.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "ERROR: cannot load arial.ttf – check path!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    //侧边栏的框
    float sidebarW = 200.f;
    float sidebarH = static_cast<float>(mRows * mCellSize);

    mSidebarBorder.setSize({sidebarW - 2.f, sidebarH - 2.f});
    mSidebarBorder.setPosition({static_cast<float>(mColumns * mCellSize) + 1.f, 1.f});
    mSidebarBorder.setFillColor(sf::Color::Transparent);
    mSidebarBorder.setOutlineThickness(2.f);
    mSidebarBorder.setOutlineColor(sf::Color::White);

        // ——— Pause / Resume 按钮 ———
    const float btnW = 180.f, btnH = 40.f;
    mPauseBtn.setSize({btnW, btnH});
    mPauseBtn.setPosition({ float(mColumns * mCellSize + 10),      // 左边距
                            float(mRows    * mCellSize - 60.f) }); // 底边距 20px
    mPauseBtn.setFillColor(sf::Color(100,100,100));

    mPauseTxt.setCharacterSize(18);
    mPauseTxt.setString("Pause");
    sf::FloatRect box = mPauseTxt.getLocalBounds();
    mPauseTxt.setOrigin(sf::Vector2f(box.size.x * 0.5f,
                                 box.size.y * 0.5f));

    mPauseTxt.setPosition(mPauseBtn.getPosition()
                     + sf::Vector2f(btnW/2.f, 10.f));
    
    

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


    // Points / Difficulty
    mPointsText.setCharacterSize(16);
    mPointsText.setPosition({ float(columns * cellSize + 10), 120.f });
    mPointsText.setString("Points: " + std::to_string(0));  // 初始值

    mDifficultyText.setCharacterSize(16);
    mDifficultyText.setPosition({ float(columns * cellSize + 10), 150.f });
    mDifficultyText.setString("Difficulty: " + std::to_string(0));  // 初始值

    // 操作说明
    mInstructionText.setCharacterSize(14);
    mInstructionText.setPosition({ float(columns * cellSize + 10), 80.f });
    mInstructionText.setString("WASD: Move\nR: Restart\nEsc: Quit");

    mUpText.setCharacterSize(14);
    mUpText.setPosition({ float(columns * cellSize + 10),  10.f });
    mUpText.setString("Up: W");

    mDownText.setCharacterSize(14);
    mDownText.setPosition({ float(columns * cellSize + 10),  30.f });
    mDownText.setString("Down: S");

    mLeftText.setCharacterSize(14);
    mLeftText.setPosition({ float(columns * cellSize + 10),  50.f });
    mLeftText.setString("Left: A");

    mRightText.setCharacterSize(14);
    mRightText.setPosition({ float(columns * cellSize + 10),  70.f });
    mRightText.setString("Right: D");

    // Leader Board
    mLeaderBoardTitleText.setCharacterSize(16);
    mLeaderBoardTitleText.setPosition({ float(columns * cellSize + 10), 190.f });
    mLeaderBoardTitleText.setString("Leader Board");

    mLeader1Text.setCharacterSize(14);
    mLeader1Text.setPosition({ float(columns * cellSize + 10), 210.f });
    mLeader1Text.setString("#1: " + std::to_string(mHighScores[0]));

    mLeader2Text.setCharacterSize(14);
    mLeader2Text.setPosition({ float(columns * cellSize + 10), 230.f });
    mLeader2Text.setString("#2: " + std::to_string(mHighScores[1]));

    mLeader3Text.setCharacterSize(14);
    mLeader3Text.setPosition({ float(columns * cellSize + 10), 250.f });
    mLeader3Text.setString("#3: " + std::to_string(mHighScores[2]));

    // 在构造函数中，加载字体后
    mHitPointsText.setFont(mFont);
    mHitPointsText.setCharacterSize(20);
    mHitPointsText.setPosition({ float(columns * cellSize + 10), 120.f }); // 在得分下方
    mHitPointsText.setString("Lives: " + std::to_string(mSnake.getHitPoints()));

    // 生成初始食物
    generateFood();
    // 生成初始障碍物
    generateObstacles();
    
    // 初始化三角形边框点，防止hitBoundary()中访问未初始化的点导致段错误
    mTriangleBorder.setPointCount(3);
    mTriangleBorder.setPoint(0, {0, static_cast<float>(mRows * mCellSize)});
    mTriangleBorder.setPoint(1, {static_cast<float>(mColumns * mCellSize / 2), 0});
    mTriangleBorder.setPoint(2, {static_cast<float>(mColumns * mCellSize), 
                                static_cast<float>(mRows * mCellSize)});
    
    // 初始化随机数生成器
    std::srand(static_cast<unsigned>(time(nullptr)));
    currentType = static_cast<mBorderType>(rand() % 2); // 游戏开始时随机地图类型
    mNewBoardActivated = false; // Initialize the flag
}

AppState GameSFML::run()
{
    sf::Clock clock;
    float acc = 0.f; // Declare acc before the loop

    while (mWindow.isOpen()) {
        float dt = clock.restart().asSeconds(); // 每帧更新
        processEvents();                    // 主窗口事件

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();          // 处理对话框事件

        
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { // 只在 Playing 时更新蛇
            acc += dt;
            if (acc >= effectiveDelay) { update(); acc -= effectiveDelay; }
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
            }
        }

        // ───── 鼠标移动：悬停高亮 ────────────────
        else if (const auto* mv = event.getIf<sf::Event::MouseMoved>()) {
            sf::Vector2f p{ float(mv->position.x), float(mv->position.y) };

            // 侧边栏 Pause / Resume
            bool hoverNow = mPauseBtn.getGlobalBounds().contains(p);
            if (hoverNow != mPauseHover) {
                mPauseHover = hoverNow;
                mPauseBtn.setFillColor(hoverNow ? sf::Color(150,150,150)
                                                : sf::Color(100,100,100));
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
        mDifficulty = mPoints / 5;
        mDelay = 0.1f * std::pow(0.75f, static_cast<float>(mDifficulty));
        // 生成初始食物
        generateFood();
        if (mPoints % 5 == 0) {
            mObstacleCount++;
            generateObstacles();
        }
    }
    // If difficulty is 1 and snake enters center area, activate new board permanently
    if (mDifficulty >= 1 && !mNewBoardActivated && isSnakeInCenterArea()) {
        mNewBoardActivated = true;
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
    
    if (shouldCheckBoundary && hitBoundary()) {      
        mSnake.decreaseHitPoints(); // 减少生命值
        if (mSnake.getHitPoints() <= 0) {
            updateHighScores(mPoints); //更新得分记录
            mState = GameState::GameOver;   // 切换状态
            openGameOverDialog();           // 打开对话框
        }
        else {
            mSnake.resetToInitial();
        }
    }
    hitObstacles();
}

void GameSFML::render() {
    mWindow.clear();                  
    if (mBgTexture.getSize().x)        // 确认加载成功
        mWindow.draw(mBgSprite);       // 先画背景
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
        
        float cx = static_cast<float>(mColumns * mCellSize) * 0.5f;
        
        mHomeBtn.setOrigin(mHomeBtn.getSize() * 0.5f);
        mHomeBtn.setPosition({cx, float(mRows*mCellSize) * 0.55f});
        sf::FloatRect hb = mHomeTxt.getLocalBounds();
        mHomeTxt.setOrigin(sf::Vector2f(hb.size.x*0.5f, hb.size.y*0.5f));
        mHomeTxt.setPosition(mHomeBtn.getPosition());
        mWindow.draw(mHomeBtn);
        mWindow.draw(mHomeTxt);
        /* Quit 按钮位置：rows*0.70f */
        mPauseQuitBtn.setOrigin(mPauseQuitBtn.getSize() * 0.5f);
        mPauseQuitBtn.setPosition(sf::Vector2f(cx,
            float(mRows * mCellSize) * 0.70f));

        sf::FloatRect qb = mPauseQuitTxt.getLocalBounds();
        mPauseQuitTxt.setOrigin(sf::Vector2f(qb.size.x*0.5f, qb.size.y*0.5f));
        mPauseQuitTxt.setPosition(mPauseQuitBtn.getPosition());

        mWindow.draw(mPauseQuitBtn);
        mWindow.draw(mPauseQuitTxt);


    }
    mWindow.display();
}

void GameSFML::renderBoard() {
    // 绘制游戏区域边界框

    this->mGameBorder.setPosition(sf::Vector2f(4.f, 4.f)); // 左上角对齐
    this->mGameBorder.setSize({mColumns * mCellSize-12.f, 
                         mRows * mCellSize-12.f});
    this->mGameBorder.setFillColor(sf::Color::Transparent); // 透明填充
    this->mGameBorder.setOutlineThickness(2.f); // 边框厚度
    this->mGameBorder.setOutlineColor(sf::Color::White); // 边框颜色
    this->mWindow.draw(this->mGameBorder);
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
    
    // 三角形边框（点已在构造函数中初始化）
    this->mTriangleBorder.setFillColor(sf::Color::Transparent);
    this->mTriangleBorder.setOutlineThickness(2.0f);
    this->mTriangleBorder.setOutlineColor(sf::Color::White);

    switch(currentType) {
        case mBorderType::Circle:
            mWindow.draw(mCircleBorder);
            break;
        case mBorderType::Triangle:
            mWindow.draw(mTriangleBorder);
            break;
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
            } else if (currentType == mBorderType::Triangle) {
                sf::Vector2f A(0, mRows);
                sf::Vector2f B(mColumns/2.0f, 0);
                sf::Vector2f C(mColumns, mRows);
                float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
                if (std::abs(denom) < 1e-6) { 
                    valid = false; 
                } else {
                    float x = fx + 0.5f;
                    float y = fy + 0.5f;
                    float a = ((B.y - C.y) * (x - C.x) + (C.x - B.x) * (y - C.y)) / denom;
                    float b = ((C.y - A.y) * (x - C.x) + (A.x - C.x) * (y - C.y)) / denom;
                    float c = 1 - a - b;
                    // 确保点在三角形内部且距离边界有一定距离
                    if (!(a >= 0.1 && a <= 0.9 && b >= 0.1 && b <= 0.9 && c >= 0.1 && c <= 0.9)) {
                        valid = false;
                    }
                }
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
    sf::RectangleShape block({ static_cast<float>(mCellSize - 1), static_cast<float>(mCellSize - 1) });
    block.setFillColor(sf::Color::Red);
    block.setPosition({ static_cast<float>(mFood.getX() * mCellSize),
                        static_cast<float>(mFood.getY() * mCellSize) });
    mWindow.draw(block);
}

void GameSFML::renderObstacles() {
    sf::RectangleShape obstacle({static_cast<float>(mCellSize - 2), 
                                static_cast<float>(mCellSize - 2)});
    obstacle.setFillColor(sf::Color(128,0,128)); // 紫色障碍物
    
    for (const auto& obs : mObstacles) {
        obstacle.setPosition(sf::Vector2f(obs.getX() * mCellSize + 1, 
                            obs.getY() * mCellSize + 1));
        mWindow.draw(obstacle);
    }
}


void GameSFML::renderPSFood() {
    // 渲染普通食物（红色）
    sf::RectangleShape regularFood({static_cast<float>(mCellSize-1), 
                                   static_cast<float>(mCellSize-1)});
    regularFood.setFillColor(sf::Color::Red);
    regularFood.setPosition({ static_cast<float>(mRegularFood.getX() * mCellSize),
                        static_cast<float>(mRegularFood.getY() * mCellSize) });
    mWindow.draw(regularFood);

    // 渲染传送食物（蓝色）
    sf::RectangleShape portalFood({static_cast<float>(mCellSize-1), 
                                 static_cast<float>(mCellSize-1)});
    portalFood.setFillColor(sf::Color::Blue);
    portalFood.setPosition({ static_cast<float>(mPortalFood.getX() * mCellSize),
                        static_cast<float>(mPortalFood.getY() * mCellSize) });
    mWindow.draw(portalFood);
}

void GameSFML::renderScoreFood() {
    sf::RectangleShape foodBlock({static_cast<float>(mCellSize - 1), 
                                 static_cast<float>(mCellSize - 1)});
    
    for (const auto& food : mScoreFoods) {
        foodBlock.setFillColor(food.color);
        foodBlock.setPosition({static_cast<float>(food.position.getX() * mCellSize),
                              static_cast<float>(food.position.getY() * mCellSize)});
        mWindow.draw(foodBlock);
    }
}

void GameSFML::renderUI() {
    // 动态更新得分和难度
    mDifficultyText.setString("Difficulty: " + std::to_string(mDifficulty));
    mPointsText.setString("Points: "     + std::to_string(mPoints));

    // 如果后续要保存并更新最高分，再在此处更新 mHighScores 并重设字符串：
    mLeader1Text.setString("#1: " + std::to_string(mHighScores[0]));
    mLeader2Text.setString("#2: " + std::to_string(mHighScores[1]));
    mLeader3Text.setString("#3: " + std::to_string(mHighScores[2]));

    // 绘制所有 UI 元素
    mWindow.draw(mUpText);
    mWindow.draw(mDownText);
    mWindow.draw(mLeftText);
    mWindow.draw(mRightText);

    mWindow.draw(mDifficultyText);
    mWindow.draw(mPointsText);

    mWindow.draw(mLeaderBoardTitleText);
    mWindow.draw(mLeader1Text);
    mWindow.draw(mLeader2Text);
    mWindow.draw(mLeader3Text);
    
    // 更新按钮文字
    mPauseTxt.setString(mState == GameState::Paused ? "Resume" : "Pause");
    sf::FloatRect r = mPauseTxt.getLocalBounds();
    mPauseTxt.setOrigin(sf::Vector2f(r.size.x * 0.5f,
                                    r.size.y * 0.5f));

    // 绘制按钮
    mWindow.draw(mPauseBtn);
    mWindow.draw(mPauseTxt);

     // 更新生命值显示
    mHitPointsText.setString("Lives: " + std::to_string(mSnake.getHitPoints()));
    mWindow.draw(mHitPointsText);

}

void GameSFML::openGameOverDialog()
{
    if (mDialog.isOpen()) return;           // 已打开就别重复建

    mDialog.create(
        sf::VideoMode({300u, 160u}),             
        "Game Over",
        sf::Style::Titlebar | sf::Style::Close);  

    // 按钮外观
    const sf::Vector2f btnSize{100.f, 40.f};
    mRestartBtn.setSize(btnSize);
    mRestartBtn.setFillColor(sf::Color(200,200,200));
    mRestartBtn.setPosition({25.f, 90.f});

    mQuitBtn   .setSize(btnSize);
    mQuitBtn   .setFillColor(sf::Color(200,200,200));
    mQuitBtn   .setPosition({175.f, 90.f});

    // 按钮文字
    mRestartTxt.setString("Restart");
    mRestartTxt.setCharacterSize(18);
    mRestartTxt.setFillColor(sf::Color::Black);
    mRestartTxt.setPosition(mRestartBtn.getPosition() + sf::Vector2f(15.f, 8.f));

    mQuitTxt.setString("Quit");
    mQuitTxt.setCharacterSize(18);
    mQuitTxt.setFillColor(sf::Color::Black);
    mQuitTxt.setPosition(mQuitBtn.getPosition() + sf::Vector2f(30.f, 8.f));

}

void GameSFML::processDialogEvents()
{
    while (auto evt = mDialog.pollEvent()) {
        // evt 是 std::optional<sf::Event>，箭头运算符直接访问内部对象
        if (evt->is<sf::Event::Closed>()) {        // 关闭按钮
            mWindow.close();
            mDialog.close();
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
            else if (mQuitBtn.getGlobalBounds().contains(pos)) {
                mWindow.close();
                mDialog.close();
            }
        }
    }

    // 绘制对话框
    mDialog.clear(sf::Color(50,50,50));
    mDialog.draw(mRestartBtn);  mDialog.draw(mQuitBtn);
    mDialog.draw(mRestartTxt);  mDialog.draw(mQuitTxt);
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
    currentType = static_cast<mBorderType>(rand() % 2); // 重启时随机地图类型
    mNewBoardActivated = false; // Reset the flag on restart
}


void GameSFML::loadHighScores()
{
    std::ifstream in("highscores.txt");
    if (!in) return;                    // 第一次运行时文件可能不存在

    for (int i = 0; i < 3 && in; ++i)
        in >> mHighScores[i];
}

void GameSFML::saveHighScores()
{
    std::ofstream out("highscores.txt", std::ios::trunc);
    if (!out) {
        std::cerr << "Cannot write highscores.txt" << std::endl;
        return;
    }
    for (int s : mHighScores) out << s << '\n';
}

void GameSFML::updateHighScores(int score)
{
    // 临时拷贝 + 排序，保持 mHighScores 为 Top-3
    std::array<int,4> tmp;
    std::copy(mHighScores.begin(), mHighScores.end(), tmp.begin());
    tmp[3] = score;
    std::sort(tmp.begin(), tmp.end(), std::greater<>());
    std::copy_n(tmp.begin(), 3, mHighScores.begin());

    saveHighScores();                   // 写文件
}


bool GameSFML::hitBoundary()
{
    // 检查蛇头是否碰到边界
    const auto& snakeVec = mSnake.getSnake();
    if (snakeVec.empty()) return false;
    if (mDifficulty == 0)
        return mSnake.checkCollision();
    else if (mDifficulty >= 1) {
        // 新增边框碰撞检测
        SnakeBody head = snakeVec.front();
        float headX = head.getX()*mCellSize + mCellSize / 2.0f;
        float headY = head.getY()*mCellSize + mCellSize / 2.0f;
        
        // 根据边框类型检测碰撞
        switch(currentType) {
            case mBorderType::Circle: {
                // 圆形边框碰撞
                float centerX = mColumns*mCellSize / 2.0f;
                float centerY = mRows*mCellSize / 2.0f;
                float radius = std::min(mColumns*mCellSize, mRows*mCellSize) / 2.0f;
                float distance = std::sqrt(std::pow(headX - centerX, 2) + 
                                        std::pow(headY - centerY, 2));
                if (distance >= radius) return true;
                break;
            }
            case mBorderType::Triangle: {
                // 三角形边框碰撞 
                sf::Vector2f A = sf::Vector2f(mTriangleBorder.getPoint(0));
                sf::Vector2f B = sf::Vector2f(mTriangleBorder.getPoint(1));
                sf::Vector2f C = sf::Vector2f(mTriangleBorder.getPoint(2));

                // 计算重心坐标
                float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
                if (std::abs(denom) < 1e-6) return false; // 防止除零
                float a = ((B.y - C.y) * (headX - C.x) + (C.x - B.x) * (headY - C.y)) / denom;
                float b = ((C.y - A.y) * (headX - C.x) + (A.x - C.x) * (headY - C.y)) / denom;
                float c = 1 - a - b;
                // 点在三角形内当且仅当所有重心坐标在[0,1]范围内
                return !((a >= 0 && a <= 1) && (b >= 0 && b <= 1) && (c >= 0 && c <= 1));
            }
        }
    }
    return false;
}

void GameSFML::runPortalMode()
{
    sf::Clock clock;
    float acc = 0.f;
    
    // 在传送门模式中强制使用基础边界（难度0）
    mDifficulty = 0;
    
    // 生成初始的传送门食物
    generatePortalFood();
    
    while (mWindow.isOpen()) {
        processEvents();                    // 主窗口事件

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();          // 处理对话框事件

        float dt = clock.restart().asSeconds();
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { // 只在 Playing 时更新蛇
            acc += dt;
            if (acc >= effectiveDelay) { 
                updatePortalMode(); 
                acc -= effectiveDelay; 
            }
        }

        renderPortalMode();                 // 使用传送门模式专用渲染
    }
}

void GameSFML::updatePortalMode() {
    // Check if snake touched portal food first
    if (mSnake.touchPortalFood()) {
        mSnake.teleportSnake();
        mPoints += 5; // Bonus points for using portal
        // Generate new portal food after teleportation
        generatePortalFood();
        return; // Skip normal movement processing this frame
    }
    
    if (mSnake.moveFoward()) {
        ++mPoints;
        // In portal mode, difficulty increases slower
        mDifficulty = mPoints / 10; 
        mDelay = 0.1f * std::pow(0.85f, static_cast<float>(mDifficulty));
        // Generate new food after eating regular food
        generatePortalFood();
    }
    
    // Portal mode uses basic rectangular boundaries only
    if (mSnake.checkCollision()) {      
        updateHighScores(mPoints);
        mState = GameState::GameOver;
        openGameOverDialog();
    }
}

void GameSFML::renderPortalMode() {
    mWindow.clear(sf::Color::Black);
    
    // Always use basic rectangular board in portal mode
    renderBoard(); // This renders the basic rectangular border
    
    renderPSFood();  // Renders both regular and portal food
    renderSnake();
    mWindow.draw(mSidebarBorder);
    renderUI();
    mWindow.display();
}

void GameSFML::generatePortalFood() {
    // 生成普通食物（红色） - 简化版本用于基础游戏板，确保不在边界上
    std::srand(static_cast<unsigned>(time(nullptr)));
    while (true) {
        int fx = 1 + std::rand() % (mColumns - 2);  // 避免边界 (1 到 mColumns-2)
        int fy = 1 + std::rand() % (mRows - 2);     // 避免边界 (1 到 mRows-2)
        if (!mSnake.isPartOfSnake(fx, fy)) {
            mRegularFood = SnakeBody(fx, fy);  // 普通食物
            break;
        }
    }

    // 生成传送食物（蓝色） - 简化版本用于基础游戏板，确保不在边界上
    while (true) {
        int fx = 1 + std::rand() % (mColumns - 2);  // 避免边界 (1 到 mColumns-2)
        int fy = 1 + std::rand() % (mRows - 2);     // 避免边界 (1 到 mRows-2)
        // 确保不与蛇身和普通食物重叠
        if (!mSnake.isPartOfSnake(fx, fy) && 
           !(fx == mRegularFood.getX() && fy == mRegularFood.getY())) {
            mPortalFood = SnakeBody(fx, fy);  // 传送食物
            break;
        }
    }

    // 蛇感知普通食物（传送食物需要特殊处理）
    mSnake.senseFood(mRegularFood);
    mSnake.sensePortalFood(mPortalFood);
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
            
            // 检查是否在有效的边界区域内
            if (mDifficulty == 0) {
                // 矩形边界：确保不在边界上
                if (ox == 0 || ox == mColumns - 1 || oy == 0 || oy == mRows - 1) {
                    valid = false;
                    continue;
                }
            } else {
                // 针对圆形和三角形地图
                if (currentType == mBorderType::Circle) {
                    float centerX = mColumns / 2.0f;
                    float centerY = mRows / 2.0f;
                    float radius = std::min(mColumns, mRows) / 2.0f;
                    float dx = ox + 0.5f - centerX;
                    float dy = oy + 0.5f - centerY;
                    if (std::sqrt(dx*dx + dy*dy) > radius) {
                        valid = false;
                        continue;
                    }
                } else if (currentType == mBorderType::Triangle) {
                    sf::Vector2f A(0, mRows);
                    sf::Vector2f B(mColumns/2.0f, 0);
                    sf::Vector2f C(mColumns, mRows);
                    float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
                    if (std::abs(denom) < 1e-6) { 
                        valid = false; 
                        continue;
                    }
                    float x = ox + 0.5f;
                    float y = oy + 0.5f;
                    float a = ((B.y - C.y) * (x - C.x) + (C.x - B.x) * (y - C.y)) / denom;
                    float b = ((C.y - A.y) * (x - C.x) + (A.x - C.x) * (y - C.y)) / denom;
                    float c = 1 - a - b;
                    if (!(a >= 0 && a <= 1 && b >= 0 && b <= 1 && c >= 0 && c <= 1)) {
                        valid = false;
                        continue;
                    }
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
            
            // 检查是否与普通食物重叠
            if (valid && ox == mRegularFood.getX() && oy == mRegularFood.getY()) {
                valid = false;
            }
            
            // 检查是否与传送食物重叠
            if (valid && ox == mPortalFood.getX() && oy == mPortalFood.getY()) {
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
                    // 正常状态：碰撞死亡
                    mSnake.decreaseHitPoints();
                    if (mSnake.getHitPoints() <= 0) {
                        mState = GameState::GameOver;
                        openGameOverDialog();
                    } else {
                        mSnake.resetToInitial();
                        generateFood();
                        mInvincibleTimer = 1.0f;
                    }
                    return true; // 处理后直接返回
                }
            }
        }
    }
    return false;
}

void GameSFML::generateScoreFood() {
    // 清空现有的分数模式食物
    mScoreFoods.clear();
    
    // 定义不同颜色的食物和对应分值（减少种类）
    std::vector<std::pair<sf::Color, int>> foodTypes = {
        {sf::Color::Red, 1},        // 红色食物：1分
        {sf::Color::Green, 2},      // 绿色食物：2分  
        {sf::Color::Blue, 3},       // 蓝色食物：3分
        {sf::Color::Yellow, 5}      // 黄色食物：5分
    };
    
    // 生成适量食物（减少数量）
    int totalFoods = std::min(15, static_cast<int>(mColumns * mRows) / 25); // 最多15个食物
    
    std::srand(static_cast<unsigned>(time(nullptr)));
    
    for (int i = 0; i < totalFoods; ++i) {
        bool validPosition = false;
        int attempts = 0;
        const int MAX_ATTEMPTS = 100;
        
        while (!validPosition && attempts < MAX_ATTEMPTS) {
            int fx = 1 + std::rand() % (mColumns - 2);  // 避免边界
            int fy = 1 + std::rand() % (mRows - 2);     // 避免边界
            attempts++;
            
            validPosition = true;
            
            // 检查是否与蛇身重叠
            if (mSnake.isPartOfSnake(fx, fy)) {
                validPosition = false;
                continue;
            }
            
            // 检查是否与已生成的食物重叠
            for (const auto& food : mScoreFoods) {
                if (food.position.getX() == fx && food.position.getY() == fy) {
                    validPosition = false;
                    break;
                }
            }
            
            // 检查是否与障碍物重叠
            if (validPosition) {
                for (const auto& obs : mObstacles) {
                    if (obs.getX() == fx && obs.getY() == fy) {
                        validPosition = false;
                        break;
                    }
                }
            }
            
            // 检查是否与隧道重叠
            if (validPosition) {
                for (const auto& tunnel : mScoreTunnels) {
                    if ((tunnel.entrance.getX() == fx && tunnel.entrance.getY() == fy) ||
                        (tunnel.exit.getX() == fx && tunnel.exit.getY() == fy)) {
                        validPosition = false;
                        break;
                    }
                }
            }
            
            if (validPosition) {
                // 随机选择食物类型
                int typeIndex = std::rand() % foodTypes.size();
                auto& foodType = foodTypes[typeIndex];
                
                ColoredFood newFood;
                newFood.position = SnakeBody(fx, fy);
                newFood.color = foodType.first;
                newFood.value = foodType.second;
                
                mScoreFoods.push_back(newFood);
                break;
            }
        }
    }
}

void GameSFML::runScoreMode()
{
    sf::Clock clock;
    float acc = 0.f;
    
    // 在分数模式中强制使用基础边界（难度0）
    mDifficulty = 0;
    
    // 生成初始的多彩食物
    generateScoreFood();
    
    // 生成分数隧道
    generateScoreTunnels();
    
    while (mWindow.isOpen()) {
        processEvents();                    // 主窗口事件

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();          // 处理对话框事件

        float dt = clock.restart().asSeconds();
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { // 只在 Playing 时更新蛇
            acc += dt;
            if (acc >= effectiveDelay) { 
                updateScoreMode(); 
                acc -= effectiveDelay; 
            }
        }

        renderScoreMode();                  // 使用分数模式专用渲染
    }
}

void GameSFML::updateScoreMode() {
    SnakeBody newHead = mSnake.createNewHead();
    
    // 检查是否进入隧道入口
    for (auto& tunnel : mScoreTunnels) {
        if (tunnel.isActive && 
            newHead.getX() == tunnel.entrance.getX() && 
            newHead.getY() == tunnel.entrance.getY()) {
            
            // 蛇进入隧道，传送到出口
            mSnake.teleportToPosition(tunnel.exit.getX(), tunnel.exit.getY());
            mPoints += tunnel.bonusPoints; // 获得奖励分数
            
            // 隧道使用后暂时失效
            tunnel.isActive = false;
            
            // 难度逐渐增加
            mDifficulty = mPoints / 20;
            mDelay = 0.1f * std::pow(0.85f, static_cast<float>(mDifficulty));
            
            return; // 使用隧道后直接返回
        }
    }
    
    // 检查是否吃到彩色食物（在移动之前检查）
    bool ateFood = false;
    for (auto it = mScoreFoods.begin(); it != mScoreFoods.end(); ++it) {
        if (newHead.getX() == it->position.getX() && newHead.getY() == it->position.getY()) {
            mPoints += it->value;  // 根据食物颜色加分
            
            // 在移除食物之前，先让蛇感知到食物位置
            mSnake.senseFood(it->position);
            
            mScoreFoods.erase(it); // 移除被吃掉的食物
            
            // 现在移动蛇，由于已经感知到食物，它会增长
            mSnake.moveFoward(); // 移动蛇并增长
            
            ateFood = true;
            
            // 如果食物数量太少，重新生成
            if (mScoreFoods.size() < 10) {
                generateScoreFood();
            }
            
            // 每获得一定分数后重新激活隧道
            if (mPoints % 50 == 0) {
                for (auto& tunnel : mScoreTunnels) {
                    tunnel.isActive = true;
                }
                // 也可以重新生成隧道位置
                generateScoreTunnels();
            }
            
            // 难度逐渐增加
            mDifficulty = mPoints / 20; // 分数模式难度增加更慢
            mDelay = 0.1f * std::pow(0.85f, static_cast<float>(mDifficulty));
            
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
    if (mSnake.checkCollision()) {      
        updateHighScores(mPoints);
        mState = GameState::GameOver;
        openGameOverDialog();
    }
    
    // 障碍物碰撞检测
    hitObstacles();
}

void GameSFML::renderScoreMode() {
    mWindow.clear(sf::Color::Black);
    
    // 总是使用基础矩形边界在分数模式中
    renderBoard(); 
    
    renderScoreFood();  // 渲染彩色食物
    renderScoreTunnels(); // 渲染分数隧道
    renderSnake();
    renderObstacles();
    mWindow.draw(mSidebarBorder);
    renderUI();
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

void GameSFML::renderScoreTunnels() {
    for (const auto& tunnel : mScoreTunnels) {
        if (!tunnel.isActive) continue;
        
        // 计算入口和出口的中心点坐标
        float entranceX = tunnel.entrance.getX() * mCellSize + mCellSize / 2.0f;
        float entranceY = tunnel.entrance.getY() * mCellSize + mCellSize / 2.0f;
        float exitX = tunnel.exit.getX() * mCellSize + mCellSize / 2.0f;
        float exitY = tunnel.exit.getY() * mCellSize + mCellSize / 2.0f;
        
        // 计算连接线的长度和角度
        float deltaX = exitX - entranceX;
        float deltaY = exitY - entranceY;
        float Length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        float angle = std::atan2(deltaY, deltaX) * 180.0f / M_PI;
        
        
        // 创建连接线（隧道主体）
        sf::RectangleShape tunnelLine({Length, static_cast<float>(mCellSize / 4)});
        tunnelLine.setFillColor(sf::Color(tunnel.color.r, tunnel.color.g, tunnel.color.b, 128)); // 半透明
        tunnelLine.setPosition({entranceX, entranceY - mCellSize / 8.0f});
        tunnelLine.setRotation(sf::degrees(angle));
        
        // 渲染隧道入口（方形）
        sf::RectangleShape entrance({static_cast<float>(mCellSize - 2), 
                                   static_cast<float>(mCellSize - 2)});
        entrance.setFillColor(tunnel.color);
        entrance.setPosition({static_cast<float>(tunnel.entrance.getX() * mCellSize + 1),
                            static_cast<float>(tunnel.entrance.getY() * mCellSize + 1)});
        
        // 添加入口标识（内部小方块）
        sf::RectangleShape entranceMarker({static_cast<float>(mCellSize / 3), 
                                         static_cast<float>(mCellSize / 3)});
        entranceMarker.setFillColor(sf::Color::White);
        entranceMarker.setPosition({static_cast<float>(tunnel.entrance.getX() * mCellSize + mCellSize/3),
                                  static_cast<float>(tunnel.entrance.getY() * mCellSize + mCellSize/3)});
        
        // 渲染隧道出口（圆形）
        sf::CircleShape exit(static_cast<float>(mCellSize / 2 - 1));
        exit.setFillColor(tunnel.color);
        exit.setPosition({static_cast<float>(tunnel.exit.getX() * mCellSize + 1),
                        static_cast<float>(tunnel.exit.getY() * mCellSize + 1)});
        
        // 添加出口标识（内部小圆）
        sf::CircleShape exitMarker(static_cast<float>(mCellSize / 6));
        exitMarker.setFillColor(sf::Color::White);
        exitMarker.setPosition({static_cast<float>(tunnel.exit.getX() * mCellSize + mCellSize/3),
                              static_cast<float>(tunnel.exit.getY() * mCellSize + mCellSize/3)});
        
        // 先画连接线，再画入口和出口（保证入口出口在最上层）
        mWindow.draw(tunnelLine);
        mWindow.draw(entrance);
        mWindow.draw(entranceMarker);
        mWindow.draw(exit);
        mWindow.draw(exitMarker);
    }
}

bool GameSFML::isSnakeInCenterArea() {
    const auto& snakeVec = mSnake.getSnake();
    if (snakeVec.empty()) return false;
    
    SnakeBody head = snakeVec.front();
    float headX = head.getX() + 0.5f;
    float headY = head.getY() + 0.5f;
    
    // Define center area based on the current border type
    switch(currentType) {
        case mBorderType::Circle: {
            float centerX = mColumns / 2.0f;
            float centerY = mRows / 2.0f;
            float radius = std::min(mColumns, mRows) / 2.0f;
            float dx = headX - centerX;
            float dy = headY - centerY;
            float distance = std::sqrt(dx*dx + dy*dy);
            // Snake is in center area if within 70% of the radius
            return distance <= (radius * 0.7f);
        }
        case mBorderType::Triangle: {
            sf::Vector2f A(0, mRows);
            sf::Vector2f B(mColumns/2.0f, 0);
            sf::Vector2f C(mColumns, mRows);
            float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
            if (std::abs(denom) < 1e-6) return false;
            
            float a = ((B.y - C.y) * (headX - C.x) + (C.x - B.x) * (headY - C.y)) / denom;
            float b = ((C.y - A.y) * (headX - C.x) + (A.x - C.x) * (headY - C.y)) / denom;
            float c = 1 - a - b;
            // Snake is in center area if within 70% margin from edges
            return (a >= 0.15 && a <= 0.85 && b >= 0.15 && b <= 0.85 && c >= 0.15 && c <= 0.85);
        }
    }
    return false;
}