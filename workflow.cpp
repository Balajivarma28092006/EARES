#include "config.h"
#include "workflowInit.h"

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
    communicationCosts[e.from][e.to] = e.weight / bandwidth;
  }

  // calculate the average execution time of the task
  for (int i = 0; i < N; i++) {
    double sum = 0;
    for (int j = 0; j < M; j++) {
      sum += WCET[j][i];
    }
    ETAvg[i] = sum / M;
  }

  // calculate ETbest for all tasks
  for(int i = 0; i < N; i++){
      for(int k = 0; k < M; k++){
          double Nt = calculateNumberofCheckPoints(WCET[k][i]);
          ETbest[k][i] = calculateEtBest(WCET[k][i], Nt);
      }
  }

  // calculate the ETbest(i) the paper doesnt provide any explicit formula so we are taking the average
  for(int i = 0; i < N; i++){
      double sum = 0;
      for(int k = 0; k < M; k++){
          sum += ETbest[k][i];
      }
      ETbestI[i] = sum / M;
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

  stable_sort(taskList.begin(), taskList.end(),
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
