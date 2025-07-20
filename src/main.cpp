#ifdef _WIN32
#include <windows.h>
#endif

#include "utils.h"
#include "constants.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <chrono>
#include <random>

// 使用高分辨率时钟
using Clock = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<double, std::milli>;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

//赖得改了awa
SDL_Color color_bg = constants::color_bg, color_gridbg = constants::color_gridbg,
        color_gridline = constants::color_gridline, color_frame = constants::color_frame,
        color_bt_frame = constants::color_bt_frame, color_bt_text = constants::color_bt_text;

bool ctn = true;

class Label {
protected:
    SDL_Surface* textSurface;
    std::string name;
    int x, y;
    SDL_Color textColor;
public:
    Label(int x, int y, std::string text, SDL_Color textColor, int textSize = 16, std::string name = ""): x(x), y(y) {
        if(name.empty()) this->name = text;
        this->textColor = textColor;
        TTF_Font* font = TTF_OpenFont("./assets/VonwaonBitmap-16px.ttf", textSize);
        if (!font) {
            std::cerr << "Failed to load font: " << SDL_GetError() << std::endl;
            return;
        }
        const char* c_text = text.c_str();
        int length = text.length();

        textSurface = TTF_RenderText_Blended(font, c_text, length, textColor);
        if (!textSurface) {
            std::cerr << "Failed to create font surface: " << SDL_GetError() << std::endl;
            TTF_CloseFont(font);
            return;
        }

        std::cout << "Label created: " << name << std::endl;
    }
    ~Label() {
        SDL_DestroySurface(textSurface);
        std::cout << "Label destroyed: " << name << std::endl;
    }
    void draw(SDL_Renderer* renderer) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (!textTexture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            return;
        }
        float centerX = x + textSurface->w / 2;
        float centerY = y + textSurface->h / 2;
        SDL_FRect dstrect = {float(centerX - textSurface->w / 2), float(centerY - textSurface->h / 2), float(textSurface->w), float(textSurface->h)};
        SDL_RenderTexture(renderer, textTexture, NULL, &dstrect);
        SDL_DestroyTexture(textTexture);
    }
    void setText(std::string text) {
        auto c_text = text.c_str();
        int length = text.length();
        TTF_Font* font = TTF_OpenFont("./assets/VonwaonBitmap-16px.ttf", 16);
        textSurface = TTF_RenderText_Blended(font, c_text, length, textColor);
        if (!textSurface) {
            std::cerr << "Failed to create font surface: " << SDL_GetError() << std::endl;
            return;
        }
    }
};

class CenteredLabel : public Label {
public:
    CenteredLabel(int centerX, int centerY, std::string text, SDL_Color textColor, int textSize = 16, std::string name = ""): Label(centerX, centerY, text, textColor, textSize, name) {
        std::cout << "Centered Label created: " << name << std::endl;
        this->x = centerX - textSurface->w / 2;
        this->y = centerY - textSurface->h / 2;
    }
};

void windowInit(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    snake::testing();
    //创建一个窗口
    window = SDL_CreateWindow("Speed Snake", constants::WINDOW_WIDTH, constants::WINDOW_HEIGHT, NULL);
    renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    
}

