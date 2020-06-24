/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DDS_SOLVEBOARD_H
#define DDS_SOLVEBOARD_H

#include <vector>

#include "dds.h"

using namespace std;

class Scheduler;

void SolveSingleCommon(paramType &param,
  const int thrId,
  const int bno);

void CopySolveSingle(paramType &param,
  const vector<int>& crossrefs);

void SolveChunkCommon(paramType &param,
  const int thrId,
  Scheduler &scheduler);

void DetectSolveDuplicates(
  const boards& bds,
  vector<int>& uniques,
  vector<int>& crossrefs);

#endif
