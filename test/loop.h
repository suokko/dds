/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DTEST_LOOP_H
#define DTEST_LOOP_H

#include "dll.h"


void loop_solve(std::ostream &out,
  boardsPBN * bop,
  solvedBoards * solvedbdp,
  dealPBN * deal_list,
  futureTricks * fut_list,
  const int number,
  const int stepsize);

bool loop_calc(std::ostream &out,
  ddTableDealsPBN * dealsp,
  ddTablesRes * resp,
  allParResults * parp,
  dealPBN * deal_list,
  ddTableResults * table_list,
  const int number,
  const int stepsize);

bool loop_par(std::ostream &out,
  int * vul_list,
  ddTableResults * table_list,
  parResults * par_list,
  const int number,
  const int stepsize);

bool loop_dealerpar(std::ostream &out,
  int * dealer_list,
  int * vul_list,
  ddTableResults * table_list,
  parResultsDealer * dealerpar_list,
  const int number,
  const int stepsize);

bool loop_play(std::ostream &out,
  boardsPBN * bop,
  playTracesPBN * playsp,
  solvedPlays * solvedplp,
  dealPBN * deal_list,
  playTracePBN * play_list,
  solvedPlay * trace_list,
  const int number,
  const int stepsize);

#endif

