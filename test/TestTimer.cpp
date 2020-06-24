/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>

#include "TestTimer.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;


TestTimer::TestTimer()
{
  TestTimer::reset();
}


TestTimer::~TestTimer()
{
}


void TestTimer::reset()
{
  name = "";
  count = 0;
  userCum = 0;
  userCumOld = 0;
  sysCum = 0;
}


void TestTimer::setname(const std::string& s)
{
  name = s;
}


void TestTimer::start(const int number)
{
  count += number;
  user0 = Clock::now();
  sys0 = clock();
}


void TestTimer::end()
{
  time_point<Clock> user1 = Clock::now();
  clock_t sys1 = clock();

  std::chrono::duration<double, std::milli> d = user1 - user0;
  int tuser = static_cast<int>(1000. * d.count());

  userCum += tuser;
  sysCum += static_cast<int>((1000 * (sys1 - sys0)) /
    static_cast<double>(CLOCKS_PER_SEC));
}


void TestTimer::printRunning(std::ostream &out,
  const int reached,
  const int divisor)
{
  if (count == 0)
    return;

  out << std::setw(8) << reached << " (" <<
    std::setw(6) << std::setprecision(1) << std::right << std::fixed <<
      100. * reached /
        static_cast<float>(divisor) << "%)" <<
    std::setw(15) << std::right << std::fixed << std::setprecision(0) <<
      (userCum - userCumOld) / 1000. << std::endl;

  userCumOld = userCum;
}


void TestTimer::printBasic(std::ostream &out) const
{
  if (count == 0)
    return;

  if (name != "")
    out << std::setw(19) << std::left << "Timer name" << ": " << name << "\n";

  out << std::setw(19) << std::left << "Number of calls" << ": " << count << "\n";

  if (userCum == 0)
    out << std::setw(19) << std::left << "User time" << ": " << "zero" << "\n";
  else
  {
    out << std::setw(19) << std::left << "User time/ticks" << ": " <<
      userCum << "\n";
    out << std::setw(19) << std::left << "User per call" << ": " <<
      std::setprecision(2) << userCum / static_cast<float>(count) << "\n";
  }

  if (sysCum == 0)
    out << std::setw(19) << std::left << "Sys time" << ": " << "zero" << "\n";
  else
  {
    out << std::setw(19) << std::left << "Sys time/ticks" << ": " <<
      sysCum << "\n";
    out << std::setw(19) << std::left << "Sys per call" << ": " <<
      std::setprecision(2) << sysCum / static_cast<float>(count) << "\n";
    out << std::setw(19) << std::left << "Ratio" << ": " <<
      std::setprecision(2) << sysCum / static_cast<float>(userCum);
  }
  out << std::endl;
}


void TestTimer::printHands(std::ostream &out) const
{
  if (name != "")
    out << std::setw(21) << std::left << "Timer name" <<
      std::setw(12) << std::right << name << "\n";

  out << std::setw(21) << std::left << "Number of hands" <<
    std::setw(12) << std::right << count << "\n";

  if (count == 0)
    return;

  if (userCum == 0)
    out << std::setw(21) << std::left << "User time (ms)" <<
      std::setw(12) << std::right << "zero" << "\n";
  else
  {
    out << std::setw(21) << std::left << "User time (ms)" <<
      std::setw(12) << std::right << std::fixed <<
        std::setprecision(0) << userCum / 1000. << "\n";
    out << std::setw(21) << std::left << "Avg user time (ms)" <<
      std::setw(12) << std::right << std::fixed << std::setprecision(2) << userCum /
        static_cast<float>(1000. * count) << "\n";
  }

  if (sysCum == 0)
    out << std::setw(21) << std::left << "Sys time" <<
      std::setw(12) << std::right << "zero" << "\n";
  else
  {
    out << std::setw(21) << std::left << "Sys time (ms)" <<
      std::setw(12) << std::right << std::fixed << std::setprecision(0) << sysCum << "\n";
    out << std::setw(21) << std::left << "Avg sys time (ms)" <<
      std::setw(12) << std::right << std::fixed << std::setprecision(2) << sysCum /
        static_cast<float>(count) << "\n";
    out << std::setw(21) << std::left << "Ratio" <<
      std::setw(12) << std::right << std::fixed << std::setprecision(2) <<
      1000. * sysCum / static_cast<float>(userCum);
  }
  out << std::endl;
}

