#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/machine.h>

/**
 *            Name                         Type          Changeable
           hw.activecpu                 int32_t       no
           hw.byteorder                 int32_t       no
           hw.cacheconfig               uint64_t[]    no
           hw.cachelinesize             int64_t       no
           hw.cachesize                 uint64_t[]    no
           hw.cpu64bit_capable          int32_t       no
           hw.cpufamily                 uint32_t      no
           hw.cpufrequency              int64_t       no
           hw.cpufrequency_max          int64_t       no
           hw.cpufrequency_min          int64_t       no
           hw.cpusubtype                int32_t       no
           hw.cputhreadtype             int32_t       no
           hw.cputype                   int32_t       no
           hw.l1dcachesize              int64_t       no
           hw.l1icachesize              int64_t       no
           hw.l2cachesize               int64_t       no
           hw.l3cachesize               int64_t       no
           hw.logicalcpu                int32_t       no
           hw.logicalcpu_max            int32_t       no
           hw.machine                   char[]        no
           hw.memsize                   int64_t       no
           hw.model                     char[]        no
           hw.ncpu                      int32_t       no
           hw.packages                  int32_t       no
           hw.pagesize                  int64_t       no
           hw.physicalcpu               int32_t       no
           hw.physicalcpu_max           int32_t       no
           hw.tbfrequency               int64_t       no

     hw.byteorder
             The byte order (4321 or 1234).

     hw.model
             The machine model.

     hw.ncpu
             The number of cpus. This attribute is deprecated and it is
             recommended that hw.logicalcpu, hw.logicalcpu_max,
             hw.physicalcpu, or hw.physicalcpu_max be used instead.

     hw.logicalcpu
             The number of logical processors available in the current power
             management mode.

     hw.logicalcpu_max
             The maximum number of logical processors that could be available
             this boot.

     hw.physicalcpu
             The number of physical processors available in the current power
             management mode.

     hw.physicalcpu_max
             The maximum number of physical processors that could be available
             this boot.

     hw.pagesize
             The software page size in bytes.


 */

typedef struct {
  const char *brand_string;
  int32_t virtual_address_size;
} MachDep;

typedef struct {
  int32_t physicalcpu;
  int32_t physicalcpu_max;
  int32_t logicalcpu;
  int32_t logicalcpu_max;
  int32_t l1dcachesize;
  int32_t l1icachesize;
  int32_t l2cachesize;
  int32_t l3cachesize;
  // These values provide the number of CPUs of the same type that
  // share L2 and L3 caches. If a cache is not present then the
  // selector will return and error.
  int32_t cpusperl2;
  int32_t cpusperl3;

  // These values provide a bitmap, where bit number of CPUs of the
  // same type that share L2 and L3 caches. If a cache is not present
  // then the selector will return and error.
  int32_t l2perflevels;
  int32_t l3perflevels;

  char *name;
} PerfLevelN;

typedef struct {

  /* uint64_t[] cacheconfig; */
  int64_t cachelinesize;
  /* uint64_t[] cachesize; */
  int32_t cpu64bit_capable;
  uint32_t cpufamily;
  int32_t cputhreadtype;

  int64_t l1dcachesize;
  int64_t l1icachesize;
  int64_t l2cachesize;
  int64_t l3cachesize;

  char product[128]; /* alias for deprecated name "hw.machine" */
  char target[128]; /* alias for deprecated name "hw.model" */

  int32_t packages;
  int64_t pagesize;


  /* CPU */
  int32_t nperflevels;
  PerfLevelN *perflevelN;
  int32_t logicalcpu;
  int32_t logicalcpu_max;
  int32_t physicalcpu;
  int32_t physicalcpu_max;
  cpu_type_t cputype;
  cpu_subtype_t cpusubtype;
  int32_t byteorder;

  /* Memory */
  int64_t memsize;


  /* OS
   *   hw.tbfrequency - This gives the time base frequency used by the OS and is the basis of
   *                    all timing services. In general is is better to use mach's or higher level
   *                    timing services, but this value is needed to convert the PPC Time Base registers
   *                    to real time.
   *
   */
  int64_t tbfrequency;

  MachDep machdep;
} SysctlHW;

