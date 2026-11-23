#include "hybridScheduler.h"
#include "config.h"
#include "deadline.h"
#include "workflowInit.h"
#include <algorithm>
#include <limits>

struct VmPower {
  double Pind, Cef, m, f_low, f_max;
};

const VmPower Vms[M] = {{0.03, 0.80, 2.90, 0.26, 1.00},
                        {0.04, 0.70, 2.50, 0.26, 1.00},
                        {0.07, 1.00, 2.50, 0.29, 1.00}};

// these two numbers are never given in the paper so we decided to take
// CALIBRATED values
const double LAMBDAO = 2.63e-3;
const double D_SENS = 1.55;

const double E_GOAL = 80.0; // energy goal

// equation 4 for a frequency
double execTime(int k, int i, double f) { return WCET[k][i] / f; }

// equation 7 for the energy
double energy(int k, int i, double f) {
  return (Vms[k].Pind + Vms[k].Cef * pow(f, Vms[k].m)) * execTime(k, i, f);
}

struct Ckpt {
  double et, n, etBest, L, etWorst;
};
Ckpt checkpoints(int k, int i, double f) { // Eq. 10-13
  Ckpt c;
  c.et = execTime(k, i, f);
  c.n = ceil(sqrt(Ku / Oc * c.et) - 1);
  c.etBest = c.et + c.n * Oc;
  c.L = c.et / (c.n + 1);
  c.etWorst = c.etBest + Ku * c.L + 2 * Ku * Oc;
  return c;
}

double reliability(int k, int i, double f) { // Eq. 14-17
  Ckpt c = checkpoints(k, i, f);
  double lambda =
      LAMBDAO * pow(10.0, D_SENS * (Vms[k].f_max - f) /
                              (Vms[k].f_max - Vms[k].f_low)); // Eq. 15
  double R = 0;
  for (int g = 0; g <= (int)Ku; g++) {
    double x = lambda * c.etWorst;
    double Pg =
        exp(-x) * pow(x, g) / tgamma(g + 1); // just calcs factorial // Eq. 14
    R += Pg * exp(-lambda * c.L);            // Eq. 16, 17
  }
  return R; // reliability for all the g transient faults
}

// task ordering according to the oct
vector<vector<double>> computeOCT() {
  vector<vector<double>> OCT(N, vector<double>(M, 0));
  vector<int> topoOrder = getTopoOrder();

  reverse(topoOrder.begin(), topoOrder.end());
  for (int task : topoOrder) {
    if (graph[task].empty())
      continue; // no childrens
    for (int current_vm = 0; current_vm < M; current_vm++) {
      double maxChildCost = 0;
      for (auto [child, weight] : graph[task]) {
        double minCost = numeric_limits<double>::infinity();
        for (int child_vm = 0; child_vm < M; child_vm++) {
          double com =
              (current_vm == child_vm) ? 0.0 : communicationCosts[task][child];
          double cost = com + WCET[child_vm][child] + OCT[child_vm][child];
          minCost = min(minCost, cost);
        }
        maxChildCost = max(maxChildCost, minCost);
      }
      OCT[task][current_vm] = maxChildCost;
    }
  }
  return OCT;
}

vector<double> computeOCTRanks() {
  auto OCT = computeOCT();
  vector<double> ranks(N, 0);

  for (int task = 0; task < N; task++) {
    for (int vm = 0; vm < M; vm++) {
      ranks[task] += OCT[task][vm];
    }
    ranks[task] /= M;
  }
  return ranks;
}

// scheduler
struct ChosenOption {
  bool found = false; // found a choice to make
  int vmIndex = -1;
  double frequency = 0;
  double energy = 0;
  double reliability = 0;
  double startTime = 0;
  double finishTime = 0;
};

Result runOCTScheduler(RankingMethod method, bool allowMinimumReserveFallback, bool printTaskTable){
    Result result;
    calculateRank();

    // lets make a simple choice on what algo we are trying to implement and check
    vector<double> rankOfTasks(N);
    if(method == RankingMethod::OCTRankingMethod){
        rankOfTasks = computeOCTRanks();
    }else{
        for(int task = 0; task < N; task++){
            rankOfTasks[task] = UpWardRanks[task];
        }
    }

    vector<int> SchedulingOrder(N);
    iota(SchedulingOrder.begin(), SchedulingOrder.end(), 0);
    stable_sort(SchedulingOrder.begin(), SchedulingOrder.begin(), [&](int a, int b){
        return rankOfTasks[a] > rankOfTasks[b] + 1e-6;
    });

    vector<double> Emax(N), Emin(N), e(N), Ep(N);
    double EmaxG = 0, EminG = 0;
    for (int i = 0; i < N; i++) {
      Emax[i] = 0;
      Emin[i] = LLONG_MAX;
      for (int k = 0; k < M; k++) {
        Emax[i] = max(Emax[i], energy(k, i, Vms[k].f_max));
        Emin[i] = min(Emin[i], energy(k, i, Vms[k].f_low));
      }
      EmaxG += Emax[i];
      EminG += Emin[i];
    }

    for (int i = 0; i < N; i++) {
      e[i] = (Emin[i] + Emax[i]) / (EmaxG + EminG);
      Ep[i] = min(Emax[i], (E_GOAL - EminG) * e[i] + Emin[i]);
    }


}
