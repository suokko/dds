/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DDS_THREADMGR_H
#define DDS_THREADMGR_H

#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std;


class ThreadMgr
{
  private:

    vector<unsigned> realThreads;
    mutable std::mutex mtx;
    std::condition_variable cv;

  public:

    ThreadMgr(const unsigned nThreads);

    unsigned Occupy();

    void Release(const unsigned thrId);

    void Print(
      const string& fname,
      const string& tag) const;
};

#endif
