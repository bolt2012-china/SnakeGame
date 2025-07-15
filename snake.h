#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

enum class Direction
{
    Up = 0,          // North (0°)
    UpRight = 1,     // Northeast (60°)
    Right = 2,       // East (90°) - Positive Right
    DownRight = 3,   // Southeast (120°)
    Down = 4,        // South (180°)
    DownLeft = 5,    // Southwest (240°)
    Left = 6,        // West (270°) - Positive Left  
    UpLeft = 7,      // Northwest (300°)
};

//Snake body segment
class SnakeBody
{
public:
    SnakeBody();
    SnakeBody(int x, int y);
    int getX() const;
    int getY() const;
    bool operator == (const SnakeBody& snakeBody);
private:
    int mX;
    int mY;
};

// Snake class should have no depency on the GUI library
class Snake
{
public:
    //Snake();
    Snake(int gameBoardWidth, int gameBoardHeight, int initialSnakeLength);
    // Set  seed
    void setRandomSeed();
    // Initialize snake
    void initializeSnake();
    // Checking API for generating random food
    bool isPartOfSnake(int x, int y);
    void senseFood(SnakeBody food);
    void sensePortalFood(SnakeBody portalFood);  // 感知传送食物
    bool touchFood();
    bool touchPortalFood();                      // 检测是否碰到传送食物
    void teleportSnake();                        // 传送蛇到随机位置
    void teleportToPosition(int x, int y);       // 传送蛇到指定位置
    // Check if the snake is dead
    bool hitWall();
    bool hitSelf();
    bool checkCollision();
    bool changeDirection(Direction newDirection);
    Direction getDirection() const { return mDirection; }
    const std::vector<SnakeBody>& getSnake();
    int getLength();
    SnakeBody createNewHead();
    bool moveFoward();
    void setSpeedMultiplier(float multiplier); // 新增速度控制方法
    float getSpeedMultiplier() const { return mSpeedMultiplier; }
    bool isAccelerating() const { return mSpeedMultiplier > 1.0f; }
    void setAccelerationEffect(bool active) { /* 可添加粒子效果标记 */ }

    int getHitPoints() const { return mHitPoints; }
    void decreaseHitPoints(int amount = 1);
    void resetHitPoints() { mHitPoints = mInitialHitPoints; }
    void resetToInitial();
    void shrink(); // 蛇长度减1

    void grow();   // 让蛇体立即增长一节（不移动）

    // Hexagonal direction helper functions
    Direction getLeftTurn() const;          // Turn 60° counter-clockwise
    Direction getRightTurn() const;         // Turn 60° clockwise
    Direction getSharpLeftTurn() const;     // Turn 120° counter-clockwise  
    Direction getSharpRightTurn() const;    // Turn 120° clockwise
    Direction getPositiveLeftTurn() const;  // Turn 90° counter-clockwise (2 hex steps)
    Direction getPositiveRightTurn() const; // Turn 90° clockwise (2 hex steps)

private:
    const int mGameBoardWidth;
    const int mGameBoardHeight;
    
    // Snake information
    const int mInitialSnakeLength;
    Direction mDirection;
    SnakeBody mFood;
    SnakeBody mPortalFood;                       // 传送食物
    std::vector<SnakeBody> mSnake;
    float mSpeedMultiplier = 1.0f; // 速度乘数

    int mHitPoints = 1;          // 当前生命值
    const int mInitialHitPoints = 1; // 初始生命值
    float mHitEffectTimer; // 生命值减少时的闪烁效果计时器
};

#endif
