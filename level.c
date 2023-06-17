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

#define FLOODFILL_TABLE_SIZE 1024
typedef struct //1 of these is 968 bytes (quite a lot) because I'm storing all these booleans inefficiently
{
    uint64_t hash;
    // uint8_t data[LEVEL_SIZE - 2*LEVEL_WIDTH - 2];
    uint8_t data[LEVEL_SIZE];
    uint16_t player_pos;
} FloodFillResult;
FloodFillResult floodfill_table[FLOODFILL_TABLE_SIZE]; //this table is almost 1MB with just 1k entries!

//This is the absolute most voracious algorithm of the entire program (TODO: optimize!)
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
    //store floodfill result in hash table
    uint64_t hash = boxes_hash ^ zobrist_keys[LEVEL_SIZE + box] ^ dir; //not quite the real hash based on player pos, and not all equivalent positions have same hash, but should be good enough
    uint16_t floodfill_table_index = hash % FLOODFILL_TABLE_SIZE;
    if (floodfill_table[floodfill_table_index].hash != hash)
    {
        //always replace
        floodfill_table[floodfill_table_index].hash = hash;
        // memcpy(floodfill_table[floodfill_table_index].data, reachable_area + LEVEL_WIDTH + 1, LEVEL_SIZE - 2*LEVEL_WIDTH - 2);
        memcpy(floodfill_table[floodfill_table_index].data, reachable_area, LEVEL_SIZE);
        floodfill_table[floodfill_table_index].player_pos = normalized_player_pos;
    }

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

    //idk why but trying to grab from hash table is very bad
    compute_reachable_area(box - dir); //compute player-reachable area
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

    //grab from hash table if we can
    uint64_t hash = boxes_hash ^ zobrist_keys[LEVEL_SIZE + box] ^ dir;
    uint16_t floodfill_table_index = hash % FLOODFILL_TABLE_SIZE;
    if (floodfill_table[floodfill_table_index].hash == hash) //if we found a match
    {
        //then just YOINK it from the table!
        // memcpy(reachable_area + LEVEL_WIDTH + 1, floodfill_table[floodfill_table_index].data, LEVEL_SIZE - 2*LEVEL_WIDTH - 2);
        memcpy(reachable_area, floodfill_table[floodfill_table_index].data, LEVEL_SIZE);
        normalized_player_pos = floodfill_table[floodfill_table_index].player_pos;
        // floodfill_table[floodfill_table_index].hash = hash - 1;
    }
    else
    {
        compute_reachable_area(box - dir); //recompute player-reachable area (only if needed)
    }
}

