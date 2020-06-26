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
#include <array>

#include "dds.h"

#include "CalcTables.h"
#include "PlayAnalyser.h"
#include "SolveBoard.h"
#include "ThreadMgr.h"

using namespace std;

class Scheduler;

typedef void (*fptrType)(paramType &param, const int thid, Scheduler &scheduler);
typedef void (*fduplType)(
  const boards& bds, vector<int>& uniques, vector<int>& crossrefs);
typedef void (*fsingleType)(paramType &param, const int thid, const int bno);
typedef void (*fcopyType)(paramType &param, const vector<int>& crossrefs);


class System
{
  private:

    int numThreads;
    int sysMem_MB;

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

    int RunThreadsSTLAsync(paramType &param, RunMode runCat, Scheduler &scheduler);

    void WorkerSTLAsync(paramType &param, fptrType, Scheduler &);

    string GetVersion(
      int& major,
      int& minor,
      int& patch) const;
    string GetSystem(int& sys) const;
    string GetBits(int& bits) const;
    string GetCompiler(int& comp) const;
    string GetCores(int& comp) const;
    string GetConstructor(int& cons) const;
    string GetThreadSizes(char * c) const;


  public:
    System();

    ~System();

    void Reset();

    int RegisterParams(
      const int nThreads,
      const int mem_usable_MB);

    bool IsSingleThreaded() const;

    bool ThreadOK(const int thrId) const;

    void GetHardware(
      int& ncores,
      unsigned long long& kilobytesFree) const;

    int RunThreads(paramType &param,
        const RunMode r,
        const boards& bop);

    int RunThreads(playparamType &param,
        const RunMode r,
        const boards& bop,
        const playTracesBin& pl);

    string str(DDSInfo * info) const;
};

#endif

