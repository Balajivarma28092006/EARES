#include "deadline.h"
#include "config.h"
#include <climits>

// deadlines store for every task
double Deadlines[N] = {};
// Earliest Execution start time means how quick a task or a process can start
// this EST depends on the execution times of the predecessors
double EST[N] = {};
// Latest Finish Time is the absolute last time that a process can take if
// moves past then the total will be pushed back
double LFT[N] = {};
// Expected execution time EET
double EET[N] = {};
// note the tasks that are in the critcal path
vector<bool> isCp(N, false);
// keep another vector to check if its marked or not
vector<bool> marked(N, false);
vector<int> CriticalPath;
double CriticalPathLength = 0;

// this ensures that every pred is processed before the current task
vector<int> getTopoOrder() {
  vector<int> indegree(N, 0);
  for (int i = 0; i < N; i++) {
    for (auto [next, _] : graph[i]) {
      indegree[next]++;
    }
  }

  queue<int> q;
  for (int i = 0; i < N; i++) {
    if (!indegree[i])
      q.push(i);
  }
  vector<int> topoOrder;
  while (!q.empty()) {
    auto node = q.front();
    q.pop();
    topoOrder.push_back(node);
    for (auto child : graph[node]) {
      indegree[child.first]--;
      if (!indegree[child.first]) {
        q.push(child.first);
      }
    }
  }
  return topoOrder;
}

double ScalingFactor(){
    double SC = 0;
    double avgET = 0;
    for(int i: CriticalPath){
        avgET += ETAvg[i];
    }
    SC = ((LFT[N - 1] - EST[0]) - CriticalPathLength) / avgET;
    return SC;
}

void assignCriticalPathDeadlines(){
    int entry = 0, exit = N - 1;
    double SCALE = ScalingFactor();
    // calculate the EET for all tasks in the critical path
    for(int task: CriticalPath){
        EET[task] = ETAvg[task] + (
            ScalingFactor() * ETAvg[task]
        );
    }

    for(int i = 0; i < N; i++){
        if(!isCp[i]) continue;

        // recalculate the EST of the current task using the EET
          if (entry == i) {
            EST[i] = 0;
          } else {
            double maxTime = 0;
            // finding all the preds
            for (int j = 0; j < N; j++) {
              if (communicationCosts[j][i] > 0) {
                double time = EST[j] + EET[i] + communicationCosts[j][i];
                maxTime = max(maxTime, time);
              }
            }
            EST[i] = maxTime;
        }
        Deadlines[i] = (i == exit) ? DEADLINE: EST[i] + EET[i];
        LFT[i] = Deadlines[i];
        if(i == exit){
            EET[i] = DEADLINE - EST[i];
        }
        marked[i] = true;
    }

    // now we need to go for unmarked things and recalculate the things
    for(int idx = N - 1; idx >= 0; idx--){
        auto tasks = getTopoOrder();
        int task = tasks[idx];

        if(isCp[task]){
            continue;
        }

        // update the LFT
        double minLFT = LLONG_MAX;
        bool hasSuccessor = false;

        for(auto [next, weight]: graph[task]){
            hasSuccessor = true;
            double midLFT = LFT[next] - EET[next] - communicationCosts[task][next];
            minLFT = min(minLFT, midLFT);
        }

        if(hasSuccessor){
            LFT[task] = minLFT;
        }else{
            LFT[task] = DEADLINE;
        }
        Deadlines[task] = LFT[task];
        EET[task] = LFT[task] - EST[task];
        marked[task] = true;
    }
}

void deadlineParition() {
  CriticalPathLength = 0;
  // first get the order of the tasks so tasks with less pred's come first
  vector<int> topoOrder = getTopoOrder();
  // assuming that the entry task is 0 and exit task is N-1
  int entry = 0, exit = N - 1;
  for (int i : topoOrder) {
    if (entry == i) {
      EST[i] = 0;
      LFT[i] = DEADLINE;
    } else {
      double maxTime = 0;
      // finding all the preds
      for (int j = 0; j < N; j++) {
        if (communicationCosts[j][i] > 0) {
          double time = EST[j] + ETAvg[j] + communicationCosts[j][i];
          maxTime = max(maxTime, time);
        }
      }
      EST[i] = maxTime;
      if(i != exit) LFT[i] = EST[i] + ETAvg[i];
      else LFT[i] = DEADLINE;
    }
  }

  CriticalPath.clear();
  int current = exit; // start form the end
  CriticalPath.push_back(current);
  isCp[current] = true;

  while (current != entry) {
    int selectedPred = -1; // keeps track of the largest ones LFT
    double maxLFT = -1;

    for (int j = 0; j < N; j++) {
      if (communicationCosts[j][current] > 0) {
        if (LFT[j] > maxLFT) {
          maxLFT = LFT[j];
          selectedPred = j;
        }
      }
    }
    if (selectedPred == -1) {
      throw runtime_error("Critical path not found");
    }
    current = selectedPred; // start form where we left;
    CriticalPath.push_back(current);
    isCp[current] = true;
  }

  reverse(CriticalPath.begin(), CriticalPath.end());
  for (int i = 0; i < CriticalPath.size(); i++) {
    int task = CriticalPath[i];
    CriticalPathLength += ETAvg[task];
    if (i + 1 < CriticalPath.size()) {
      int next = CriticalPath[i + 1];
      CriticalPathLength += communicationCosts[task][next];
    }
  }

  // update the EST for all the tasks using EET value of the task CP instead of ETbest
  assignCriticalPathDeadlines();
}


void printDeadlines() {
  cout << fixed << setprecision(2);

  cout << "\nCritical Path: ";
  for (int task : CriticalPath)
    cout << "t" << task + 1 << " ";

  cout << "\nCritical Path Length: " << CriticalPathLength << "\n";

  cout << "\n\nTask\tEST\tLFT\tEET\tDeadline\n";

  for (int i = 0; i < N; i++) {
    cout << "t" << i + 1 << "\t" << EST[i] << "\t" << LFT[i]
         << "\t" << EET[i] << "\t" << Deadlines[i] << "\n";
  }

  cout << "\n";
  double sc = ScalingFactor();
    cout << "Scaling factor: " << sc << "\n";
}
