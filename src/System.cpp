/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <array>
#include <future>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>

#include "SolveBoard.h"
#include "CalcTables.h"
#include "PlayAnalyser.h"
#include "System.h"
#include "Memory.h"
#include "Scheduler.h"
#include "ThreadMgr.h"

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

extern Memory memory;

const std::array<std::string, 5> DDS_SYSTEM_PLATFORM =
{
  "",
  "Windows",
  "Cygwin",
  "Linux",
  "Apple"
};

const std::array<std::string, 5> DDS_SYSTEM_COMPILER =
{
  "",
  "Microsoft Visual C++",
  "MinGW",
  "GNU g++",
  "clang"
};

const std::array<std::string, 3> DDS_SYSTEM_CONSTRUCTOR =
{
  "",
  "DllMain",
  "Unix-style"
};

constexpr std::array<fptrType, 3> System::CallbackSimpleList;
constexpr std::array<fduplType, 3> System::CallbackDuplList;
constexpr std::array<fsingleType, 3> System::CallbackSingleList;
constexpr std::array<fcopyType, 3> System::CallbackCopyList;

System::System()
{
  System::Reset();
}


System::~System()
{
}


void System::Reset()
{
  numThreads = 0;
}


void System::GetHardware(
  int& ncores,
  unsigned long long& kilobytesFree) const
{
  kilobytesFree = 0;
  ncores = 1;
  (void) System::GetCores(ncores);

#if defined(_WIN32) || defined(__CYGWIN__)
  // Using GlobalMemoryStatusEx instead of GlobalMemoryStatus
  // was suggested by Lorne Anderson.
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatusEx(&statex);
  kilobytesFree = static_cast<unsigned long long>(
                    statex.ullTotalPhys / 1024);

  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return;
#endif

#ifdef __APPLE__
  // Using sysctl libc function instead of popen for a better reliability
  // Based on: https://stackoverflow.com/questions/30511579

  uint64_t mem;
  size_t len = sizeof(mem);
  int name[] = {CTL_HW, HW_USERMEM};

  if (sysctl(name, size(name), &mem, &len, NULL, 0)) {
    perror("sysctlbyname(hw.memsize) failed, fallback to 1GB memory:");
    kilobytesFree = 1024ULL * 1024ULL;
  } else {
    kilobytesFree = static_cast<unsigned long long>(mem / 1024);
  }
  return;
#endif

#ifdef __linux__
  // Use half of the physical memory
  unsigned long pages = sysconf (_SC_PHYS_PAGES);
  unsigned long pagesize = sysconf (_SC_PAGESIZE);
  if (pages > 0 && pagesize > 0)
    kilobytesFree = static_cast<unsigned long long>(pages * (pagesize / 1024));
  else
    kilobytesFree = 1024 * 1024; // guess 1GB

  return;
#endif
}


int System::RegisterParams(
  const int nThreads,
  const int mem_usable_MB)
{
  // No upper limit -- caveat emptor.
  if (nThreads < 1)
    return RETURN_THREAD_INDEX;

  // Make sure background threads have completed.
  threadMgr.Resize(nThreads, numThreads);
  numThreads = nThreads;
  sysMem_MB = mem_usable_MB;
  return RETURN_NO_FAULT;
}


bool System::IsSingleThreaded() const
{
  return false;
}

bool System::ThreadOK(const int thrId) const
{
  return (thrId >= 0 && thrId < numThreads);
}

//////////////////////////////////////////////////////////////////////
//                          Threading                               //
//////////////////////////////////////////////////////////////////////

void System::WorkerSTLAsync(paramType &param, fptrType fptr, Scheduler &scheduler)
{
  auto threadId = threadMgr.Occupy();
  fptr(param, threadId, scheduler);
}

int System::RunThreadsSTLAsync(paramType &param, RunMode runCat, Scheduler &scheduler)
{
  auto fptr = CallbackSimpleList[runCat];

  std::vector<std::future<void>> futures;
  std::vector<int> uniques;
  std::vector<int> crossrefs;
  (* CallbackDuplList[runCat])(*param.bop, uniques, crossrefs);

  const unsigned nu = static_cast<unsigned>(numThreads);
  futures.reserve(nu);
  // Launch first worker as deferred to allow it run in same thread
  futures.push_back(std::async(std::launch::deferred,
        &System::WorkerSTLAsync, this,
        std::ref(param), fptr, std::ref(scheduler)));
  for (unsigned k = 1; k < nu; k++)
    futures.push_back(std::async(&System::WorkerSTLAsync, this,
          std::ref(param), fptr, std::ref(scheduler)));

  for (auto& f: futures)
    f.wait();

  return RETURN_NO_FAULT;
}

int System::RunThreads(playparamType &param,
    RunMode runCat,
    const boards& bop,
    const playTracesBin& pl)
{
  Scheduler scheduler{numThreads, runCat, bop, pl};

  return RunThreadsSTLAsync(param, runCat, scheduler);
}

