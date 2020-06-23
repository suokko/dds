/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DDS_SYSTEM_H
#define DDS_SYSTEM_H

/*
   This class encapsulates all the system-dependent stuff.
 */

#include <string>
#include <vector>

#include "dds.h"

#include "CalcTables.h"
#include "PlayAnalyser.h"
#include "SolveBoard.h"
#include "ThreadMgr.h"

using namespace std;

class Scheduler;

typedef void (*fptrType)(const int thid, Scheduler &scheduler);
typedef void (*fduplType)(
  const boards& bds, vector<int>& uniques, vector<int>& crossrefs);
typedef void (*fsingleType)(const int thid, const int bno);
typedef void (*fcopyType)(const vector<int>& crossrefs);


class System
{
  private:

    int numThreads;
    int sysMem_MB;

    unsigned preferredSystem;

    vector<bool> availableSystem;

    static constexpr std::array<fptrType, 3> CallbackSimpleList = {
      SolveChunkCommon,
      CalcChunkCommon,
      PlayChunkCommon
    };
    static constexpr std::array<fduplType, 3> CallbackDuplList = {
      DetectSolveDuplicates,
      DetectCalcDuplicates,
      DetectPlayDuplicates
    };
    static constexpr std::array<fsingleType, 3> CallbackSingleList = {
      SolveSingleCommon,
      CalcSingleCommon,
      PlaySingleCommon
    };
    static constexpr std::array<fcopyType, 3> CallbackCopyList = {
      CopySolveSingle,
      CopyCalcSingle,
      CopyPlaySingle
    };

    ThreadMgr threadMgr;

    int RunThreadsBasic(RunMode runCat, Scheduler &scheduler);
    int RunThreadsBoost(RunMode runCat, Scheduler &scheduler);
    int RunThreadsOpenMP(RunMode runCat, Scheduler &scheduler);
    int RunThreadsGCD(RunMode runCat, Scheduler &scheduler);
    int RunThreadsWinAPI(RunMode runCat, Scheduler &scheduler);
    int RunThreadsSTL(RunMode runCat, Scheduler &scheduler);
    int RunThreadsSTLAsync(RunMode runCat, Scheduler &scheduler);
    int RunThreadsTBB(RunMode runCat, Scheduler &scheduler);
    int RunThreadsSTLIMPL(RunMode runCat, Scheduler &scheduler);
    int RunThreadsPPLIMPL(RunMode runCat, Scheduler &scheduler);

    typedef int (System::*RunPtr)(RunMode, Scheduler &);
    static constexpr std::array<RunPtr, 10> RunPtrList = {
      &System::RunThreadsBasic,
      &System::RunThreadsWinAPI,
      &System::RunThreadsOpenMP,
      &System::RunThreadsGCD,
      &System::RunThreadsBoost,
      &System::RunThreadsSTL,
      &System::RunThreadsTBB,
      &System::RunThreadsSTLIMPL,
      &System::RunThreadsPPLIMPL,
      &System::RunThreadsSTLAsync
    };

    void WorkerSTLAsync(fptrType, Scheduler &);

    string GetVersion(
      int& major,
      int& minor,
      int& patch) const;
    string GetSystem(int& sys) const;
    string GetBits(int& bits) const;
    string GetCompiler(int& comp) const;
    string GetCores(int& comp) const;
    string GetConstructor(int& cons) const;
    string GetThreading(int& thr) const;
    string GetThreadSizes(char * c) const;


  public:
    System();

    ~System();

    void Reset();

    int RegisterParams(
      const int nThreads,
      const int mem_usable_MB);

    bool IsSingleThreaded() const;

    bool IsIMPL() const;

    bool ThreadOK(const int thrId) const;

    void GetHardware(
      int& ncores,
      unsigned long long& kilobytesFree) const;

    int PreferThreading(const unsigned code);

    int RunThreads(
        const RunMode r,
        const boards& bop);

    int RunThreads(
        const RunMode r,
        const boards& bop,
        const playTracesBin& pl);

    string str(DDSInfo * info) const;
};

#endif

