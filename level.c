#include "level.h"

//directions that boxes can be pushed along
const int16_t DIRECTIONS[4] = {1, LEVEL_WIDTH, -LEVEL_WIDTH, -1};

//the current level
Tile level[LEVEL_SIZE] = {};
uint16_t nboxes = 0;
uint8_t reachable_area[LEVEL_SIZE];
uint64_t zobrist_keys[LEVEL_SIZE * 2]; //we need some zobrist keys (we have a bit more than required here)
uint16_t boxes[MAX_BOXES], goals[MAX_BOXES];

void reachable_area_flood_fill(uint16_t pos)
{
    if (reachable_area[pos]) return; //square already filled!
    reachable_area[pos] = 1; //fill current square
    //fill adjacent squares recursively
    if (!(level[pos + 1] & BOX)) reachable_area_flood_fill(pos + 1);
    if (!(level[pos - 1] & BOX)) reachable_area_flood_fill(pos - 1);
    if (!(level[pos + LEVEL_WIDTH] & BOX)) reachable_area_flood_fill(pos + LEVEL_WIDTH);
    if (!(level[pos - LEVEL_WIDTH] & BOX)) reachable_area_flood_fill(pos - LEVEL_WIDTH);
}

void compute_reachable_area(uint16_t player_pos)
{
    memset(reachable_area, 0, sizeof(reachable_area)); //at first start at all zeros
    reachable_area_flood_fill(player_pos);
}

uint64_t zobrist_hash() //easiest and slowest method of zobrist hashing imaginable: just hash the entire board
{
    uint64_t hash = 0;
    uint16_t i = LEVEL_WIDTH + 1;
    uint8_t playerpos_todo = 1;
    for (uint16_t remaining_boxes = nboxes; remaining_boxes; i++) //we can prune the top and bottom edges
    {
        if (playerpos_todo && reachable_area[i])
        {
            hash ^= zobrist_keys[LEVEL_SIZE + i];
            playerpos_todo = 0;
        }
        if ((level[i] & 3) != BOX) continue; //not a box = not interesting
        hash ^= zobrist_keys[i];
        --remaining_boxes; //decrement the amount of remaining boxes
    }
    return hash;    
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
    compute_reachable_area(box - dir); //recompute player-reachable area (wondering if there is a more efficient way to do this, but this floodfill is pretty good)
}

