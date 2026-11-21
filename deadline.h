#ifndef DEADLINE_H
#define DEADLINE_H

#include "config.h"

// Deadline-related global variables
extern double Deadlines[N];
extern double EST[N];
extern double LFT[N];
extern vector<int> CriticalPath;
extern double CriticalPathLength;

// Deadline functions
vector<int> getTopoOrder();

void calculateESTLFT();
void deadlineParition();
void printDeadlines();

#endif
