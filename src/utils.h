#pragma once

#include "constants.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
// #include <SDL2/SDL_mixer.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <set>

// 使用高分辨率时钟
using Clock = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<double, std::milli>;

namespace snake {
    //BGRA
    extern uint8_t snakehead_pixel[4*4];
    extern uint8_t snakebody_pixel[4*4];
    extern uint8_t snakeapple_pixel[16*4];

    struct SnakeData; // 构成链表的node，存储蛇的数据

    class Renderable;
    class Snake;
    class Apple;
    class Round;
    enum Direction { NORTH, WEST, SOUTH, EAST };

    void testing();
    void snakePrevLocation(SnakeData* currData, int &prevX, int &prevY);
    const bool inGrid(int x, int y);
    SDL_FRect getDrect(int grid_x, int grid_y);
}

namespace utils {
    void test_utils();

    class Timer;

    extern std::mt19937 rng_loc; //随机数
}

class utils::Timer {
public:
    Clock::time_point startTime;
    Clock::time_point pauseTime;
    bool paused = false;
    Timer() {
    }
    void reset() {
        startTime = Clock::now();
        pauseTime = startTime;
        paused = false;
    }

    void pause() {
        pauseTime = Clock::now();
        paused = true;
    }

    void resume() {
        startTime += Clock::now() - pauseTime;
        paused = false;
    }

    double elapsed() const {
        if (paused) {
            return std::chrono::duration<double, std::milli>(pauseTime - startTime).count();
        }
        return std::chrono::duration<double, std::milli>(Clock::now() - startTime).count();
    }

    auto now() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count();
    }

    auto getStartTime() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count();
    }
};

class snake::Renderable {
public:
    int grid_x, grid_y;
    SDL_FRect drect;
    SDL_Surface* surface;
    Renderable(int grid_x, int grid_y, int pixelX, int pixelY, uint8_t* pixels, int pitch) {
        this->grid_x = grid_x;
        this->grid_y = grid_y;
        this->surface = SDL_CreateSurfaceFrom(pixelX, pixelY, SDL_PIXELFORMAT_RGBA8888, pixels, pitch);
        if (!surface) {
            std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
            std::cout << "pitch: " << pitch << std::endl;
        }
        drect = SDL_FRect({ (grid_x + 0.5f) * constants::GRID_SIZE - constants::GAP + constants::GRID_X, 
                    (grid_y + 0.5f) * constants::GRID_SIZE - constants::GAP + constants::GRID_Y, 
                    constants::GRID_SIZE - constants::GAP * 2, 
                    constants::GRID_SIZE - constants::GAP * 2 });
    }
    ~Renderable() {
        SDL_DestroySurface(surface);
        // std::cout << "Renderable destroyed at " << grid_x << ", " << grid_y << std::endl;
    }
    void draw(SDL_Renderer* renderer) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            std::cerr << "Failed to create texture at " << grid_x << ", " << grid_y << ": " << SDL_GetError() << std::endl;
            return;
        }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_RenderTexture(renderer, texture, NULL, &drect);
        SDL_DestroyTexture(texture);
    }
};

//通过链表存储蛇身数据
struct snake::SnakeData
{
    int x, y;
    snake::Direction direction;
    SnakeData* next;
    SnakeData* prev;

    SDL_Texture* texture = nullptr;
    SnakeData(int x, int y, Direction direction = NORTH) : x(x), y(y), direction(direction), next(nullptr) {}
    void setNext(SnakeData* nextData) {
        next = nextData;
    }
    void setPrev(SnakeData* prevData) {
        prev = prevData;
    }
    void setTexture(SDL_Texture* texture) {
        this->texture = texture;
    }
};

class snake::Snake {
public:
    SnakeData* head;
    SnakeData* tail;
    int length;
    Direction newDirection;

    SDL_Surface* headSurface;
    SDL_Surface* bodySurface;

