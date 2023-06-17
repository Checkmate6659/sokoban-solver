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

//BFS for initial lower bound search
typedef struct { int id; int distance; } IlbBfsNode;
int ilb_pushable_bfs(int start, int end)
{
    std::queue<IlbBfsNode> bfs_queue;
    sa_temp[start] = 1; //label node as discovered
    bfs_queue.push({start, 0});
    while (!bfs_queue.empty())
    {
        //get first node
        IlbBfsNode node = bfs_queue.front();
        bfs_queue.pop();
        if (node.id == end)
        {
            return node.distance; //goal found
        }
        for (uint8_t i = 0; i < 4; i++)
        {
            //we have to check if its not already discovered AND its pushable; and forbid deadlocks using dead squares
            if (!sa_temp[node.id + DIRECTIONS[i]] && CAN_PUSH_STATIC_DEADSQ(node.id, DIRECTIONS[i]))
            {
                sa_temp[node.id + DIRECTIONS[i]] = 1; //label as discovered
                bfs_queue.push({node.id + DIRECTIONS[i], node.distance + 1}); //enqueue (with extra distance)
            }
        }
    }
    //no path exists (should never happen, so this is in case of bug)
    return 0; //in this case it just returns 0 in order not to overestimate
}

//Initial lower bound
//For now it's the sum of lengths of shortest paths from each box to each goal (computed with lazy as hell bfs, and NOT dfs because that overestimates!)
//after that it will be the minimum weight perfect matching (using Hungarian algorithm because large box sizes)
uint32_t compute_initial_lower_bound()
{
    uint8_t distances[nboxes][nboxes]; //allocate it (and it will be freed automatically when compiling)
    std::vector<std::vector<int>> matrix; //for Hungarian algorithm (this matrix is gonna get messed up, so we need an original copy)

    //iterate through all box/goal pairs and compute the distances
    for (uint16_t i = 0; i < nboxes; i++)
    {
        matrix.push_back(std::vector<int>{});
        for (uint16_t j = 0; j < nboxes; j++)
        {
            CLEAR_TEMP; //clear temporary array (and then use it for labeling discovered nodes)
            distances[i][j] = ilb_pushable_bfs(boxes[i], goals[j]);
            // matrix[i][j] = distances[i][j];
            matrix.at(matrix.size() - 1).push_back(distances[i][j]);

            // printf("%d From %d To %d\n", distances[i][j], boxes[i], goals[j]);
        }
    }

    //do shortest path distance of each box to closest goal (TODO: MWPM)
    /* uint32_t total = 0;
    for (uint16_t i = 0; i < nboxes; i++)
    {
        uint8_t best_distance = LEVEL_WIDTH + LEVEL_HEIGHT;
        for (uint16_t j = 0; j < nboxes; j++)
        {
            uint8_t cur_distance = distances[i][j];
            if(cur_distance < best_distance) best_distance = cur_distance;
        }
        total += best_distance;
    }
    return total; */


    //compute the maximum weight bipartite matching using the Hungarian/Munkres Assignment algorithm
    //time complexity O(n³)

    // printf("RESULT %d\n", do_hungarian(matrix));
    return do_hungarian(matrix);
}
