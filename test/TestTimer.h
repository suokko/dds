/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DTEST_TESTTIMER_H
#define DTEST_TESTTIMER_H

#include <string>
#include <chrono>

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;


class TestTimer
{
  private:
    std::string name;
    long count;
    long userCum;
    long userCumOld;
    long sysCum;

    time_point<Clock> user0;
    clock_t sys0;

  public:

    TestTimer();
    ~TestTimer();

    void reset();

    void setname(const std::string& s);

    void start(const int number = 1);
    void end();

    void printRunning(std::ostream &out,
      const int reached,
      const int number);
    void printBasic(std::ostream &out) const;
    void printHands(std::ostream &out) const;
};

#endif
