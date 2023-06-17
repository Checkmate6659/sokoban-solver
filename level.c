#include "level.h"

//directions that boxes can be pushed along
const int16_t DIRECTIONS[4] = {1, LEVEL_WIDTH, -LEVEL_WIDTH, -1};

//the current level
Tile level[LEVEL_SIZE] = {};
uint16_t nboxes = 0;
uint64_t boxes_hash = 0;
uint8_t reachable_area[LEVEL_SIZE];
uint16_t normalized_player_pos;
uint64_t zobrist_keys[LEVEL_SIZE * 2]; //we need some zobrist keys (we have a bit more than required here)
uint16_t boxes[MAX_BOXES], goals[MAX_BOXES];

uint16_t fill_stack_index = 0;
uint16_t fill_stack[(LEVEL_WIDTH - 2) * (LEVEL_HEIGHT - 2)]; //flood fill stack (its probably way too large)

void flood_fill_scan(uint8_t lx, uint8_t rx, uint8_t y) //check for free space from lx (incl) to rx (not incl)
{
    uint8_t span_added = 0;
    for (uint8_t x = lx; x < rx; x++)
    {
        if (reachable_area[COORDINATES(rx, y)] || level[COORDINATES(x, y)] & BOX) //if obstacle
        {
            span_added = 0;
        }
        else if (!span_added)
        {
            fill_stack[fill_stack_index++] = COORDINATES(x, y);
            span_added = 1;
        }
    }
}

void compute_reachable_area(uint16_t player_pos)
{
    memset(reachable_area, 0, sizeof(reachable_area)); //at first start at all zeros
    normalized_player_pos = player_pos;

    fill_stack_index = 1; //clear stack and add only the start position
    fill_stack[0] = player_pos;
    while (fill_stack_index--) //while stack is not empty, loop starts with a pop
    {
        uint16_t pos = fill_stack[fill_stack_index];
        //update normalized player position (its needed here too)
        if (normalized_player_pos > pos) normalized_player_pos = pos;

        uint8_t y = Y_POSITION(pos); //thiese x and y position calculations should only be temporary
        uint8_t lx = X_POSITION(pos);
        uint8_t rx = lx;
        while (!reachable_area[COORDINATES(lx - 1, y)] && !(level[COORDINATES(lx - 1, y)] & BOX)) //while there is no obstacle, keep going left
        {
            --lx;
            reachable_area[COORDINATES(lx, y)] = 1;
            //update normalized player position (only needed on the LEFT! and the middle)
            if (normalized_player_pos > COORDINATES(lx, y)) normalized_player_pos = COORDINATES(lx, y);
        }
        while (!reachable_area[COORDINATES(rx, y)] && !(level[COORDINATES(rx, y)] & BOX)) //while there is no obstacle, keep going right
        {
            reachable_area[COORDINATES(rx, y)] = 1;
            ++rx;
        }

        flood_fill_scan(lx, rx, y + 1); //rx not included!
        flood_fill_scan(lx, rx, y - 1);
    }
}

void move(uint16_t box, int16_t dir)
{
    //move in box list
    for (uint16_t i = 0; i < nboxes; i++)
        if (boxes[i] == box)
        {
            boxes[i] += dir;
            break;
        }
    level[box] &= ~BOX; //be careful with goal squares
    level[box + dir] |= BOX;
    
    boxes_hash ^= zobrist_keys[box] ^ zobrist_keys[box + dir]; //update hash

    compute_reachable_area(box); //recompute player-reachable area
}

void unmove(uint16_t box, int16_t dir)
{
    //unmove in box list
    for (uint16_t i = 0; i < nboxes; i++)
        if (boxes[i] == box + dir)
        {
            boxes[i] -= dir;
            break;
        }
    level[box + dir] &= ~BOX; //be careful with goal squares
    level[box] |= BOX;

    boxes_hash ^= zobrist_keys[box] ^ zobrist_keys[box + dir]; //update hash

    compute_reachable_area(box - dir); //recompute player-reachable area (wondering if there is a more efficient way to do this, but this floodfill is pretty good)
}

