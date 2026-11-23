#ifndef HYBRID_SCHEDULER_H
#define HYBRID_SCHEDULER_H

#include "config.h"
#include "deadline.h"

enum class RankingMethod {
    UpWardRankingMethod,
    OCTRankingMethod
};

struct Result {
    bool completedAllTasks = false;
    string failedTask;
    double totalEnergy = 0;
    double workflowReliability = 0;
    double makespan = 0;
    int NoTasksNeedFallback = 0;
};

Result runOCTScheduler(RankingMethod method, bool allowMinimumReserveFallback, bool printTaskTable);

#endif
