#include "search.h"




uint32_t idastar_heuristic()
{
    //just do manhattan distance of each box to closest goal
    uint32_t total = 0;
    for (uint16_t i = 0; i < nboxes; i++)
    {
        uint8_t best_distance = LEVEL_WIDTH + LEVEL_HEIGHT;
        for (uint16_t j = 0; j < nboxes; j++)
        {
            uint16_t offset = abs(goals[j] - boxes[i]); //orientation doesn't matter, so ABS!
            uint8_t cur_distance = (offset / LEVEL_WIDTH) + (offset % LEVEL_WIDTH);
            if(cur_distance < best_distance) best_distance = cur_distance;
        }
        total += best_distance;
    }
    return total;
}

std::set<uint64_t> tt;
uint32_t idastar_search(uint32_t g, uint32_t bound)
{
    // uint64_t hash = zobrist_hash();
    // if (tt.find(hash) != tt.end()) return PATH_NOT_FOUND; //TEMP, use hashtable later
    // tt.emplace(hash);

    //check if we won (O(n²) brute-force strategy)
    uint8_t boxesleft = nboxes;
    for (uint16_t i = 0; i < nboxes; i++)
        for (uint16_t j = 0; j < nboxes; j++)
            if (boxes[i] == goals[j]) boxesleft--;
    if (!boxesleft) return PATH_FOUND;

    uint32_t f = g + idastar_heuristic(); //for some reason when these 2 lines are at the front (as in wikipedia) it doesn't find solution
    if (f > bound) return f;
    
    uint32_t min_cost = PATH_NOT_FOUND;
    //iterate through successors
    for (int i = 0; i < nboxes; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int dir = DIRECTIONS[j];
            int box = boxes[i];
            if (!reachable_area[box - dir] || !CAN_PUSH(box, dir)) continue; //can't push the box! so skip move

            move(box, dir);

            uint64_t hash = zobrist_hash();//TEMP, use hashtable later
            if (tt.find(hash) != tt.end())
            {
                unmove(box, dir);
                continue;
            }
            tt.emplace(hash); //TMP

            uint32_t result = idastar_search(g + 1, bound); //no macromoves so far, so cost of each move is 1
            unmove(box, dir); //boxes[i] gets modified, and so without this local var it wouldn't work!

            tt.erase(hash); //TMP

            //TODO: record moves of solution!
            if (result == PATH_FOUND)
            {
                printf("%d %d\n", box, dir);
                return PATH_FOUND; //make sure to do after undoing, otherwise its gonna screw up the level
            }
            if (result < min_cost) min_cost = result;
        }
    }
    return min_cost;
}

