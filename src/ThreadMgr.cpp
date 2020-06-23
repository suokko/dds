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


void ThreadMgr::Resize(unsigned nThreads, unsigned nOldThreads)
{
  std::unique_lock<std::mutex> guard{mtx};
  // Wait until all threads are free to avoid race condition with allocation
  cv.wait(guard, [&]() {
        if (realThreads.size() == nOldThreads)
          return true;
        // Make sure any other blocking thread gets woken up because this thread
        // can't wakeup yet.
        cv.notify_one();
        return false;
      });

  realThreads.resize(nThreads);

  for (unsigned i = 0; i < nThreads; i++)
    realThreads[i] = nThreads - i - 1;
}

ThreadMgr::ThreadId ThreadMgr::Occupy()
{
  std::unique_lock<std::mutex> guard{mtx};
  cv.wait(guard, [&]() {return realThreads.size() > 0;});
  unsigned r = realThreads.back();
  realThreads.pop_back();
  return {*this, r};
}


void ThreadMgr::Release(unsigned thrId)
{
  std::unique_lock<std::mutex> guard{mtx};
  realThreads.push_back(thrId);
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

ThreadMgr::ThreadId::ThreadId(ThreadMgr &p, unsigned i) :
  parent(p),
  id(i)
{
}

ThreadMgr::ThreadId::~ThreadId()
{
  parent.Release(id);
}

