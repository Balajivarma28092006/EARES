#include "energyPartitioning.h"
#include "config.h"
#include "deadline.h"
#include "workflowInit.h"

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

void EnergyPartitioning() {
  calculateRank();
  vector<int> taskList(N);
  iota(taskList.begin(), taskList.end(), 0);
  stable_sort(taskList.begin(), taskList.end(),
              [](int a, int b) { return UpWardRanks[a] > UpWardRanks[b]; });

  // calculate the preassigned energy for tasks
  // max energy consuption that is allowed
  // min energy consuption that is allowed
  // energy consuption level represents what portion of the average energy
  // consumption of task ti to the average energy consumption preassigned energy
  // consuption
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

  vector<int> vmOf(N, -1); // vm we assign
  // freq we assign
  // energy we assign
  // reliability we assign
  // start finish time finally Energy budget thats given
  // the time at which the current Vm becomes free
  vector<double> fOf(N, 0), Eact(N, 0), Ract(N, 0), ST(N, 0), FT(N, 0),
      Egiven(N, 0);
  vector<double> avail(M, 0);
  double used = 0;

  for (int pos = 0; pos < N; pos++) {
    int i = taskList[pos];
    double rest = 0;
    for (int q = pos + 1; q < N; q++)
      rest += Ep[taskList[q]];
    double Egv = E_GOAL - used - rest;
    Egiven[i] = Egv;

    // now we need to find the best reliability, energy, start, finish, and freq
    double bestR = 0, bestE = 0, bestST = 0, bestFT = 0, bestF = 0;
    int bestK = -1; // tracks the best VM

    for (int k = 0; k < M; k++) {
      for (int s = (int)ceil(Vms[k].f_low * 100 - 1e-9);
           s <= (int)Vms[k].f_max * 100; s++) {
        double f = (double)s / 100;
        double E = energy(k, i, f);
        if (E <= Egv) {
          double R = reliability(k, i, f);
          Ckpt c = checkpoints(k, i, f);
          double start = avail[k];
          for (int j = 0; j < N; j++) {
            // we need to select a task that is already pre assigned to a vm
            if (communicationCosts[j][i] > 0 && vmOf[j] != -1) {
              double CC = (vmOf[j] == k) ? 0 : communicationCosts[j][i];
              start = max(start, FT[j] + CC);
            }
          }
          double AFT = start + c.et;
          if (R > bestR && AFT < Deadlines[i]) {
            bestR = R;
            bestK = k;
            bestF = f;
            bestE = E;
            bestST = start;
            bestFT = AFT;
          }
        }
      }
    }
    if (bestK == -1) {
      cout << "No VM/frequency meets both the sub-deadline and the energy "
              "limit for t"
           << i + 1 << " (d=" << Deadlines[i] << ", Egiven=" << Egv << ")\n";
      return;
    }
    vmOf[i] = bestK;
    fOf[i] = bestF;
    Eact[i] = bestE;
    Ract[i] = bestR;
    ST[i] = bestST;
    FT[i] = bestFT;
    avail[bestK] =
        bestFT; // we need to chose the finish time of the current task
    used += bestE;
  }

  // finally calculate the R E and SL(makespan);
  // EG is already calculated using used
  double makespan = 0, RG = 1;
  for (int i = 0; i < N; i++) {
    makespan = max(makespan, FT[i]);
    RG *= Ract[i];
  }

  cout << fixed << setprecision(4);
  cout << "\nEnergy assignment and Vm assigning\n";
  cout << "Task\td(ti)\tEgiven\tvm\tf\tE(ti)\tR(ti)\tST(ti)\tFT(ti)\n";
  for (int i : taskList)
    cout << "t" << i + 1 << "\t" << setprecision(2) << Deadlines[i] << "\t"
         << setprecision(4) << Egiven[i] << "\tvm" << vmOf[i] + 1 << "\t"
         << setprecision(2) << fOf[i] << "\t" << setprecision(4) << Eact[i]
         << "\t" << Ract[i] << "\t" << setprecision(2) << ST[i] << "\t" << setprecision(4) << FT[i] << "\n";
  cout << "\nE(G) = " << used << "   R(G) = " << RG
       << "   SL(G) = " << setprecision(2) << makespan << "\n";
}
