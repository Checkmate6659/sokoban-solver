#include "search.h"

uint64_t path_nodes[PATH_SIZE]; //simple hash table for current path
TTEntry tt[TT_SIZE]; //large transposition table
#define TT_STRATEGY_NAIVE //this strategy doesn't work on all levels, but its faster most of the time; also adding tt makes solver suboptimal


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

uint64_t nodes = 0;
uint32_t idastar_search(uint32_t bound)
{
    nodes++;

    //check if we won (O(n²) brute-force strategy)
    uint8_t boxesleft = nboxes;
    for (uint16_t i = 0; i < nboxes; i++)
        for (uint16_t j = 0; j < nboxes; j++)
            if (boxes[i] == goals[j]) boxesleft--;
    if (!boxesleft)
    {
        printf("NODES %lu\n", nodes);
        return PATH_FOUND;
    }
    
    uint32_t new_esti = PATH_NOT_FOUND;
    uint32_t new_bound = PATH_NOT_FOUND;
    //iterate through successors
    for (int i = 0; i < nboxes; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int dir = DIRECTIONS[j];
            int box = boxes[i];
            if (!reachable_area[box - dir] || !CAN_PUSH(box, dir)) continue; //can't push the box! so skip move

            move(box, dir);

            //do TT lookup
            uint64_t successor_hash = boxes_hash ^ zobrist_keys[LEVEL_SIZE + normalized_player_pos];
            uint64_t tt_index = successor_hash % TT_SIZE;
            if (tt[tt_index].key != successor_hash)
            {
                tt[tt_index].key = successor_hash; //always replace strategy
                tt[tt_index].cost = idastar_heuristic(); //TT stores heuristics
            }
            uint32_t lookup_result = tt[tt_index].cost;

            if (new_esti > lookup_result) new_esti = lookup_result;

            uint32_t pathnode_index = successor_hash % PATH_SIZE;
            if (path_nodes[pathnode_index] == successor_hash)
            {
                unmove(box, dir);
                continue;
            }
            path_nodes[pathnode_index] = successor_hash; //put current node in the path table

            uint32_t result = 1;
            if (1 + lookup_result <= bound)
                result += idastar_search(bound - 1); //no macromoves so far, so cost of each move is 1
            else
                result += lookup_result;
            unmove(box, dir); //boxes[i] gets modified, and so without this local var it wouldn't work!

            path_nodes[pathnode_index] -= 1; //and remove it after the search

            //TODO: record moves of solution!
            if (result == PATH_FOUND + 1)
            {
                // printf("%d %d\n", box, dir); //printing the solution backwards
                return PATH_FOUND; //make sure to do after undoing, otherwise its gonna screw up the level
            }
            if (result < new_bound) new_bound = result;
        }
    }
    new_esti++;

    uint64_t current_hash = boxes_hash ^ zobrist_keys[LEVEL_SIZE + normalized_player_pos];
    uint64_t tt_index = current_hash % TT_SIZE;
    tt[tt_index].key = current_hash;
#ifdef TT_STRATEGY_NAIVE
    tt[tt_index].cost = new_bound; //naive alg: really fast, but suboptimal
#else
    tt[tt_index].cost = (new_bound < new_esti) ? new_bound : new_esti; //take the minimum of both; more solid, but slower (many more nodes!) and still suboptimal
#endif

    return new_bound;
}

