#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
} SysctlHW;

// NOTES:
// Docs used:
// - https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_system_capabilities
// - Comments in /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/sysctl.h
//   (search for hw.memsize and look at the comment there)
// - man 3 sysctl

#define GetSystemInfo(name, retvalPtr) do {                             \
    size_t size = sizeof(*(retvalPtr));                                 \
                                                                        \
    if (sysctlbyname((name), (retvalPtr), &size, NULL, 0) < 0) {        \
      fprintf(stderr, "Failed to get " name ": %s\n", strerror(errno)); \
      perror("sysctl");                                                 \
      return 1;                                                         \
    }                                                                   \
  } while(0)

#define KB(n)   (n) * 1024
#define MB(n) KB(n) * 1024
#define GB(n) MB(n) * 1024

void PrintSysctlHW(const SysctlHW *const hw)
{
  printf("Processor:\n");
  printf("\tPhysical cores:      %d (enabled = %d)\n",
         hw->physicalcpu_max, hw->physicalcpu);
  printf("\tLogical cores:       %d (enabled = %d)\n",
         hw->logicalcpu_max, hw->logicalcpu);
  printf("\tCPU type:            %d (subtype = %d)\n",
         hw->cputype, hw->cpusubtype);
  printf("\tByte order:          %s Endian (%d)\n",
         hw->byteorder == 1234 ? "Little" : "Big", hw->byteorder);

  printf("Memory:\n");
  printf("\tTotal physical:      %lld GB\n",
         hw->memsize/(GB(1)));

  printf("OS:\n");
  printf("\tTime base frequency: %lld\n",
         hw->tbfrequency);
}

int main(void)
{
  SysctlHW hw = {0};

  /* CPU */
  GetSystemInfo("hw.physicalcpu_max", &hw.physicalcpu_max);
  GetSystemInfo("hw.physicalcpu", &hw.physicalcpu);
  GetSystemInfo("hw.logicalcpu_max", &hw.logicalcpu_max);
  GetSystemInfo("hw.logicalcpu", &hw.logicalcpu);
  GetSystemInfo("hw.cputype", &hw.cputype);
  GetSystemInfo("hw.cpusubtype", &hw.cpusubtype);
  GetSystemInfo("hw.byteorder", &hw.byteorder);

  /* Memory */
  GetSystemInfo("hw.memsize", &hw.memsize);

  /* OS */
  GetSystemInfo("hw.tbfrequency", &hw.tbfrequency);


  PrintSysctlHW(&hw);

  return 0;
}