int System::RunThreads(paramType &param, RunMode runCat, const boards& bop)
{
  Scheduler scheduler{numThreads, runCat, bop};

  return RunThreadsSTLAsync(param, runCat, scheduler);
}


//////////////////////////////////////////////////////////////////////
//                     Self-identification                          //
//////////////////////////////////////////////////////////////////////

string System::GetVersion(
  int& major,
  int& minor,
  int& patch) const
{
  major = DDS_VERSION / 10000;
  minor = (DDS_VERSION - major * 10000) / 100;
  patch = DDS_VERSION % 100;

  string st = to_string(major) + "." + to_string(minor) +
    "." + to_string(patch);
  return st;
}


string System::GetSystem(int& sys) const
{
#if defined(_WIN32)
  sys = 1;
#elif defined(__CYGWIN__)
  sys = 2;
#elif defined(__linux)
  sys = 3;
#elif defined(__APPLE__)
  sys = 4;
#else
  sys = 0;
#endif

  return DDS_SYSTEM_PLATFORM[static_cast<unsigned>(sys)];
}


string System::GetBits(int& bits) const
{
#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4127)
#endif

  string st;
  if (sizeof(void *) == 4)
  {
    bits = 32;
    st = "32 bits";
  }
  else if (sizeof(void *) == 8)
  {
    bits = 64;
    st = "64 bits";
  }
  else
  {
    bits = 0;
    st = "unknown";
  }
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

  return st;
}


string System::GetCompiler(int& comp) const
{
#if defined(_MSC_VER)
  comp = 1;
#elif defined(__MINGW32__)
  comp = 2;
#elif defined(__clang__)
  comp = 4; // Out-of-order on purpose
#elif defined(__GNUC__)
  comp = 3;
#else
  comp = 0;
#endif

  return DDS_SYSTEM_COMPILER[static_cast<unsigned>(comp)];
}


string System::GetConstructor(int& cons) const
{
  cons = 0;

  return DDS_SYSTEM_CONSTRUCTOR[static_cast<unsigned>(cons)];
}


string System::GetCores(int& cores) const
{
#if defined(_WIN32) || defined(__CYGWIN__)
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  cores = static_cast<int>(sysinfo.dwNumberOfProcessors);
#elif defined(__APPLE__) || defined(__linux__)
  cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif

  // TODO Think about thread::hardware_concurrency().
  // This should be standard in C++11.

  return to_string(cores);
}


string System::GetThreadSizes(char * sizes) const
{
  int l = 0, s = 0;
  for (unsigned i = 0; i < static_cast<unsigned>(numThreads); i++)
  {
    if (memory.ThreadSize(i) == "S")
      s++;
    else
      l++;
  }

  const string st = to_string(s) + " S, " + to_string(l) + " L";
  strcpy(sizes, st.c_str());
  return st;
}


string System::str(DDSInfo * info) const
{
  stringstream ss;
  ss << "DDS DLL\n-------\n";

  const string strSystem = System::GetSystem(info->system);
  ss << left << setw(13) << "System" <<
    setw(20) << right << strSystem << "\n";

  const string strBits = System::GetBits(info->numBits);
  ss << left << setw(13) << "Word size" <<
    setw(20) << right << strBits << "\n";

  const string strCompiler = System::GetCompiler(info->compiler);
  ss << left << setw(13) << "Compiler" <<
    setw(20) << right << strCompiler << "\n";

  const string strConstructor = System::GetConstructor(info->constructor);
  ss << left << setw(13) << "Constructor" <<
    setw(20) << right << strConstructor << "\n";

  const string strVersion = System::GetVersion(info->major,
    info->minor, info->patch);
  ss << left << setw(13) << "Version" <<
    setw(20) << right << strVersion << "\n";
  strcpy(info->versionString, strVersion.c_str());

  ss << left << setw(17) << "Memory max (MB)" <<
    setw(16) << right << sysMem_MB << "\n";

  const string stm = to_string(THREADMEM_SMALL_DEF_MB) + "-" +
    to_string(THREADMEM_SMALL_MAX_MB) + " / " +
    to_string(THREADMEM_LARGE_DEF_MB) + "-" +
    to_string(THREADMEM_LARGE_MAX_MB);
  ss << left << setw(17) << "Threads (MB)" <<
    setw(16) << right << stm << "\n";

  System::GetCores(info->numCores);
  ss << left << setw(17) << "Number of cores" <<
    setw(16) << right << info->numCores << "\n";

  info->noOfThreads = numThreads;
  ss << left << setw(17) << "Number of threads" <<
    setw(16) << right << numThreads << "\n";

  const string strThrSizes = System::GetThreadSizes(info->threadSizes);
  ss << left << setw(13) << "Thread sizes" <<
    setw(20) << right << strThrSizes << "\n";

  const string st = ss.str();
  strcpy(info->systemString, st.c_str());
  return st;
}