static const int64_t FAILED_FETCH = -9999;

// NOTES:
// Docs used:
// - output of `sysctl -a`
// - https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_system_capabilities
// - Comments in /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/sysctl.h
//   (search for hw.memsize and look at the comment there)
// - man 3 sysctl

#define KB(n)   (n) * 1024
#define MB(n) KB(n) * 1024
#define GB(n) MB(n) * 1024

bool GetSystemInfo_(const char *name, void *retvalPtr, size_t size)
{
  if (sysctlbyname(name, retvalPtr, &size, NULL, 0) < 0) {
    fprintf(stderr,
            "Failed to get sysctlbyname(\"%s\", ...): %s\n",
            name, strerror(errno));
    return false;
  }

  return true;
}
#define GetSystemInfo(name, retvalPtr) do {             \
    const size_t size = sizeof(*(retvalPtr));           \
    if (!GetSystemInfo_((name), (retvalPtr), size)) {   \
      *(retvalPtr) = FAILED_FETCH;                      \
    }                                                   \
  } while(0)


// NOTES: Leaks memory. Don't care as program isn't long running.
const char *FormatStr(const char *fmt, ...)
{

  va_list ap;

  va_start(ap, fmt);

  int len   = vsnprintf(NULL, 0, fmt, ap) + 1;
  char *str = malloc(len);

  if (str == NULL) {
    return NULL;
  }

  (void) vsnprintf(str, len, fmt, ap);

  va_end(ap);

  return str;
}

// TODO(mudit): Figure out how to report a failed fetch for a name aka
//              val = FAILED_FETCH in a nice human readable manner
void PrintSysctlHW(const SysctlHW *const hw)
{
  printf("Processor:\n");
  printf("\tNumber of packages:  %d\n",
         hw->packages);
  printf("\tPhysical cores:      %d (enabled = %d)\n",
         hw->physicalcpu_max, hw->physicalcpu);
  printf("\tLogical cores:       %d (enabled = %d)\n",
         hw->logicalcpu_max, hw->logicalcpu);

  for (int32_t i = 0; i < hw->nperflevels; ++i) {
    printf("\tCore type:           %s\n",
           hw->perflevelN[i].name);
    printf("\t\tPhysical:       %d (max for this boot = %d)\n",
           hw->perflevelN[i].physicalcpu, hw->perflevelN[i].physicalcpu_max);
    printf("\t\tLogical:        %d (max for this boot = %d)\n",
           hw->perflevelN[i].logicalcpu, hw->perflevelN[i].logicalcpu_max);
    printf("\t\tL1 data cache:  %d bytes\n",
           hw->perflevelN[i].l1dcachesize);
    printf("\t\tL1 inst cache:  %d bytes\n",
           hw->perflevelN[i].l1icachesize);
    printf("\t\tL2 cache:       %d bytes\n",
           hw->perflevelN[i].l2cachesize);
    printf("\t\tL3 cache:       %d bytes\n",
           hw->perflevelN[i].l3cachesize);
  }

  printf("\tCPU type:            %d (subtype = %d, threadtype = %d)\n",
         hw->cputype, hw->cpusubtype, hw->cputhreadtype);
  printf("\tByte order:          %s Endian (%d)\n",
         hw->byteorder == 1234 ? "Little" : "Big", hw->byteorder);

  printf("Memory:\n");
  printf("\tTotal physical:      %lld GB\n",
         hw->memsize/(GB(1)));
  printf("\tCache line size:     %lld bytes\n",
         hw->cachelinesize);
  printf("\tL1 data cache size:  %lld bytes\n",
         hw->l1dcachesize);
  printf("\tL1 inst cache size:  %lld bytes\n",
         hw->l1icachesize);
  printf("\tL2 cache size:       %lld bytes\n",
         hw->l2cachesize);
  printf("\tL3 cache size:       %lld bytes\n",
         hw->l3cachesize);
  printf("\tVirtual addr size:   %d bits (min=%#llx, max=%#llx)\n",
         hw->machdep.virtual_address_size, (uint64_t)0, (uint64_t)1 << hw->machdep.virtual_address_size);


  printf("OS:\n");
  printf("\tTime base frequency: %lld\n",
         hw->tbfrequency);
  printf("\tPage size:           %lld KB (%lld bytes)\n",
         hw->pagesize/(KB(1)), hw->pagesize);
}

