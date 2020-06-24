/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DDS_CALCTABLES_H
#define DDS_CALCTABLES_H

#include <vector>

#include "dds.h"

using namespace std;

class Scheduler;

void CalcSingleCommon(paramType &cparam,
  const int thrID,
  const int bno);

void CopyCalcSingle(paramType &cparam,
  const vector<int>& crossrefs);

void CalcChunkCommon(paramType &cparam,
  const int thrId,
  Scheduler &scheduler);

void DetectCalcDuplicates(
  const boards& bds,
  vector<int>& uniques,
  vector<int>& crossrefs);

#endif
