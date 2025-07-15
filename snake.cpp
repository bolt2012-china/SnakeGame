#include <string>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "snake.h"


SnakeBody::SnakeBody()
{
}


SnakeBody::SnakeBody(int x, int y): mX(x), mY(y)
{
}

int SnakeBody::getX() const
{
    return mX;
}

int SnakeBody::getY() const
{
    return mY;
}

bool SnakeBody::operator == (const SnakeBody& snakeBody)
{
	return this->mX == snakeBody.mX && this->mY == snakeBody.mY;
}

Snake::Snake(int gameBoardWidth, int gameBoardHeight, int initialSnakeLength): mGameBoardWidth(gameBoardWidth), mGameBoardHeight(gameBoardHeight), mInitialSnakeLength(initialSnakeLength)
{
    this->initializeSnake();
    this->setRandomSeed();
}

void Snake::setRandomSeed()
{
    // use current time as seed for random generator
    std::srand(std::time(nullptr));
}

void Snake::initializeSnake()
{
    // Instead of using a random initialization algorithm
    // We always put the snake at the center of the game mWindows
    mSnake.clear(); 
    
    int centerX = this->mGameBoardWidth / 2;
    int centerY = this->mGameBoardHeight / 2;

    for (int i = 0; i < this->mInitialSnakeLength; i ++)
    {
        this->mSnake.push_back(SnakeBody(centerX, centerY + i));
    }
    this->mDirection = Direction::Up;
}

bool Snake::isPartOfSnake(int x, int y)
{
	for (const SnakeBody& bodyPart : this->mSnake){
        if (bodyPart.getX() == x && bodyPart.getY() == y){
            return true;
        }  
    }
    return false;
}

/*
 * Assumption:
 * Only the head would hit wall.
 */
bool Snake::hitWall()
{
		// TODO check if the snake has hit the wall
    int xhead = this-> mSnake[0].getX();
    int yhead = this-> mSnake[0].getY();
    
    if (xhead <= 0 || xhead >= this->mGameBoardWidth-1 || 
        yhead <= 0 || yhead >= this->mGameBoardHeight-1){
        return true;
    }
    return false;
}

/*
 * The snake head is overlapping with its body
 */
bool Snake::hitSelf()
{
	SnakeBody& head = this->mSnake[0];

    for (size_t i = 1; i < this->mSnake.size(); ++i)
    {
        // check if bite itself
        if (head == this->mSnake[i])
        {
            return true; 
        }
    }

    return false; 
}


bool Snake::touchFood()
{
    return !mSnake.empty() && (mSnake[0] == mFood);
}

void Snake::senseFood(SnakeBody food)
{
    this->mFood = food;
}

void Snake::sensePortalFood(SnakeBody portalFood)
{
    this->mPortalFood = portalFood;
}

bool Snake::touchPortalFood()
{
    return !mSnake.empty() && (mSnake[0] == mPortalFood);
}

void Snake::grow() {                         // 方便 Portal 二连吃
    if (!mSnake.empty())
        mSnake.push_back(mSnake.back());
}

void Snake::teleportSnake()
{
    if (mSnake.empty()) return;
    
    // 找到一个安全的传送位置（不在蛇身上）
    int attempts = 0;
    int newX, newY;
    do {
        newX = std::rand() % mGameBoardWidth;
        newY = std::rand() % mGameBoardHeight;
        attempts++;
    } while (isPartOfSnake(newX, newY) && attempts < 100);
    
    // 传送蛇头到新位置
    if (attempts < 100) {
        mSnake[0] = SnakeBody(newX, newY);
    }
}

const std::vector<SnakeBody>& Snake::getSnake()
{
    return this->mSnake;
}

bool Snake::changeDirection(Direction newDirection)
{
    switch (this->mDirection)
    {
        case Direction::Up:
        {
			if (newDirection == Direction::Down){
                return false;
            }
            break;
        }
        case Direction::Down:
        {
			if (newDirection == Direction::Up){
                return false;
            }            break;
        }
        case Direction::Left:
        {
			if (newDirection == Direction::Right){
                return false;
            }
            break;
        }
        case Direction::Right:
        {
		    if (newDirection == Direction::Left){
                return false;
            }
            break;
        }
    }

    this->mDirection = newDirection;
    return true;
}


SnakeBody Snake::createNewHead()
{
    if (mSnake.empty()) return SnakeBody(0, 0); // 判空保护
    int headX = this->mSnake[0].getX();
    int headY = this->mSnake[0].getY();
    
    // Modify according to current direction
    switch (this->mDirection)
    {
        case Direction::Up:
            headY -= 1;
            break;
        case Direction::Down:
            headY += 1;
            break;
        case Direction::Left:
            headX -= 1;
            break;
        case Direction::Right:
            headX += 1;
            break;
    }

    SnakeBody newHead(headX, headY);
    return newHead;
}


/*
 * If eat food, return true, otherwise return false
 */
bool Snake::moveFoward()
{
    if (mSnake.empty()) return false; // 判空保护
    SnakeBody newHead = this->createNewHead();
    this->mSnake.insert(this->mSnake.begin(), newHead);  // Add new head

    if (this->touchFood())
    {
        return true; // Eat food
    }
    else
    {
        // Not eat food
        this->mSnake.pop_back();
        return false;
    }
}

bool Snake::checkCollision()
{
    if (this->hitWall() || this->hitSelf())
    {
        return true;
    }
    else
    {
        return false;
    }
}


int Snake::getLength()
{
    return this->mSnake.size();
}

void Snake::setSpeedMultiplier(float multiplier) {
    if (multiplier < 0.5f) multiplier = 0.5f;
    if (multiplier > 3.0f) multiplier = 3.0f;
    mSpeedMultiplier = multiplier;
}

void Snake::decreaseHitPoints(int amount) {
    mHitPoints -= amount;
    if (mHitPoints < 0) mHitPoints = 0;
    
    // 生命值减少时的视觉反馈
    if (mHitPoints > 0) {
        // 闪烁效果（在渲染中实现）
        mHitEffectTimer = 0.5f; // 0.5秒闪烁效果
    }
}

void Snake::resetToInitial() {
    initializeSnake();
    mDirection = Direction::Up;
    // 保留生命值，只重置位置
}

void Snake::shrink() {
    if (mSnake.size() > 1) {
        mSnake.pop_back(); // 移除尾部
    }
}

void Snake::teleportToPosition(int x, int y)
{
    if (mSnake.empty()) return;
    
    // 确保目标位置在游戏区域内
    if (x >= 0 && x < mGameBoardWidth && y >= 0 && y < mGameBoardHeight) {
        // 传送蛇头到指定位置
        mSnake[0] = SnakeBody(x, y);
    }
}
