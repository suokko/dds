/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <array>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>

#include "dll.h"
#include "portab.h"

#include "testcommon.h"
#include "TestTimer.h"
#include "parse.h"
#include "loop.h"
#include "compare.h"
#include "print.h"
#include "cst.h"

std::string GetSystem();
std::string GetBits();
std::string GetCompiler();


const std::array<std::string, 5> DDS_SYSTEM_PLATFORM =
{
  "",
  "Windows",
  "Cygwin",
  "Linux",
  "Apple"
};

const std::array<std::string, 5> DDS_SYSTEM_COMPILER =
{
  "",
  "Microsoft Visual C++",
  "MinGW",
  "GNU g++",
  "clang"
};


extern OptionsType options;
thread_local TestTimer timer;


void main_identify(std::ostream& ostream);

struct Flusher {
  std::ostringstream &stream;
  ~Flusher() {
    std::cout << stream.str();
  }
};

int realMain(const std::string& fname, Solver solver)
{
  bool GIBmode = false;

  std::ostringstream out;
  Flusher flusher{out};
  int stepsize = 0;
  if (solver == DTEST_SOLVER_SOLVE)
    stepsize = MAXNOOFBOARDS;
  else if (solver == DTEST_SOLVER_CALC)
    stepsize = MAXNOOFTABLES;
  else if (solver == DTEST_SOLVER_PLAY)
    stepsize = MAXNOOFBOARDS;
  else if (solver == DTEST_SOLVER_PAR)
    stepsize = 1;
  else if (solver == DTEST_SOLVER_DEALERPAR)
    stepsize = 1;

  main_identify(out);

  int number;
  int * dealer_list;
  int * vul_list;
  dealPBN * deal_list;
  futureTricks * fut_list;
  ddTableResults * table_list;
  parResults * par_list;
  parResultsDealer * dealerpar_list;
  playTracePBN * play_list;
  solvedPlay * trace_list;
  if (read_file(out, fname, number, GIBmode, &dealer_list, &vul_list,
        &deal_list, &fut_list, &table_list, &par_list, &dealerpar_list,
        &play_list, &trace_list) == false)
  {
    out << "read_file failed\n";
    return EXIT_FAILURE;
  }

  if (GIBmode && solver != DTEST_SOLVER_CALC)
  {
    out << "GIB file only works works with calc\n";
    return EXIT_FAILURE;
  }

  timer.reset();
  timer.setname("Hand stats");

  boardsPBN bop;
  solvedBoards solvedbdp;
  ddTableDealsPBN dealsp;
  ddTablesRes resp;
  allParResults parp;
  playTracesPBN playsp;
  solvedPlays solvedplp;

  switch (solver) {
  case DTEST_SOLVER_SOLVE:
    loop_solve(out, &bop, &solvedbdp, deal_list, fut_list, number, stepsize);
    break;
  case DTEST_SOLVER_CALC:
    loop_calc(out, &dealsp, &resp, &parp, deal_list, table_list,
        number, stepsize);
    break;
  case DTEST_SOLVER_PLAY:
    loop_play(out, &bop, &playsp, &solvedplp, deal_list, play_list, trace_list,
        number, stepsize);
    break;
  case DTEST_SOLVER_PAR:
    loop_par(out, vul_list, table_list, par_list, number, stepsize);
    break;
  case DTEST_SOLVER_DEALERPAR:
    loop_dealerpar(out, dealer_list, vul_list, table_list, dealerpar_list,
        number, stepsize);
    break;
  default:
    out << "Unknown type " <<
      static_cast<unsigned>(solver) << "\n";
    return EXIT_FAILURE;
  }

  timer.printHands(out);

  free(dealer_list);
  free(vul_list);
  free(deal_list);
  free(fut_list);
  free(table_list);
  free(par_list);
  free(dealerpar_list);
  free(play_list);
  free(trace_list);

  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////
//                     Self-identification                          //
//////////////////////////////////////////////////////////////////////

std::string GetSystem()
{
  unsigned sys;
#if defined(_WIN32)
  sys = 1;
#elif defined(__CYGWIN__)
  sys = 2;
#elif defined(__linux)
  sys = 3;
#elif defined(__APPLE__)
  sys = 4;
#else
  sys = 0;
#endif

  return DDS_SYSTEM_PLATFORM[sys];
}


std::string GetBits()
{
  if (sizeof(void *) == 4)
    return "32 bits";
  else if (sizeof(void *) == 8)
    return "64 bits";
  else
    return "unknown";
}


std::string GetCompiler()
{
  unsigned comp;
#if defined(_MSC_VER)
  comp = 1;
#elif defined(__MINGW32__)
  comp = 2;
#elif defined(__clang__)
  comp = 4; // Out-of-order on purpose
#elif defined(__GNUC__)
  comp = 3;
#else
  comp = 0;
#endif

  return DDS_SYSTEM_COMPILER[comp];
}


void main_identify(std::ostream &out)
{
  out << "test program\n";
  out << std::string(13, '-') << "\n";

  const std::string strSystem = GetSystem();
  out << std::left << std::setw(13) << "System" <<
    std::setw(20) << std::right << strSystem << "\n";

  const std::string strBits = GetBits();
  out << std::left << std::setw(13) << "Word size" <<
    std::setw(20) << std::right << strBits << "\n";

  const std::string strCompiler = GetCompiler();
  out << std::left << std::setw(13) << "Compiler" <<
    std::setw(20) << std::right << strCompiler << "\n\n";
}




