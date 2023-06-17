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

void reachable_area_flood_fill(uint16_t pos)
{
    if (reachable_area[pos]) return; //square already filled!
    reachable_area[pos] = 1; //fill current square
    if (pos < normalized_player_pos) normalized_player_pos = pos; //update normalized player position

    //fill adjacent squares recursively
    if (!(level[pos + 1] & BOX)) reachable_area_flood_fill(pos + 1);
    if (!(level[pos - 1] & BOX)) reachable_area_flood_fill(pos - 1);
    if (!(level[pos + LEVEL_WIDTH] & BOX)) reachable_area_flood_fill(pos + LEVEL_WIDTH);
    if (!(level[pos - LEVEL_WIDTH] & BOX)) reachable_area_flood_fill(pos - LEVEL_WIDTH);
}

void compute_reachable_area(uint16_t player_pos)
{
    memset(reachable_area, 0, sizeof(reachable_area)); //at first start at all zeros
    normalized_player_pos = player_pos;
    reachable_area_flood_fill(player_pos);
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

