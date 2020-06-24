/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <future>
#include <iostream>

#include "dll.h"
#include "testcommon.h"
#include "args.h"
#include "cst.h"

OptionsType options;


int main(int argc, char * argv[])
{
  ReadArgs(argc, argv);

  SetResources(options.memoryMB, options.numThreads);

  DDSInfo info;
  GetDDSInfo(&info);
  std::cout << info.systemString << std::endl;

  std::vector<std::future<int>> rv;
  size_t sz = options.fname.size()*options.solver.size();

  rv.reserve(sz);
  std::launch policy = sz == 1 ?
    std::launch::deferred :
    std::launch::deferred | std::launch::async;

  for (const std::string& fn: options.fname)
    for (const Solver& s: options.solver)
      rv.push_back(std::async(policy, realMain, std::ref(fn), s));

  int r = 0;
  for (std::future<int>& f: rv)
    r |= f.get();
  return r;
}