    // 蛇身是否在增加
    bool growing = false;
    Snake(int grid_x, int grid_y, int initLength = 3, Direction initDirection = NORTH) {
        if (initLength < 2 || initLength > 10) {
            std::cerr << "Invalid initLength: " << initLength << ", replaced with 3." << std::endl;
            initLength = 3;
        }

        length = initLength;

        std::cout << "Snake Length: " << length << std::endl;

        // 构建链表
        head = new SnakeData(grid_x, grid_y, initDirection);
        head->setPrev(nullptr);
        auto prev = head;

        //计算方向
        int dx = 0, dy = 0;
        switch (initDirection)
        {
        case NORTH: dy = 1; break;
        case SOUTH: dy = -1; break;
        case WEST: dx = 1; break;
        case EAST: dx = -1; break;
        }

        // 构建蛇身
        for (int i = 1; i < initLength; i++) {
            SnakeData* data = new SnakeData(grid_x + i * dx, grid_y + i * dy, initDirection);
            prev->setNext(data);
            data->setPrev(prev);
            prev = data;
        }
        tail = prev;
        tail->setNext(nullptr);
        
        newDirection = initDirection; //初始化方向

        // 创建表面
        this->headSurface = SDL_CreateSurfaceFrom(2, 2, SDL_PIXELFORMAT_RGBA8888, snakehead_pixel, 8);
        this->bodySurface = SDL_CreateSurfaceFrom(2, 2, SDL_PIXELFORMAT_RGBA8888, snakebody_pixel, 8);
    }
    ~Snake() {
        SDL_DestroySurface(headSurface);
        SDL_DestroySurface(bodySurface);
        SnakeData* curr = head;
        while (curr) {
            SnakeData* next = curr->next;
            SDL_DestroyTexture(curr->texture);
            delete curr;
            curr = next;
        }
    }
    void draw(SDL_Renderer* renderer) {
        // 绘制蛇头，每次都要重新绘制因为蛇头一直变化
        head->setTexture(SDL_CreateTextureFromSurface(renderer, headSurface));
        SDL_SetTextureBlendMode(head->texture, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(head->texture, SDL_SCALEMODE_NEAREST);
        SDL_FRect drect = getDrect(head->x, head->y);
        SDL_RenderTexture(renderer, head->texture, NULL, &drect);

        // 绘制第二节蛇身，每次都要重新绘制因为第二节蛇身从蛇头变化得来
        auto curr = head->next;
        curr->setTexture(SDL_CreateTextureFromSurface(renderer, bodySurface));
        SDL_SetTextureBlendMode(curr->texture, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(curr->texture, SDL_SCALEMODE_NEAREST);
        drect = getDrect(curr->x, curr->y);
        SDL_RenderTexture(renderer, curr->texture, NULL, &drect);

        for (curr = curr->next; curr; curr = curr->next) {
            // 绘制蛇身，第二节以后的蛇身仅在初始化重绘即可
            if (curr->texture != nullptr){
                drect = getDrect(curr->x, curr->y);
                SDL_RenderTexture(renderer, curr->texture, NULL, &drect);
                continue;
            }
            auto temp = SDL_CreateTextureFromSurface(renderer, bodySurface);

            if (!temp) std::cerr << "Snake: Failed to create body texture: " << SDL_GetError() << std::endl;

            curr->setTexture(temp);
            SDL_SetTextureBlendMode(curr->texture, SDL_BLENDMODE_NONE);
            SDL_SetTextureScaleMode(curr->texture, SDL_SCALEMODE_NEAREST);
            drect = getDrect(curr->x, curr->y);
            SDL_RenderTexture(renderer, curr->texture, NULL, &drect);
        }
    }

    // void handleEvent(SDL_Event* event) {
    //     if (event->type == SDL_EVENT_KEY_DOWN) {
    //         switch (event->key.key) {
    //         case SDLK_UP:
    //             newDirection = NORTH;
    //             break;

    //         case SDLK_DOWN:
    //             newDirection = SOUTH;
    //             break;

    //         case SDLK_LEFT:
    //             newDirection = WEST;
    //             break;

    //         case SDLK_RIGHT:
    //             newDirection = EAST;
    //             break;

    //         }
    //     }
    // }

    void updateHead() {
        // 判断方向，为了防止操作太快导致出现调头现象，故将方向更新也独立于事件处理。
        switch (newDirection)
        {
        case NORTH:
            if(head->direction != SOUTH) head->direction = NORTH;
            break;

        case SOUTH:
            if(head->direction != NORTH) head->direction = SOUTH;
            break;
        
        case WEST:
            if(head->direction != EAST) head->direction = WEST;
            break;

        case EAST:
            if(head->direction != WEST) head->direction = EAST;
            break;
        }
        // 蛇身移动
        int newHeadX, newHeadY;
        snakePrevLocation(head, newHeadX, newHeadY);

        // 解开这个可以不死 //

        // if (!inGrid(newHeadX, newHeadY)){
        //     growing = true;
        //     return;
        // } 

        // --------------- //

        // std::cout << "Snake at: " << newHeadX << ", " << newHeadY << " direction: " << head->direction << std::endl;

        auto newHead = new SnakeData(newHeadX, newHeadY, head->direction);
        newHead->setNext(head);
        head->setPrev(newHead);
        head = newHead;
    }

    void updateTail() {
        // 蛇尾移动
        if (!growing){
            auto curr = tail;
            tail = tail->prev;
            tail->setNext(nullptr);
            curr->setPrev(nullptr);
            delete curr;
        }
        else {
            growing = false;
        }
    }

    void update() {
        updateHead();
        updateTail();
    }
};

class snake::Apple : public Renderable {
public:
    Apple(int grid_x, int grid_y): Renderable(grid_x, grid_y, 4, 4, snakeapple_pixel, 16) {
        std::cout << "Generate apple at: " << grid_x << ", " << grid_y << std::endl;
    }
    ~Apple() {
    }
};

class snake::Round {
private:
    std::string name;
    int score;
    //目前还没有用，用于调整游戏难度。
    int level;
    
    utils::Timer tickTimer;
    int TPS;

    Snake* snake = nullptr; //蛇
    std::vector<Apple*> apples; //苹果
    int appleCount = 0; //苹果数量

    std::set<std::pair<int, int>> collisionGrids; //发生碰撞的的格子(蛇身)
    std::set<std::pair<int, int>> occupiedGrids; //已经占用的格子(苹果)
    std::set<std::pair<int, int>> bounderyGrids; //边界格子

    bool isGameOver = false; //游戏是否应当结束，在update中更新
    bool isPaused = true; //游戏是否处于暂停状态

    bool snakeHidden = false; //蛇是否隐藏
    bool appleHidden = false; //苹果是否隐藏
    bool gridHidden = false; //格子是否隐藏

public:
    Round(std::string name, int level, int speed = 10): name(name), score(0), level(level), TPS(speed) {
        if (level == 1) {
            //设置随机数
            std::uniform_int_distribution<int> rng_loc(0 + 3, constants::GRID_NUMBER - 1 - 3);
            std::uniform_int_distribution<int> rng_dir(0, 3);

            snake = new Snake(rng_loc(utils::rng_loc), rng_loc(utils::rng_loc), 3, static_cast<Direction>(rng_dir(utils::rng_loc)));

            appleCount = 3;
            apples.push_back(new Apple(0,0));
            apples.push_back(new Apple(19,0));
            apples.push_back(new Apple(0,19));

            // 蛇身碰撞体积
            for (auto curr = snake->head; curr; curr = curr->next){
                collisionGrids.insert(std::make_pair(curr->x, curr->y));
            }
            // 围墙碰撞体积
            for (int i = 0; i < constants::GRID_NUMBER; i++){
                bounderyGrids.insert(std::make_pair(-1, i));
                bounderyGrids.insert(std::make_pair(i, -1));
                bounderyGrids.insert(std::make_pair(constants::GRID_NUMBER, i));
                bounderyGrids.insert(std::make_pair(i, constants::GRID_NUMBER));
            }

            for (auto apple : apples) {
                occupiedGrids.insert(std::make_pair(apple->grid_x, apple->grid_y));
            }
        }
    }
    ~Round() {
    }
    static void drawGrid(SDL_Renderer* renderer, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // 绘制水平线
        for (int i = 0; i <= constants::GRID_HEIGHT; i += constants::GRID_SIZE) {
            SDL_RenderLine(renderer, constants::GRID_X, constants::GRID_Y + i, constants::GRID_X + constants::GRID_WIDTH, constants::GRID_Y + i);
        }

        // 绘制垂直线
        for (int i = 0; i <= constants::GRID_WIDTH; i += constants::GRID_SIZE) {
            SDL_RenderLine(renderer, constants::GRID_X + i, constants::GRID_Y, constants::GRID_X + i, constants::GRID_Y + constants::GRID_HEIGHT);
        }
    }

    static void drawFrame(SDL_Renderer* renderer, SDL_Color color)  {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int i = 0; i < constants::FRAME_THICKNESS; i++) {
            SDL_FRect rect_frame = {float(constants::GRID_X - i), float(constants::GRID_Y - i), float(constants::GRID_WIDTH + 2 * i + 1), float(constants::GRID_HEIGHT + 2 * i + 1)};
            SDL_RenderRect(renderer, &rect_frame);
        }
    }
    void draw(SDL_Renderer* renderer){
        if (!gridHidden) {
            // grid background
            SDL_FRect rect = {constants::GRID_X, constants::GRID_Y, constants::GRID_WIDTH, constants::GRID_HEIGHT};
            SDL_SetRenderDrawColor(renderer, constants::color_gridbg.r, constants::color_gridbg.g, constants::color_gridbg.b, constants::color_gridbg.a);
            SDL_RenderFillRect(renderer, &rect);
            // grid line
            drawGrid(renderer, constants::color_gridline);
            // grid frame
            drawFrame(renderer, constants::color_frame);
        }

        if (!appleHidden) {
            for(auto apple : apples){
                apple->draw(renderer);
            }
        }

        if (!snakeHidden) {
            snake->draw(renderer);
        }
    }

    // 不需要handleEvent，event是可视化的内容，须在Round类外处理。
    // void handleEvent(SDL_Event* event) {
    //     if (event->type == SDL_EVENT_KEY_DOWN) {
    //         switch (event->key.key) {
    //         case SDLK_P:
    //             isPaused = !isPaused;
    //             break;
    //         }
    //     }
    //     snake->handleEvent(event);
    // }

    void update() {
        // std::cout << "Round update" << std::endl;

        // 处理暂停和结束
        if (isPaused || isGameOver) {
            // std::cout << "fuck 1" << std::endl; 
            return;
        }

        // 计时器
        if (tickTimer.elapsed() < 1000.0f / TPS) {
            // std::cout << "fuck 2 " << tickTimer.getStartTime() << " " << 
            // tickTimer.now() << " " << 
            // 1000.0f / TPS << std::endl; 
            return;
        }
        tickTimer.reset();

        // 更新蛇的位置
        if (!snake->growing) {
            collisionGrids.erase(std::make_pair(snake->tail->x, snake->tail->y));
        }
        snake->update();

        // 获取蛇头移动后的位置
        int headX = snake->head->x;
        int headY = snake->head->y;
        // std::cout << "Snake at: " << headX << ", " << headY << " direction: " << snake->head->direction << std::endl;

        // 死亡判定
        if (collisionGrids.find(std::make_pair(headX, headY))!= collisionGrids.end() ||
                bounderyGrids.find(std::make_pair(headX, headY))!= bounderyGrids.end()) {
            std::cout << "Game Over!" << std::endl;
            isGameOver = true;
        }

        collisionGrids.insert(std::make_pair(snake->head->x, snake->head->y));

        // printCollisionGrids();

        // 蛇吃到苹果
        for (auto& apple : apples) {
            if (headX == apple->grid_x && headY == apple->grid_y) {
                score += 1;
                occupiedGrids.erase(std::make_pair(apple->grid_x, apple->grid_y));
                // 重新生成苹果
                std::uniform_int_distribution<int> temp(0 + 1, constants::GRID_NUMBER - 1 - 1);
                delete apple;

                // 防止重复
                auto newX = temp(utils::rng_loc);
                auto newY = temp(utils::rng_loc);

                auto tempPair = std::make_pair(newX, newY);
                
                while (collisionGrids.find(tempPair) != collisionGrids.end() ||
                        occupiedGrids.find(tempPair) != occupiedGrids.end() ||
                        bounderyGrids.find(tempPair) != bounderyGrids.end()) {
                    tempPair = std::make_pair(temp(utils::rng_loc), temp(utils::rng_loc));
                }
                occupiedGrids.insert(std::make_pair(newX, newY));
                apple = new Apple(newX, newY);
                std::cout << "Generate apple at: " << apple->grid_x << ", " << apple->grid_y << std::endl;
                snake->growing = true;
                break;
            }
        }

        // 调整难度
        if (snake->growing && score % 5 == 0 && TPS < 20) {
            TPS += 1;
            std::cout << "Speed up to " << TPS << std::endl;
        }

        // printOccupiedGridsAndCollisionGrids();
    }
    
    const int getScore(){
        return score;
    }
    const int getLevel(){
        return level;
    }
    const int getSpeed(){
        return TPS;
    }
    const std::string getName(){
        return name;
    }
    const bool getIsGameOver(){
        return isGameOver;
    }
    const bool getIsPaused(){
        return isPaused;
    }
    void setIsPaused(const bool isPaused){
        this->isPaused = isPaused;
    }
    void togglePause(){
        isPaused = !isPaused;
        if (isPaused) {
            this->tickTimer.pause();
            std::cout << "Game Paused!" << std::endl;
        }
        else {
            this->tickTimer.resume();
            std::cout << "Game Resumed!" << std::endl;
        }
    }
    void toggleRestart(){
        isGameOver = false;
        isPaused = true;
        score = 0;
        delete snake;
        std::uniform_int_distribution<int> rng_loc(0 + 3, constants::GRID_NUMBER - 1 - 3);
        std::uniform_int_distribution<int> rng_dir(0, 3);

        occupiedGrids.clear();
        collisionGrids.clear();
        // bounderyGrids.clear();

        snake = new Snake(rng_loc(utils::rng_loc), rng_loc(utils::rng_loc), 3, static_cast<Direction>(rng_dir(utils::rng_loc)));
        // 蛇身碰撞体积
        for (auto curr = snake->head; curr; curr = curr->next){
            collisionGrids.insert(std::make_pair(curr->x, curr->y));
        }

        apples.clear();
        appleCount = 3;
        apples.push_back(new Apple(0,0));
        apples.push_back(new Apple(19,0));
        apples.push_back(new Apple(0,19));

        for (auto apple : apples) {
            occupiedGrids.insert(std::make_pair(apple->grid_x, apple->grid_y));
        }

        std::cout << "Game Restarted!" << std::endl;

        TPS = 5;

        tickTimer.reset();
    }

    void toggleHideSnake(){
        snakeHidden = !snakeHidden;
    }

    void toggleHideApple(){
        appleHidden = !appleHidden;
    }

    void toggleHideGrid(){
        gridHidden = !gridHidden;
    }

    void playerMove(const Direction direction){
        snake->newDirection = direction;
    }

    void printCollisionGrids() {
        std::cout << "Collision Grids: " << std::endl;
        for (auto grid : collisionGrids) {
            std::cout << "(" << grid.first << ", " << grid.second << ") " << std::endl;
        }
    }
};