int main(void)
{
  SysctlHW hw = {0};


  /* CPU */
  GetSystemInfo("hw.packages", &hw.packages);
  GetSystemInfo("hw.nperflevels", &hw.nperflevels);
  PerfLevelN perflevelN[hw.nperflevels];
  hw.perflevelN = perflevelN;

  const char *buffers[hw.nperflevels][128];

  for (int32_t i = 0; i < hw.nperflevels; ++i) {
    hw.perflevelN[i].name = (char *)buffers[i];
    memset(hw.perflevelN[i].name, 0, sizeof(*buffers)/sizeof(**buffers));

    GetSystemInfo(FormatStr("hw.perflevel%d.physicalcpu", i), &hw.perflevelN[i].physicalcpu);
    GetSystemInfo(FormatStr("hw.perflevel%d.physicalcpu_max", i), &hw.perflevelN[i].physicalcpu_max);
    GetSystemInfo(FormatStr("hw.perflevel%d.logicalcpu", i), &hw.perflevelN[i].logicalcpu);
    GetSystemInfo(FormatStr("hw.perflevel%d.logicalcpu_max", i), &hw.perflevelN[i].logicalcpu_max);
    GetSystemInfo(FormatStr("hw.perflevel%d.l1dcachesize", i), &hw.perflevelN[i].l1dcachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.l1icachesize", i), &hw.perflevelN[i].l1icachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.l2cachesize", i), &hw.perflevelN[i].l2cachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.l3cachesize", i), &hw.perflevelN[i].l3cachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.cpusperl2", i), &hw.perflevelN[i].cpusperl2);
    GetSystemInfo(FormatStr("hw.perflevel%d.cpusperl3", i), &hw.perflevelN[i].cpusperl3);
    GetSystemInfo(FormatStr("hw.perflevel%d.l2perflevels", i), &hw.perflevelN[i].l2perflevels);
    GetSystemInfo(FormatStr("hw.perflevel%d.l3perflevels", i), &hw.perflevelN[i].l3perflevels);
    GetSystemInfo_(FormatStr("hw.perflevel%d.name", i), hw.perflevelN[i].name, 128);
  }

  GetSystemInfo("hw.physicalcpu_max", &hw.physicalcpu_max);
  GetSystemInfo("hw.physicalcpu", &hw.physicalcpu);
  GetSystemInfo("hw.logicalcpu_max", &hw.logicalcpu_max);
  GetSystemInfo("hw.logicalcpu", &hw.logicalcpu);
  GetSystemInfo("hw.cputype", &hw.cputype);
  GetSystemInfo("hw.cpusubtype", &hw.cpusubtype);
  GetSystemInfo("hw.cputhreadtype", &hw.cputhreadtype);
  GetSystemInfo("hw.byteorder", &hw.byteorder);

  /* Memory */
  GetSystemInfo("hw.memsize", &hw.memsize);
  GetSystemInfo("hw.cachelinesize", &hw.cachelinesize);
  GetSystemInfo("hw.l1dcachesize", &hw.l1dcachesize);
  GetSystemInfo("hw.l1icachesize", &hw.l1icachesize);
  GetSystemInfo("hw.l2cachesize", &hw.l2cachesize);
  GetSystemInfo("hw.l3cachesize", &hw.l3cachesize);
  GetSystemInfo("machdep.virtual_address_size", &hw.machdep.virtual_address_size);

  /* OS */
  GetSystemInfo("hw.tbfrequency", &hw.tbfrequency);
  GetSystemInfo("hw.pagesize", &hw.pagesize);

  PrintSysctlHW(&hw);

  // TODO(mudit): Print sizes of all old style built in C types

  return 0;
}
