
#ifndef CONFIG_H
#define CONFIG_H

#include <bits/stdc++.h>
using namespace std;

// System configuration
inline constexpr int N = 10;
inline constexpr int M = 3;

inline constexpr int bandwidth = 1;
inline constexpr double Oc = 0.1;
inline constexpr double Ku = 2;
inline constexpr double DEADLINE = 110.0;

// WCET
extern double WCET[M][N];

// Execution time calculations
extern double ETAvg[N];
extern double ETworst[M][N];
extern double maxETworst[N];
extern double ETbest[M][N];
extern double ETbestI[N];


// Upward ranks
extern double UpWardRanks[N];

// Communication costs
extern double communicationCosts[N][N];

// DAG edge
struct Edge {
    int from, to;
    double weight;
};

// DAG data
extern vector<Edge> edges;
extern vector<vector<pair<int, double>>> graph;

#endif
