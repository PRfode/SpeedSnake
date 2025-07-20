#pragma once

#include <SDL3/SDL.h>

namespace constants {
    constexpr int SNAKE_MOVE_INTERVAL = 500; // 毫秒

    // 窗口尺寸
    constexpr int WINDOW_WIDTH = 480;
    constexpr int WINDOW_HEIGHT = 640;
    
    // 网格尺寸
    constexpr int GRID_WIDTH = 360;
    constexpr int GRID_HEIGHT = 360;
    constexpr int GRID_NUMBER = 20;
    constexpr int GAP = 4;
    
    // 计算网格位置和大小
    constexpr int GRID_CENTER_X = WINDOW_WIDTH / 2;
    constexpr int GRID_CENTER_Y = WINDOW_HEIGHT / 2 + 60;
    constexpr int GRID_X = GRID_CENTER_X - GRID_WIDTH / 2;
    constexpr int GRID_Y = GRID_CENTER_Y - GRID_HEIGHT / 2;
    constexpr float GRID_SIZE = static_cast<float>(GRID_WIDTH) / GRID_NUMBER;
    
    // 边框厚度
    constexpr int FRAME_THICKNESS = 4;

    // FPS
    constexpr int FPS = 60;
    constexpr float FRAME_TIME = 1000.0f / FPS;

    constexpr SDL_Color color_bg = {16, 0, 32, 255}, color_gridbg = {32, 0, 32, 255},
        color_gridline = {32, 32, 32, 255}, color_frame = {188, 188, 188, 255},
        color_bt_frame = {255, 0, 0, 255}, color_bt_text = {255, 255, 255, 255};
}