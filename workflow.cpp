#include "workflowInit.h"

// system configurations
const int bandwith = 1;
const double Oc = 0.1; // over head cost its 0.1 at one place and 0.5 at one's
const double Ku = 2;   // number of faults occurrences

/*
 * WCET represents the worst case execution times for the vm and tasks
 * these are given as say WCET[i][j] represents ith VM and jth task
 * so if i want 3rd vms task 9s wcet then it will WCET[2][8]
 */
double WCET[M][N] = {{14, 13, 11, 13, 12, 13, 7, 5, 18, 21},
                     {16, 19, 13, 8, 13, 16, 15, 11, 12, 7},
                     {9, 18, 19, 17, 10, 9, 11, 14, 20, 16}};

// average execution time ETAvg
double ETAvg[N];
// worst execution times of each when Ku faults occur
double ETworst[M][N];
// max ETworst
double maxETworst[N];
// Upward ranks for all the tasks
double UpWardRanks[N] = {}; // primarily everything is going to have ranks 0
// communication cost for each task with respect to each task
double communicationCosts[N][N];

struct Edge {
  int from, to;
  double weight;
};

vector<Edge> edges = {{0, 1, 18}, {0, 2, 12}, {0, 3, 9},  {0, 4, 11},
                      {0, 5, 14},

                      {1, 7, 19}, {1, 8, 16}, {2, 6, 23}, {3, 7, 27},
                      {3, 8, 23}, {4, 8, 13}, {5, 7, 15},

                      {6, 9, 17}, {7, 9, 11}, {8, 9, 13}};

// adjacency matrix for the given graph
vector<vector<pair<int, double>>> graph(N);

// number of check points thats needed
double calculateNumberofCheckPoints(double et) {
  double Nt = ceil((sqrt(Ku * et / Oc)) - 1);
  return Nt;
}

// best execution time
double calculateEtBest(double et, double Nt) { return et + (Nt * Oc); }

// length of checkpoints interval of a task denotes how
// often do we need the check point for this task as is denoted as L
double LengthOfCheckPoint(double et, double Nt) {
  double L = et / (Nt + 1);
  return L;
}

// the worst execution time includes all the check points length
// the final time is said to be all that overhead + best execution time
// this adds the Overhead for every fault occurrence and the time we waited for
// the check point it will be number of faults * waiting time
double calculateWorstExecutionTime(double et) {
  double Nt = calculateNumberofCheckPoints(et);
  double ETbest = calculateEtBest(et, Nt);
  double ETworst = ETbest + (Ku * LengthOfCheckPoint(et, Nt)) + (2 * Ku * Oc);
  return ETworst;
}

// upward rank is used to determine the order of task execution
void calculateRank() {
  for (int i = N - 1; i >= 0; i--) {
    double maxRank = 0;
    for (auto [next, weight] : graph[i]) {
      maxRank = max(maxRank, UpWardRanks[next] + communicationCosts[i][next]);
    }
    UpWardRanks[i] = maxRank + ETAvg[i];
  }
}

void initializeWorkFlow() {
  // build the dag
  for (auto e : edges) {
    graph[e.from].push_back({e.to, e.weight});
    communicationCosts[e.from][e.to] = e.weight / bandwith;
  }

  // calculate the average execution time of the task
  for (int i = 0; i < N; i++) {
    double sum = 0;
    for (int j = 0; j < M; j++) {
      sum += WCET[j][i];
    }
    ETAvg[i] = sum / M;
  }

  // calculate ETworst for all the tasks
  for (int i = 0; i < N; i++) {
    for (int k = 0; k < M; k++) {
      ETworst[k][i] = calculateWorstExecutionTime(WCET[k][i]);
    }
  }

  // calculate the worst Execution time for all the tasks
  for (int i = 0; i < N; i++) {
    double worstET = 0;
    for (int k = 0; k < M; k++) {
      worstET = max(worstET, ETworst[k][i]);
    }
    maxETworst[i] = worstET;
  }
}

void printAll() {
  calculateRank();
  vector<int> taskList(N);
  iota(taskList.begin(), taskList.end(), 0);

  sort(taskList.begin(), taskList.end(),
       [](int a, int b) { return UpWardRanks[a] > UpWardRanks[b]; });

  cout << fixed << setprecision(2);

  cout << "Task\tETAvg\tMaxETworst\tURank\n";

  for (int task : taskList) {
    cout << "t" << task + 1 << "\t" << ETAvg[task] << "\t" << maxETworst[task]
         << "\t\t" << UpWardRanks[task] << "\n";
  }

  cout << "\nTask scheduling order:\n";

  for (int task : taskList) {
    cout << "t" << task + 1 << " ";
  }
}
