/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <array>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>

#include "SolveBoard.h"
#include "CalcTables.h"
#include "PlayAnalyser.h"
#include "parallel.h"
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

#define DDS_SYSTEM_THREAD_BASIC 0
#define DDS_SYSTEM_THREAD_WINAPI 1
#define DDS_SYSTEM_THREAD_OPENMP 2
#define DDS_SYSTEM_THREAD_GCD 3
#define DDS_SYSTEM_THREAD_BOOST 4
#define DDS_SYSTEM_THREAD_STL 5
#define DDS_SYSTEM_THREAD_TBB 6
#define DDS_SYSTEM_THREAD_STLIMPL 7
#define DDS_SYSTEM_THREAD_PPLIMPL 8
#define DDS_SYSTEM_THREAD_STLASYNC 9
#define DDS_SYSTEM_THREAD_SIZE 10

const std::array<std::string, DDS_SYSTEM_THREAD_SIZE> DDS_SYSTEM_THREADING =
{
  "None",
  "Windows",
  "OpenMP",
  "GCD",
  "Boost",
  "STL",
  "TBB",
  "STL-impl",
  "PPL-impl",
  "STL-async"
};

constexpr std::array<fptrType, 3> System::CallbackSimpleList;
constexpr std::array<fduplType, 3> System::CallbackDuplList;
constexpr std::array<fsingleType, 3> System::CallbackSingleList;
constexpr std::array<fcopyType, 3> System::CallbackCopyList;
constexpr std::array<System::RunPtr, 10> System::RunPtrList;

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
  preferredSystem = DDS_SYSTEM_THREAD_BASIC;

  availableSystem.resize(DDS_SYSTEM_THREAD_SIZE);
  availableSystem[DDS_SYSTEM_THREAD_BASIC] = true;
  for (unsigned i = 1; i < DDS_SYSTEM_THREAD_SIZE; i++)
    availableSystem[i] = false;

#ifdef DDS_THREADS_WINAPI
  availableSystem[DDS_SYSTEM_THREAD_WINAPI] = true;
#endif

#ifdef DDS_THREADS_OPENMP
  availableSystem[DDS_SYSTEM_THREAD_OPENMP] = true;
#endif

#ifdef DDS_THREADS_GCD
  availableSystem[DDS_SYSTEM_THREAD_GCD] = true;
#endif

#ifdef DDS_THREADS_BOOST
  availableSystem[DDS_SYSTEM_THREAD_BOOST] = true;
#endif

#ifdef DDS_THREADS_STL
  availableSystem[DDS_SYSTEM_THREAD_STL] = true;
#endif

#ifdef DDS_THREADS_STLASYNC
  availableSystem[DDS_SYSTEM_THREAD_STLASYNC] = true;
#endif

#ifdef DDS_THREADS_TBB
  availableSystem[DDS_SYSTEM_THREAD_TBB] = true;
#endif

#ifdef DDS_THREADS_STLIMPL
  availableSystem[DDS_SYSTEM_THREAD_STLIMPL] = true;
#endif

#ifdef DDS_THREADS_PPLIMPL
  availableSystem[DDS_SYSTEM_THREAD_PPLIMPL] = true;
#endif

  // Take the first of any multi-threading system defined.
  for (unsigned k = 1; k < availableSystem.size(); k++)
  {
    if (availableSystem[k])
    {
      preferredSystem = k;
      break;
    }
  }
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
  return (preferredSystem == DDS_SYSTEM_THREAD_BASIC);
}


bool System::IsIMPL() const
{
  return (preferredSystem >= DDS_SYSTEM_THREAD_STLIMPL);
}


bool System::ThreadOK(const int thrId) const
{
  return (thrId >= 0 && thrId < numThreads);
}


