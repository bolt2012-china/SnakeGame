#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

enum class Direction
{
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
};

//Snake body segment
class SnakeBody
{
public:
    SnakeBody();
    SnakeBody(int x, int y);
    int getX() const;
    int getY() const;
    bool operator==(const SnakeBody& other) const;
    bool operator!=(const SnakeBody& other) const { return !(*this == other); }
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
    void sensePortalFood(SnakeBody portalFood); 
    bool touchFood();
    bool touchPortalFood();                 
    void teleportSnake();                      
    void teleportToPosition(int x, int y);  
    // Check if the snake is dead
    bool hitWall();
    bool hitSelf();
    bool checkCollision();
    bool changeDirection(Direction newDirection);
    Direction getDirection() const { return mDirection; }
    const std::vector<SnakeBody>& getSnake();
    void reverseDirection();
    int getLength();
    SnakeBody createNewHead();
    bool moveFoward();
    void setSpeedMultiplier(float multiplier); 
    float getSpeedMultiplier() const { return mSpeedMultiplier; }
    bool isAccelerating() const { return mSpeedMultiplier > 1.0f; }
    void setAccelerationEffect(bool active) {  }

    int getHitPoints() const { return mHitPoints; }
    void decreaseHitPoints(int amount = 1);
    void resetHitPoints() { mHitPoints = mInitialHitPoints; }
    void resetToInitial();
    void shrink();

    void grow();

    void increaseHitPoints(int amount = 1);
    
    void rollback(int steps);

private:
    const int mGameBoardWidth;
    const int mGameBoardHeight;
    
    // Snake information
    const int mInitialSnakeLength;
    Direction mDirection;
    SnakeBody mFood;
    SnakeBody mPortalFood;              
    std::vector<SnakeBody> mSnake;
    float mSpeedMultiplier = 1.0f; 

    int mHitPoints = 1;        
    const int mInitialHitPoints = 1;
    float mHitEffectTimer;
};

#endif
