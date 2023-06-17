#include <stdio.h>
#include <string.h>
#include <time.h>
#include "level.h"
#include "static_analysis.h"
#include "search.h"

#define LEVEL_PATH "levels/levels.txt" //the path from which the levels shall be loaded

//TILE_CONVERSION[symbol] == tile
const Tile TILE_CONVERSION[] = {[' '] = AIR, ['@'] = AIR, ['#'] = WALL, ['$'] = BOX, ['.'] = GOAL, ['+'] = GOAL, ['*'] = BOXONGOAL};
const char TILE_PRINT_CONVERSION[] = " $-#.*"; //converting back when printing

int load_level(const int LEVEL_NUMBER)
{
    FILE *fptr = fopen(LEVEL_PATH, "r"); //WARNING! safety stuff (but I don't care, this is just for testing)
    if (!fptr)
    {
        printf("ERROR: Unable to load levels\nThe file levels.txt doesn't exist");
        return 1;
    }

    memset(level, AIR, sizeof(level)); //replace AIR with WALL to get walls filling up the space on the right and below the level
    nboxes = 0;

    // jump to the level number we want
    if (LEVEL_NUMBER) //otherwise we skip the first character of the first level
    {
        char current = fgetc(fptr);
        for (int i = 0; i < LEVEL_NUMBER; i++)
        {
            char lastchar = 0;
            while (current != '\n' || lastchar != '\n') //keep going until we have 2 newlines next to one another (WARNING when using CRLF encoding)
            {
                lastchar = current;
                current = fgetc(fptr);
            }
        }
    }

    char current_line[LEVEL_WIDTH];
    memset(current_line, '#', LEVEL_WIDTH);
    int row = 0, ngoals_temp = 0;
    int player_pos = LEVEL_WIDTH + 1; //just safety, not needed for well-conditioned levels
    while(fgets(current_line, LEVEL_WIDTH, fptr)) //while there are still lines to read in the level (NOTE: fgets only reads one line)
    {
        if(current_line[0] == '\n') break;
        for (int i = 0; i < LEVEL_WIDTH; i++)
        {
            if (current_line[i] == '\n') break;
            // printf("%d %d %c %d\n", row, i, current_line[i], current_line[i]);
            level[row + i] = TILE_CONVERSION[(uint8_t)current_line[i]];
            if ((level[row + i] & 3) == BOX)
            {
                boxes[nboxes] = row + i;
                nboxes++;
            }
            if (level[row + i] & GOAL)
            {
                goals[ngoals_temp] = row + i;
                ngoals_temp++;
            }
            if (current_line[i] == '@' || current_line[i] == '+') player_pos = row + i;
        }
        memset(current_line, '#', LEVEL_WIDTH);
        row += LEVEL_WIDTH;
    }

    compute_reachable_area(player_pos); //compute area reachable by the player
    fclose(fptr); //don't forget closing the file!
    return 0;
}

void print_level()
{
    int i = 0;
    int onlywalls = 1, onlyair = 1, onlydead = 1;
    while (i < LEVEL_SIZE)
    {
        char current = TILE_PRINT_CONVERSION[level[i]];
        if (reachable_area[i])
        {
            current = '@';
            if (level[i] & GOAL) current = '+';
        }
        putchar(current);
        //update printing end flags
        if (current != ' ') onlyair = 0;
        if (current != '-') onlydead = 0;
        if (current != '#') onlywalls = 0;

        if ((++i) % LEVEL_WIDTH == 0)
        {
            putchar('\n');
            if (onlyair | onlywalls | onlydead) return; //level ended with air line or wall line (possible when filling up empty spaces around level with walls), newline required
            onlyair = onlywalls = onlydead = 1; //reset flags
        }
    }
}

int main()
{
    //initialize zobrist keys (randomly) and clear tt (in case of unluckily placed garbage)
    for (int i = 0; i < LEVEL_SIZE * 2; i++) zobrist_keys[i] = pseudo_rng();
    for (unsigned long i = 0; i < PATH_SIZE; i++) path_nodes[i] = i - 1; //make all the path keys invalid (unobtainable)
    for (unsigned long i = 0; i < TT_SIZE; i++) tt[i].key = i - 1; //make all the keys invalid (unobtainable)

    //Load levels
    //69 is like 1 second with basic search, 109 is much easier; 152 is a very hard level for this kind of program
    //105 in a bit less than 1s, 104 in like 4s
    load_level(104);
    compute_dead_squares();

    print_level();
    printf("%d boxes in level\n", nboxes);
    for (int i = 0; i < nboxes; i++) printf("Box %d at %d\n", i, boxes[i]);
    for (int i = 0; i < nboxes; i++) printf("Goal %d at %d\n", i, goals[i]);

    clock_t timer = -clock();
    uint32_t bound = idastar_heuristic();
    while (bound != PATH_FOUND && bound != PATH_NOT_FOUND)
    {
        bound = idastar_search(bound);
    }
    timer += clock();
    printf("RETURN %u IN TIME %lf\n", bound, timer/(double)CLOCKS_PER_SEC);
}

