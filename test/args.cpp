/* 
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund / 
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


// These functions parse the command line for options.


#include <array>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#include "args.h"
#include "cst.h"


extern OptionsType options;

struct optEntry
{
  std::string shortName;
  std::string longName;
  unsigned numArgs;
};

#define DTEST_NUM_OPTIONS 4

const std::array<optEntry, DTEST_NUM_OPTIONS> optList =
{
  optEntry{"f", "file", 1},
  optEntry{"s", "solver", 1},
  optEntry{"n", "numthr", 1},
  optEntry{"m", "memory", 1}
};

const std::array<std::string, 5> solverList =
{
  "solve",
  "calc",
  "play",
  "par",
  "dealerpar"
};

std::string shortOptsAll, shortOptsWithArg;

int GetNextArgToken(
  int argc,
  char * argv[]);

void SetDefaults();

bool ParseRound();


void Usage(
  const char base[])
{
  std::string basename(base);
  const size_t l = basename.find_last_of("\\/");
  if (l != std::string::npos)
    basename.erase(0, l+1);

  std::cout <<
    "Usage: " << basename << " [options]\n\n" <<
    "-f, --file s       Input file, or the number n;\n" <<
    "                   '100' means ../hands/list100.txt).\n" <<
    "                   (Default: input.txt)\n" <<
    "                   You can specify multiple to run cases in parallel.\n"
    "\n" <<
    "-s, --solver       One of: solve, calc, play, par, dealerpar.\n" <<
    "                   (Default: solve)\n" <<
    "                   You can specify multiple to run cases in parallel.\n"
    "\n" <<
    "-n, --numthr n     Maximum number of threads.\n" <<
    "                   (Default: 0 meaning that DDS decides)\n" <<
    "\n" <<
    "-m, --memory n     Total DDS memory size in MB.\n" <<
    "                   (Default: 0 meaning that DDS decides)\n" <<
    "\n" <<
    std::endl;
}


int nextToken = 1;
char * optarg;

int GetNextArgToken(
  int argc,
  char * argv[])
{
  // 0 means done, -1 means error.

  if (nextToken >= argc)
    return 0;

  std::string str(argv[nextToken]);
  if (str[0] != '-' || str.size() == 1)
    return -1;

  if (str[1] == '-')
  {
    if (str.size() == 2)
      return -1;
    str.erase(0, 2);
  }
  else if (str.size() == 2)
    str.erase(0, 1);
  else
    return -1;

  for (unsigned i = 0; i < DTEST_NUM_OPTIONS; i++)
  {
    if (str == optList[i].shortName || str == optList[i].longName)
    {
      if (optList[i].numArgs == 1)
      {
        if (nextToken+1 >= argc)
          return -1;

        optarg = argv[nextToken+1];
        nextToken += 2;
      }
      else
        nextToken++;

      return str[0];
    }
  }

  return -1;
}


void SetDefaults()
{
  options.numThreads = 0;
  options.memoryMB = 0;
}


void ReadArgs(
  int argc,
  char * argv[])
{
  for (unsigned i = 0; i < DTEST_NUM_OPTIONS; i++)
  {
    shortOptsAll += optList[i].shortName;
    if (optList[i].numArgs)
      shortOptsWithArg += optList[i].shortName;
  }

  if (argc == 1)
  {
    Usage(argv[0]);
    exit(0);
  }

  SetDefaults();

  int c, m = 0;
  bool errFlag = false, matchFlag;
  std::string stmp;
  char * ctmp;
  struct stat buffer;

  while ((c = GetNextArgToken(argc, argv)) > 0)
  {
    switch(c)
    {
      case 'f':
        if (stat(optarg, &buffer) == 0)
        {
          options.fname.push_back(optarg);
          break;
        }

        stmp = "../hands/list" + std::string(optarg) + ".txt";
        if (stat(stmp.c_str(), &buffer) == 0)
        {
          options.fname.push_back(stmp);
          break;
        }

        std::cout << "Input file '" << optarg << "' not found\n";
        std::cout << "Input file '" << stmp << "' not found\n";
        nextToken -= 2;
        errFlag = true;
        break;

      case 's':
        matchFlag = false;
        stmp = optarg;
        std::transform(stmp.begin(), stmp.end(), stmp.begin(), ::tolower);

        for (unsigned i = 0; i < DTEST_SOLVER_SIZE && ! matchFlag; i++)
        {
          std::string s = solverList[i];
          transform(s.begin(), s.end(), s.begin(), ::tolower);
          if (stmp == s)
          {
            m = static_cast<int>(i);
            matchFlag = true;
          }
        }

        if (matchFlag)
          options.solver.push_back(static_cast<Solver>(m));
        else
        {
          std::cout << "Solver '" << optarg << "' not found\n";
          nextToken -= 2;
          errFlag = true;
        }
        break;

      case 'n':
        m = static_cast<int>(strtol(optarg, &ctmp, 0));
        if (m < 0)
        {
          std::cout << "Number of threads must be >= 0\n\n";
          nextToken -= 2;
          errFlag = true;
        }
        options.numThreads = m;
        break;

      case 'm':
        m = static_cast<int>(strtol(optarg, &ctmp, 0));
        if (m < 0)
        {
          std::cout << "Memory in MB must be >= 0\n\n";
          nextToken -= 2;
          errFlag = true;
        }
        options.memoryMB = m;
        break;

      default:
        std::cout << "Unknown option\n";
        errFlag = true;
        break;
    }
    if (errFlag)
      break;
  }

  if (errFlag || c == -1)
  {
    std::cout << "Error while parsing option '" << argv[nextToken] << "'\n";
    std::cout << "Invoke the program without arguments for help" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (options.fname.empty())
    options.fname.push_back("input.txt");
  if (options.solver.empty())
    options.solver.push_back(DTEST_SOLVER_SOLVE);
}

