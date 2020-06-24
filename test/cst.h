/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DTEST_CST_H
#define DTEST_CST_H

#include <string>
#include <vector>


enum Solver
{
  DTEST_SOLVER_SOLVE = 0,
  DTEST_SOLVER_CALC = 1,
  DTEST_SOLVER_PLAY = 2,
  DTEST_SOLVER_PAR = 3,
  DTEST_SOLVER_DEALERPAR = 4,
  DTEST_SOLVER_SIZE = 5
};

struct OptionsType
{
  std::vector<std::string> fname;
  std::vector<Solver> solver;
  int numThreads;
  int memoryMB;
};

#endif

