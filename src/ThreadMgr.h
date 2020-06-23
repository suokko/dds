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

    void Release(unsigned thrId);

  public:

    void Resize(unsigned nThreads, unsigned nOldThreads);

    class ThreadId {
      public:
        friend class ThreadMgr;
        ~ThreadId();

        operator unsigned() const { return id; }

      private:
        ThreadId(ThreadMgr &parent, unsigned id);

        ThreadMgr &parent;
        unsigned id;
    };

    ThreadId Occupy();

    void Print(
      const string& fname,
      const string& tag) const;

};

#endif
