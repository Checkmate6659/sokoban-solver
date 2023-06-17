#include "util.h"

uint64_t prng_state = 1234567890123456789; //a very uninteresting and boring seed (no idea if it is any good for zobrist keys tho)
uint64_t pseudo_rng() //a quick and dirty pseudorandom integer generator (xorshift64)
{
    prng_state ^= prng_state << 13;
    prng_state ^= prng_state >> 7;
    prng_state ^= prng_state << 17;

    return prng_state;
}
//NOTE using seed 0x123456789ABCDEF: there is something very weird that happens more times than expected: when
//viewing the decimal expansions of the hashes, usually there is at least 1 or 2 digits that are not present at all



//Hungarian algorithm for minimum perfect matching
//TODO: find a more optimized version (?) in C or C++
/**
 * Solution to https://open.kattis.com/problems/cordonbleu using Hungarian
 * algorithm.
 */

using namespace std;

/**
 * Sets a = min(a, b)
 * @return true if b < a
 */
template <class T> bool ckmin(T &a, const T &b) { return b < a ? a = b, 1 : 0; }

/**
 * Given J jobs and W workers (J <= W), computes the minimum cost to assign each
 * prefix of jobs to distinct workers.
 *
 * @tparam T a type large enough to represent integers on the order of J *
 * max(|C|)
 * @param C a matrix of dimensions JxW such that C[j][w] = cost to assign j-th
 * job to w-th worker (possibly negative)
 *
 * @return a vector of length J, with the j-th entry equaling the minimum cost
 * to assign the first (j+1) jobs to distinct workers
 */
template <class T> vector<T> hungarian(const vector<vector<T>> &C) {
    const int J = (int)C.size(), W = (int)C[0].size();
    // job[w] = job assigned to w-th worker, or -1 if no job assigned
    // note: a W-th worker was added for convenience
    vector<int> job(W + 1, -1);
    vector<T> ys(J), yt(W + 1);  // potentials
    // -yt[W] will equal the sum of all deltas
    vector<T> answers;
    const T inf = numeric_limits<T>::max();
    for (int j_cur = 0; j_cur < J; ++j_cur) {  // assign j_cur-th job
        int w_cur = W;
        job[w_cur] = j_cur;
        // min reduced cost over edges from Z to worker w
        vector<T> min_to(W + 1, inf);
        vector<int> prv(W + 1, -1);  // previous worker on alternating path
        vector<bool> in_Z(W + 1);    // whether worker is in Z
        while (job[w_cur] != -1) {   // runs at most j_cur + 1 times
            in_Z[w_cur] = true;
            const int j = job[w_cur];
            T delta = inf;
            int w_next;
            for (int w = 0; w < W; ++w) {
                if (!in_Z[w]) {
                    if (ckmin(min_to[w], C[j][w] - ys[j] - yt[w]))
                        prv[w] = w_cur;
                    if (ckmin(delta, min_to[w])) w_next = w;
                }
            }
            // delta will always be non-negative,
            // except possibly during the first time this loop runs
            // if any entries of C[j_cur] are negative
            for (int w = 0; w <= W; ++w) {
                if (in_Z[w]) ys[job[w]] += delta, yt[w] -= delta;
                else min_to[w] -= delta;
            }
            w_cur = w_next;
        }
        // update assignments along alternating path
        for (int w; w_cur != -1; w_cur = w) job[w_cur] = job[w = prv[w_cur]];
        answers.push_back(-yt[W]);
    }
    return answers;
}

/**
 * Sanity check: https://en.wikipedia.org/wiki/Hungarian_algorithm#Example
 * First job (5):
 *   clean bathroom: Bob -> 5
 * First + second jobs (9):
 *   clean bathroom: Bob -> 5
 *   sweep floors: Alice -> 4
 * First + second + third jobs (15):
 *   clean bathroom: Alice -> 8
 *   sweep floors: Dora -> 4
 *   wash windows: Bob -> 3
 */
// void sanity_check_hungarian() {
//     vector<vector<int>> costs{{8, 5, 9}, {4, 2, 4}, {7, 3, 8}};
//     assert((hungarian(costs) == vector<int>{5, 9, 15}));
//     cerr << "Sanity check passed.\n";
// }

//just a simple utility function, because using the raw function directly doesn't do anything good
int do_hungarian(vector<vector<int>> costs)
{
    vector<int> result = hungarian(costs);
    return result.at(result.size() - 1);
}

