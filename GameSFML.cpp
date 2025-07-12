#include "GameSFML.h"
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>     
#include <algorithm>   

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
, mRestartTxt(mFont)   
, mQuitTxt  (mFont)    
, mHighScores{0,0,0}
{
    loadHighScores();
    // 加载字体
    if (!mFont.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
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

    // 生成初始食物
    std::srand(static_cast<unsigned>(time(nullptr)));
    while (true) {
        int fx = std::rand() % mColumns;
        int fy = std::rand() % mRows;
        // 检查是否在蛇体上
        if (mSnake.isPartOfSnake(fx, fy)) continue;
        // 检查是否在有效区域
        bool valid = true;
        if (currentType == mBorderType::Circle) {
            float centerX = mColumns / 2.0f;
            float centerY = mRows / 2.0f;
            float radius = std::min(mColumns, mRows) / 2.0f;
            float dx = fx + 0.5f - centerX;
            float dy = fy + 0.5f - centerY;
            if (std::sqrt(dx*dx + dy*dy) > radius) valid = false;
        } else if (currentType == mBorderType::Triangle) {
            sf::Vector2f A(0, mRows);
            sf::Vector2f B(mColumns/2.0f, 0);
            sf::Vector2f C(mColumns, mRows);
            float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
            float x = fx + 0.5f;
            float y = fy + 0.5f;
            float a = ((B.y - C.y) * (x - C.x) + (C.x - B.x) * (y - C.y)) / denom;
            float b = ((C.y - A.y) * (x - C.x) + (A.x - C.x) * (y - C.y)) / denom;
            float c = 1 - a - b;
            if (!(a >= 0 && a <= 1 && b >= 0 && b <= 1 && c >= 0 && c <= 1)) valid = false;
        }
        if (valid) {
            mFood = SnakeBody(fx, fy);
            break;
        }
    }
    mSnake.senseFood(mFood);
    currentType = static_cast<mBorderType>(rand() % 2); // 游戏开始时随机地图类型
}

void GameSFML::run()
{
    sf::Clock clock;
    float acc = 0.f;

    while (mWindow.isOpen()) {
        processEvents();                    // 主窗口事件

        if (mState == GameState::GameOver && mDialog.isOpen())
            processDialogEvents();          // 处理对话框事件

        float dt = clock.restart().asSeconds();
        float effectiveDelay = mDelay / mSnake.getSpeedMultiplier();

        if (mState == GameState::Playing) { // 只在 Playing 时更新蛇
            acc += dt;
            if (acc >= effectiveDelay) { update(); acc -= effectiveDelay; }
        }

        render();                           // 一直重绘主窗口（静止画面）
    }
}



void GameSFML::processEvents() {
    while (auto eventOpt = mWindow.pollEvent()) {
        const auto& event = *eventOpt;

        // 关闭窗口
        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }
        // 按键按下
        else if (auto key = event.getIf<sf::Event::KeyPressed>()) {
            Direction currentDir = mSnake.getDirection(); // 获取当前方向
            Direction newDir = currentDir; // 初始化新方向
            switch (key->scancode) {
                // 向上
                case sf::Keyboard::Scancode::W:
                case sf::Keyboard::Scancode::Up:
                    newDir = Direction::Up;
                    break;
                // 向下
                case sf::Keyboard::Scancode::S:
                case sf::Keyboard::Scancode::Down:
                    newDir = Direction::Down;
                    break;
                // 向左
                case sf::Keyboard::Scancode::A:
                case sf::Keyboard::Scancode::Left:
                    newDir = Direction::Left;
                    break;
                // 向右
                case sf::Keyboard::Scancode::D:
                case sf::Keyboard::Scancode::Right:
                    newDir = Direction::Right;
                    break;
                // 重启
                case sf::Keyboard::Scancode::R:
                    mPoints = 0;
                    mSnake.initializeSnake();
                    do {
                        int fx = std::rand() % mColumns;
                        int fy = std::rand() % mRows;
                        mFood = SnakeBody(fx, fy);
                    } while (mSnake.isPartOfSnake(mFood.getX(), mFood.getY()));
                    mSnake.senseFood(mFood);
                    mDelay = 0.1f;
                    currentType = static_cast<mBorderType>(rand() % 2); // 重启时随机地图类型
                    break;
                // 退出
                case sf::Keyboard::Scancode::Escape:
                    mWindow.close();
                    break;
                default:
                    break;
            }
            // 同方向键加速逻辑
            if (newDir == currentDir) {
                // 按下同方向键时加速 (1.5倍)
                mSnake.setSpeedMultiplier(1.5f);
                
                // 视觉反馈：蛇头变色
                if (!mSnake.isAccelerating()) {
                    mSnake.setAccelerationEffect(true);
                }
            } else if (newDir != currentDir) {
                // 改变方向时重置速度
                mSnake.setSpeedMultiplier(1.0f);
                // mSnake.setAccelerationEffect(false);
                
                // 更新方向
                mSnake.changeDirection(newDir);
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
        std::srand(static_cast<unsigned>(time(nullptr)));
        while (true) {
            int fx = std::rand() % mColumns;
            int fy = std::rand() % mRows;
            // 检查是否在蛇体上
            if (mSnake.isPartOfSnake(fx, fy)) continue;
            // 检查是否在有效区域
            bool valid = true;
            if (currentType == mBorderType::Circle) {
                float centerX = mColumns / 2.0f;
                float centerY = mRows / 2.0f;
                float radius = std::min(mColumns, mRows) / 2.0f;
                float dx = fx + 0.5f - centerX;
                float dy = fy + 0.5f - centerY;
                if (std::sqrt(dx*dx + dy*dy) > radius) valid = false;
            } else if (currentType == mBorderType::Triangle) {
                sf::Vector2f A(0, mRows);
                sf::Vector2f B(mColumns/2.0f, 0);
                sf::Vector2f C(mColumns, mRows);
                float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
                float x = fx + 0.5f;
                float y = fy + 0.5f;
                float a = ((B.y - C.y) * (x - C.x) + (C.x - B.x) * (y - C.y)) / denom;
                float b = ((C.y - A.y) * (x - C.x) + (A.x - C.x) * (y - C.y)) / denom;
                float c = 1 - a - b;
                if (!(a >= 0 && a <= 1 && b >= 0 && b <= 1 && c >= 0 && c <= 1)) valid = false;
            }
            if (valid) {
                mFood = SnakeBody(fx, fy);
                mSnake.senseFood(mFood); // 更新蛇对食物的感知
                break;
            }
        }
    }
    if (hitBoundary()) {      
        updateHighScores(mPoints); //更新得分记录
        mState = GameState::GameOver;   // 切换状态
        openGameOverDialog();           // 打开对话框
    }
}

void GameSFML::render() {
    mWindow.clear(sf::Color::Black);
    if (mDifficulty == 0) {
        renderBoard(); 
    } else {
        renderNewBoard(); 
    }
    renderBoard();
    renderFood();
    renderSnake();
    mWindow.draw(mSidebarBorder);
    renderUI();
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
    
    // 三角形边框
    this->mTriangleBorder.setPointCount(3);
    this->mTriangleBorder.setPoint(0, {0, static_cast<float>(mRows * mCellSize)});
    this->mTriangleBorder.setPoint(1, {static_cast<float>(mColumns * mCellSize / 2), 0});
    this->mTriangleBorder.setPoint(2, {static_cast<float>(mColumns * mCellSize), 
                                static_cast<float>(mRows * mCellSize)});
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
    sf::RectangleShape block({ static_cast<float>(mCellSize - 1), static_cast<float>(mCellSize - 1) });
    block.setFillColor(sf::Color::Green);
    SnakeBody head = mSnake.getSnake().front();
    for (SnakeBody part : mSnake.getSnake()) {
        block.setPosition({ static_cast<float>(part.getX() * mCellSize),
                            static_cast<float>(part.getY() * mCellSize) });

        if (part == head && mSnake.isAccelerating()) {
            // 如果是蛇头且正在加速，改变颜色
            block.setFillColor(sf::Color(255, 200, 50)); // 金色
        } else {
            block.setFillColor(sf::Color::Green); // 恢复默认颜色
        }
        mWindow.draw(block);
    }
    
}


void GameSFML::renderFood() {
    sf::RectangleShape block({ static_cast<float>(mCellSize - 1), static_cast<float>(mCellSize - 1) });
    block.setFillColor(sf::Color::Red);
    block.setPosition({ static_cast<float>(mFood.getX() * mCellSize),
                        static_cast<float>(mFood.getY() * mCellSize) });
    mWindow.draw(block);
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

    mSnake.initializeSnake();

    do {
        int fx = std::rand() % mColumns;
        int fy = std::rand() % mRows;
        mFood = SnakeBody(fx, fy);
    } while (mSnake.isPartOfSnake(mFood.getX(), mFood.getY()));
    mSnake.senseFood(mFood);
    currentType = static_cast<mBorderType>(rand() % 2); // 重启时随机地图类型
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
    if (mDifficulty==0)
        return mSnake.checkCollision();
    else if (mDifficulty==1) {
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
