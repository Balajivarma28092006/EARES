
#include "config.h"

double WCET[M][N] = {
    {14, 13, 11, 13, 12, 13, 7, 5, 18, 21},
    {16, 19, 13, 8, 13, 16, 15, 11, 12, 7},
    {9, 18, 19, 17, 10, 9, 11, 14, 20, 16}
};

double ETAvg[N] = {};
double ETworst[M][N] = {};
double maxETworst[N] = {};

double ETbest[M][N] = {};
double ETbestI[N] = {};
double UpWardRanks[N] = {};
double communicationCosts[N][N] = {};

vector<Edge> edges = {
    {0, 1, 18}, {0, 2, 12}, {0, 3, 9},
    {0, 4, 11}, {0, 5, 14},

    {1, 7, 19}, {1, 8, 16}, {2, 6, 23},
    {3, 7, 27}, {3, 8, 23}, {4, 8, 13},
    {5, 7, 15},

    {6, 9, 17}, {7, 9, 11}, {8, 9, 13}
};

vector<vector<pair<int, double>>> graph(N);
