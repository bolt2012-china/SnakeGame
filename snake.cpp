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
    SnakeBody newHead = this->createNewHead();
    if (this->mFood == newHead)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Snake::senseFood(SnakeBody food)
{
    this->mFood = food;
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
