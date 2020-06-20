/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <thread>

#include "ThreadMgr.h"


ThreadMgr::ThreadMgr(const unsigned nThreads) :
  realThreads{nThreads}
{
  for (unsigned i = 0; i < nThreads; i++)
    realThreads[i] = nThreads - i - 1;
}

unsigned ThreadMgr::Occupy()
{
  std::unique_lock<std::mutex> guard{mtx};
  cv.wait(guard, [&]() {return realThreads.size() > 0;});
  unsigned r = realThreads.back();
  realThreads.pop_back();
  return r;
}


void ThreadMgr::Release(const unsigned thrId)
{
  std::unique_lock<std::mutex> guard{mtx};
  realThreads.push_back(thrId);
  // If list was empty then wake up one sleeper
  if (realThreads.size() == 1)
    cv.notify_one();
}


void ThreadMgr::Print(
  const string& fname,
  const string& tag) const
{
  std::unique_lock<std::mutex> guard{mtx};
  ofstream fo(fname, std::ios_base::app);

  fo << tag <<
    ": Real threads occupied (out of " << realThreads.size() << "):\n";
  for (unsigned t: realThreads)
  {
      fo << t << endl;
  }
}

