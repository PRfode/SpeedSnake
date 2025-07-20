#include "utils.h"

void snake::testing()
{
    std::cout << "utils.cpp: Testing avaliability..." << std::endl;
}

void snake::snakePrevLocation(SnakeData* currData, int &prevX, int &prevY)
{
    switch (currData->direction){
        
    case NORTH:
        prevX = currData->x;
        prevY = currData->y - 1;
        break;
    
    case WEST:
        prevX = currData->x - 1;
        prevY = currData->y;
        break;
    
    case SOUTH:
        prevX = currData->x;
        prevY = currData->y + 1;
        break;
    
    case EAST:
        prevX = currData->x + 1;
        prevY = currData->y;
        break;
    }
}

const bool snake::inGrid(int x, int y)
{
    return (x >= 0 && x < constants::GRID_NUMBER && y >= 0 && y < constants::GRID_NUMBER);
}

SDL_FRect snake::getDrect(int grid_x, int grid_y)
{
    SDL_FRect drect({   (grid_x + 0.5f) * constants::GRID_SIZE - constants::GAP + constants::GRID_X, 
                        (grid_y + 0.5f) * constants::GRID_SIZE - constants::GAP + constants::GRID_Y, 
                        constants::GRID_SIZE - constants::GAP * 2, 
                        constants::GRID_SIZE - constants::GAP * 2 });
    return drect;
}

void utils::test_utils()
{
    std::cout << "utils.cpp: Testing utils..." << std::endl;
}

std::mt19937 utils::rng_loc(std::chrono::high_resolution_clock::now().time_since_epoch().count());
// std::random_device{}()

uint8_t snake::snakehead_pixel[4*4] = {255, 0, 0, 255,
                                        0, 255, 0, 255,
                                        0, 0, 255, 255,
                                        255, 255, 0, 255 };
uint8_t snake::snakebody_pixel[4*4] = {255, 255, 255, 255,
                                        255, 255, 255, 255,
                                        255, 255, 255, 255,
                                        255, 255, 255, 255 };
uint8_t snake::snakeapple_pixel[16*4] = {0, 0, 0, 0, 50, 20, 150, 255, 50, 20, 150, 255, 0, 0, 0, 0,
                                        50, 20, 150, 255, 50, 20, 150, 255, 50, 20, 150, 255, 50, 20, 150, 255,
                                        50, 20, 150, 255, 50, 20, 150, 255, 50, 20, 150, 255, 50, 20, 150, 255,
                                        0, 0, 0, 0, 50, 20, 150, 255, 50, 20, 150, 255, 0, 0, 0, 0 };
