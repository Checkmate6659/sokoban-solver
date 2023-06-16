#include "static_analysis.h"

uint8_t sa_temp[LEVEL_SIZE];
#define CLEAR_TEMP memset(sa_temp, 0, sizeof(sa_temp))

int dss_pushable_dfs(int node)
{
    if (level[node] == AIR || (level[node] & GOAL)) return 1; //Goal square reachable! Also, when computing dead squares, a square labeled as AIR is actually never dead
    sa_temp[node] = 1; //label node as discovered

    //this "dumb" scheme works much better than a "smarter" heuristic for some reason
    for (uint8_t i = 0; i < 4; i++)
    {
        if (0 < node + DIRECTIONS[i] && node + DIRECTIONS[i] < LEVEL_SIZE && //cannot go OOB!
            !sa_temp[node + DIRECTIONS[i]] && CAN_PUSH_STATIC(node, DIRECTIONS[i])) //and we have to check if its not already discovered AND its pushable
        {
            if (dss_pushable_dfs(node + DIRECTIONS[i])) return 1;
        }
    }

    return 0;
}

int dss_pushable_path_exist(int start)
{
    CLEAR_TEMP; //clear temporary array (and then use it for labeling discovered nodes)
    return dss_pushable_dfs(start);
}

//compute all dead squares in the level (could be optimized a LOT, because it takes a significant amount of time for larger levels)
void compute_dead_squares()
{
    //replace all air with dead squares
    for (int j = 0; j < LEVEL_SIZE; j++)
        if (level[j] == AIR)
            level[j] = DEAD;

    //find non-dead squares
    for (int i = 0; i < LEVEL_SIZE; i++)
    {
        if (level[i] == DEAD)
            if (dss_pushable_path_exist(i)) //if we can push box from our current square to the goal
                level[i] = AIR;
    }
}