void windowDestroy(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void gameInit() {
    
}



void drawFont(SDL_Renderer* renderer, std::string text, int x, int y, int size, SDL_Color color) {
    TTF_Font* font = TTF_OpenFont("./assets/VonwaonBitmap-16px.ttf", size);
    if (!font) {
        std::cerr << "Failed to load font: " << SDL_GetError() << std::endl;
        return;
    }
    const char* c_text = text.c_str();
    int length = text.length();

    SDL_Surface* surface = TTF_RenderText_Blended(font, c_text, length, color);
    if (!surface) {
        std::cerr << "Failed to create font surface: " << SDL_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(surface);
        TTF_CloseFont(font);
        return;
    }
    SDL_FRect dstrect = {(float)x, (float)y, (float)surface->w, (float)surface->h};
    SDL_RenderTexture(renderer, texture, NULL, &dstrect);
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* argv[]){
    windowInit();
    //随机数初始化
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> rand_grid(0, constants::GRID_NUMBER - 1);

    // Button exitButton = Button(10, 10, 100, 30, "Exit", color_bt_frame, color_bt_text);
    // CenteredLabel exitLabel(50, 20, "Exit", color_bt_text, 16, "exitLabel");
    CenteredLabel titleLabel(constants::WINDOW_WIDTH / 2, 50, "Speed Snake", color_bt_text, 48, "titleLabel");

    CenteredLabel pauseLabel(constants::WINDOW_WIDTH / 2, constants::WINDOW_HEIGHT / 2, "Game Pause", {255, 255, 0, 255}, 48, "pauseLabel");
    CenteredLabel tipsLabel1(constants::WINDOW_WIDTH / 2, constants::WINDOW_HEIGHT / 2 + 300, "Press P to pause", {255, 255, 0, 255}, 24, "tipsLabel1");
    CenteredLabel tipsLabel2(constants::WINDOW_WIDTH / 2, constants::WINDOW_HEIGHT / 2 + 300, "Press P to continue", {255, 255, 0, 255}, 24, "tipsLabel2");
    CenteredLabel tipsLabel3(constants::WINDOW_WIDTH / 2, constants::WINDOW_HEIGHT / 2 + 268, "Press R to restart", {255, 0, 0, 255}, 24, "tipsLabel3");
    CenteredLabel gameOverLabel(constants::WINDOW_WIDTH / 2, constants::WINDOW_HEIGHT / 2, "Game Over", {255, 0, 0, 255}, 48, "gameOverLabel");

    snake::Round levelOne = snake::Round("Level 1", 1, 5);

    int lastScore = levelOne.getScore();
    int currScore;
    auto scoreText = "Score: " + std::to_string(lastScore);
    CenteredLabel* scoreLabelp = new CenteredLabel(constants::WINDOW_WIDTH / 2, 150, scoreText, color_bt_text, 24);
    utils::Timer Timer;

    while(ctn){
        SDL_Event event;
        auto stime = Clock::now();

        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                ctn = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (event.key.key) {
                case SDLK_ESCAPE:
                    ctn = false;
                    break;

                case SDLK_P:
                    // 暂停
                    levelOne.togglePause();
                    break;

                case SDLK_R:
                    // 重新开始
                    levelOne.toggleRestart();
                    break;

                case SDLK_UP:
                    levelOne.playerMove(snake::Direction::NORTH);
                    break;

                case SDLK_DOWN:
                    levelOne.playerMove(snake::Direction::SOUTH);
                    break;

                case SDLK_LEFT:
                    levelOne.playerMove(snake::Direction::WEST);
                    break;

                case SDLK_RIGHT:
                    levelOne.playerMove(snake::Direction::EAST);
                    break;

                default:
                    break;

                }
                break;

            default:
                break;
            }
        }

        levelOne.update();
        // 清屏
        SDL_SetRenderDrawColor(renderer, color_bg.r, color_bg.g, color_bg.b, color_bg.a);
        SDL_RenderClear(renderer);

        // 绘制内容
        // drawFont(renderer, "Snake", 200, 50, 32, {255, 255, 255, 255});
        titleLabel.draw(renderer);

        // exitLabel.draw(renderer);

        levelOne.draw(renderer);

        // score 的打印
        lastScore = currScore;
        currScore = levelOne.getScore();
        if (currScore != lastScore) {
            auto scoreText = "Score: " + std::to_string(currScore);
            delete scoreLabelp;
            scoreLabelp = new CenteredLabel(constants::WINDOW_WIDTH / 2, 150, scoreText, color_bt_text, 24);
        }
        scoreLabelp->draw(renderer);

        // tips 的打印
        tipsLabel3.draw(renderer);
        if (levelOne.getIsPaused()) tipsLabel2.draw(renderer);
        else tipsLabel1.draw(renderer);  

        // 显示暂停或游戏结束
        if (levelOne.getIsGameOver()) gameOverLabel.draw(renderer);
        else if (levelOne.getIsPaused()) pauseLabel.draw(renderer);

        // 帧率
        auto duration = Duration(Clock::now() - stime);
        auto delay = constants::FRAME_TIME - duration.count();

        if (delay > 0) {
            SDL_Delay(delay);
        }

        auto real_FPS = 1000.0 / Duration(Clock::now() - stime).count();
        //保留两位小数
        std::string fps_str = std::to_string(real_FPS);
        size_t pos = fps_str.find('.');
        if (pos == std::string::npos) {
            fps_str.append(".00");
        }
        else{
            fps_str.append(std::string(2, '0'));
            fps_str = fps_str.substr(0, pos + 3);
        }

        drawFont(renderer, "FPS: " + fps_str, 370, 10, 16, {255, 255, 255, 255});

        // 显示渲染内容
        SDL_RenderPresent(renderer);
    }
    windowDestroy();
    return 0;
}