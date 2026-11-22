#include "workflowInit.h"
#include "deadline.h"
#include "energyPartitioning.h"

int main() {
    initializeWorkFlow();
    printAll();
    deadlineParition();
    printDeadlines();
    EnergyPartitioning();
    return 0;
}