int System::PreferThreading(const unsigned code)
{
  if (code >= DDS_SYSTEM_THREAD_SIZE)
    return RETURN_THREAD_MISSING;

  if (! availableSystem[code])
    return RETURN_THREAD_MISSING;

  preferredSystem = code;
  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                           Basic                                  //
//////////////////////////////////////////////////////////////////////

int System::RunThreadsBasic(RunMode runCat, Scheduler &scheduler)
{
  CallbackSimpleList[runCat](0, scheduler);
  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                           WinAPI                                 //
//////////////////////////////////////////////////////////////////////

#ifdef DDS_THREADS_WINAPI
struct WinWrapType
{
  int thrId;
  fptrType fptr;
  HANDLE *waitPtr;
  Scheduler *scheduler;
};

DWORD CALLBACK WinCallback(void * p);

DWORD CALLBACK WinCallback(void * p)
{
  WinWrapType * winWrap = static_cast<WinWrapType *>(p);
  (*(winWrap->fptr))(winWrap->thrId, *winWrap->scheduler);

  if (SetEvent(winWrap->waitPtr[winWrap->thrId]) == 0)
    return 0;

  return 1;
}
#endif


int System::RunThreadsWinAPI(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_WINAPI
  auto fptr = CallbackSimpleList[runCat];
  HANDLE * solveAllEvents = static_cast<HANDLE * >(
    malloc(static_cast<unsigned>(numThreads) * sizeof(HANDLE)));

  for (int k = 0; k < numThreads; k++)
  {
    solveAllEvents[k] = CreateEvent(NULL, FALSE, FALSE, 0);
    if (solveAllEvents[k] == 0)
      return RETURN_THREAD_CREATE;
  }

  vector<WinWrapType> winWrap;
  const unsigned nt = static_cast<unsigned>(numThreads);
  winWrap.resize(nt);

  for (unsigned k = 0; k < nt; k++)
  {
    winWrap[k].thrId = static_cast<int>(k);
    winWrap[k].fptr = fptr;
    winWrap[k].waitPtr = solveAllEvents;
    winWrap[k].scheduler = &scheduler;

    int res = QueueUserWorkItem(WinCallback,
      static_cast<void *>(&winWrap[k]), WT_EXECUTELONGFUNCTION);
    if (res != 1)
      return res;
  }

  DWORD solveAllWaitResult;
  solveAllWaitResult = WaitForMultipleObjects(
    static_cast<unsigned>(numThreads), solveAllEvents, TRUE, INFINITE);

  if (solveAllWaitResult != WAIT_OBJECT_0)
    return RETURN_THREAD_WAIT;

  for (int k = 0; k < numThreads; k++)
    CloseHandle(solveAllEvents[k]);

  free(solveAllEvents);
#endif

  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                           OpenMP                                 //
//////////////////////////////////////////////////////////////////////

int System::RunThreadsOpenMP(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_OPENMP
  // Added after suggestion by Dirk Willecke.
  if (omp_get_dynamic())
    omp_set_dynamic(0);

  omp_set_num_threads(numThreads);

  auto fptr = CallbackSimpleList[runCat];
  #pragma omp parallel shared(fptr, scheduler)
  {
    #pragma omp for schedule(dynamic)
    for (int k = 0; k < numThreads; k++)
    {
      int thrId = omp_get_thread_num();
      (*fptr)(thrId, scheduler);
    }
  }
#endif

  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                            GCD                                   //
//////////////////////////////////////////////////////////////////////

int System::RunThreadsGCD(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_GCD
  auto fptr = CallbackSimpleList[runCat];
  dispatch_apply(static_cast<size_t>(numThreads),
    dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0),
    ^(size_t t)
  {
    int thrId = static_cast<int>(t);
    (*fptr)(thrId, scheduler);
  });
#endif

  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                           Boost                                  //
//////////////////////////////////////////////////////////////////////

int System::RunThreadsBoost(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_BOOST
  auto fptr = CallbackSimpleList[runCat];
  vector<boost::thread *> threads;

  const unsigned nu = static_cast<unsigned>(numThreads);
  threads.resize(nu);

  for (unsigned k = 0; k < nu; k++)
    threads[k] = new boost::thread(fptr, k, std::ref(scheduler));

  for (unsigned k = 0; k < nu; k++)
  {
    threads[k]->join();
    delete threads[k];
  }
#endif

  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                            STL                                   //
//////////////////////////////////////////////////////////////////////

int System::RunThreadsSTL(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_STL
  auto fptr = CallbackSimpleList[runCat];
  vector<thread> threads;

  vector<int> uniques;
  vector<int> crossrefs;
  (* CallbackDuplList[runCat])(*scheduler.GetBOP(), uniques, crossrefs);

  const unsigned nu = static_cast<unsigned>(numThreads);
  threads.reserve(nu);
  for (unsigned k = 1; k < nu; k++)
    threads.emplace_back(fptr, k, std::ref(scheduler));

  fptr(0, scheduler);

  for (auto& t: threads)
    t.join();
#endif

  return RETURN_NO_FAULT;
}

void System::WorkerSTLAsync(fptrType fptr, Scheduler &scheduler)
{
#ifdef DDS_THREADS_STLASYNC
  auto threadId = threadMgr.Occupy();
  fptr(threadId, scheduler);
#endif
}

int System::RunThreadsSTLAsync(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_STLASYNC
  auto fptr = CallbackSimpleList[runCat];

  std::vector<std::future<void>> futures;
  std::vector<int> uniques;
  std::vector<int> crossrefs;
  (* CallbackDuplList[runCat])(*scheduler.GetBOP(), uniques, crossrefs);

  const unsigned nu = static_cast<unsigned>(numThreads);
  futures.reserve(nu);
  // Launch first worker as deferred to allow it run in same thread
  futures.push_back(std::async(std::launch::deferred,
        &System::WorkerSTLAsync, this));
  for (unsigned k = 0; k < nu; k++)
    futures.push_back(std::async(&System::WorkerSTLAsync, this, fptr, std::ref(scheduler)));

  for (auto& f: futures)
    f.wait();
#endif

  return RETURN_NO_FAULT;
}


int System::RunThreadsSTLIMPL(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_STLIMPL
  vector<int> uniques;
  vector<int> crossrefs;
  (* CallbackDuplList[runCat])(*scheduler.GetBOP(), uniques, crossrefs);

  for_each(std::execution::par, uniques.begin(), uniques.end(),
    [&](int &bno)
  {
    auto realThrId = threadMgr.Occupy();

    (* CallbackSingleList[runCat])(realThrId, bno);
  });

  (* CallbackCopyList[runCat])(crossrefs);
#endif

  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                            TBB                                   //
//////////////////////////////////////////////////////////////////////

int System::RunThreadsTBB(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_TBB
  auto fptr = CallbackSimpleList[runCat];
  vector<tbb::tbb_thread *> threads;

  const unsigned nu = static_cast<unsigned>(numThreads);
  threads.resize(nu);

  for (unsigned k = 0; k < nu; k++)
    threads[k] = new tbb::tbb_thread(fptr, k, std::ref(scheduler));

  for (unsigned k = 0; k < nu; k++)
  {
    threads[k]->join();
    delete threads[k];
  }
#endif

  return RETURN_NO_FAULT;
}


//////////////////////////////////////////////////////////////////////
//                            PPL                                   //
//////////////////////////////////////////////////////////////////////


int System::RunThreadsPPLIMPL(RunMode runCat, Scheduler &scheduler)
{
  UNUSED(runCat);
  UNUSED(scheduler);
#ifdef DDS_THREADS_PPLIMPL
  vector<int> uniques;
  vector<int> crossrefs;
  (* CallbackDuplList[runCat])(* bop, uniques, crossrefs);

  Concurrency::parallel_for_each(uniques.begin(), uniques.end(),
    [&](int &bno)
  {
    auto realThrId = threadMgr.Occupy();

    (* CallbackSingleList[runCat])(realThrId, bno);
  });


  (* CallbackCopyList[runCat])(crossrefs);
#endif

  return RETURN_NO_FAULT;
}

int System::RunThreads(RunMode runCat,
    const boards& bop,
    const playTracesBin& pl)
{
  Scheduler scheduler{numThreads, runCat, bop, pl};

  return (this->*RunPtrList[preferredSystem])(runCat, scheduler);
}

int System::RunThreads(RunMode runCat, const boards& bop)
{
  Scheduler scheduler{numThreads, runCat, bop};

  return (this->*RunPtrList[preferredSystem])(runCat, scheduler);
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
#if defined(USES_DLLMAIN)
  cons = 1;
#elif defined(USES_CONSTRUCTOR)
  cons = 2;
#else
  cons = 0;
#endif

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


string System::GetThreading(int& thr) const
{
  string st = "";
  thr = 0;
  for (unsigned k = 0; k < DDS_SYSTEM_THREAD_SIZE; k++)
  {
    if (availableSystem[k])
    {
      st += " " + DDS_SYSTEM_THREADING[k];
      if (k == preferredSystem)
      {
        st += "(*)";
        thr = static_cast<int>(k);
      }
    }
  }
  return st;
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

  const string strThreading = System::GetThreading(info->threading);
  ss << left << setw(9) << "Threading" <<
    setw(24) << right << strThreading << "\n";

  const string st = ss.str();
  strcpy(info->systemString, st.c_str());
  return st;
}

