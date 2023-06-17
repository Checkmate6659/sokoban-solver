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

//This is the absolute most voracious algorithm of the entire program
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

        uint16_t lpos = pos;
        uint16_t rpos = pos;
        while (!reachable_area[lpos - 1] && !(level[lpos - 1] & BOX)) //while there is no obstacle, keep going left
        {
            --lpos;
            reachable_area[lpos] = 1;
            //update normalized player position (only needed on the LEFT! and the middle)
            if (normalized_player_pos > lpos) normalized_player_pos = lpos;
        }
        while (!reachable_area[rpos] && !(level[rpos] & BOX)) //while there is no obstacle, keep going right
        {
            reachable_area[rpos] = 1;
            ++rpos;
        }

        uint8_t no_span_added_down = 1, no_span_added_up = 1;
        for (uint16_t i = lpos; i < rpos; i++)
        {
            //check scanline down
            if (reachable_area[i + LEVEL_WIDTH] || (level[i + LEVEL_WIDTH] & BOX)) //if obstacle down
                no_span_added_down = 1;
            else if (no_span_added_down)
            {
                fill_stack[fill_stack_index++] = i + LEVEL_WIDTH;
                no_span_added_down = 0;
            }
            //check scanline up
            if (reachable_area[i - LEVEL_WIDTH] || (level[i - LEVEL_WIDTH] & BOX)) //if obstacle up
                no_span_added_up = 1;
            else if (no_span_added_up)
            {
                fill_stack[fill_stack_index++] = i - LEVEL_WIDTH;
                no_span_added_up = 0;
            }
        }
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

    compute_reachable_area(box - dir); //recompute player-reachable area (TODO: try optimizing?)
}

