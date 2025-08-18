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


typedef struct {
  char    brand_string[32];
  int32_t virtual_address_size;
  struct {
    int32_t cores_per_package;
    int32_t logical_per_package;
    int32_t thread_count;
  } cpu;
} MachDep;

typedef struct {
  char version[128];
  int32_t stack_size;
} Kern;

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
  char    *name;
} PerfLevelN;

typedef struct {
  int32_t       cpufamily;
  int32_t       cpusubfamily;
  int32_t       packages;
  int32_t       nperflevels;
  int32_t       byteorder;
  cpu_type_t    cputype;
  cpu_subtype_t cpusubtype;
  int32_t       cputhreadtype;
  int32_t       cpu64bit_capable;

  int64_t       memsize;
  int64_t       pagesize;
  int64_t       cachelinesize;
  int64_t       l1dcachesize;
  int64_t       l1icachesize;
  int64_t       l2cachesize;
  int64_t       l3cachesize;
  int64_t       tbfrequency;
  PerfLevelN    *perflevelN;
} HW;

typedef struct {
  Kern    kern;
  MachDep machdep;
  HW      hw;
} Sysctl;

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
#ifndef NDEBUG
    fprintf(stderr,
            "Failed to get sysctlbyname(\"%s\", ...): %s\n",
            name, strerror(errno));
#endif // NDEBUG
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

// CPUFAMILY_ARM_FIRESTORM_ICESTORM, etc are defined in mach/machine.h
// in system headers
const char *GetCPUFamilyName(int32_t family)
{
  // NOTES: Returning pointers to static strings is fine as they are
  // in the readonly data segement of the binary image (.rodata/.rdata
  // sections in asm for e.g.)  and hence have stable global pointers
  switch(family) {
    case CPUFAMILY_ARM_LIGHTNING_THUNDER:
      return "ARM Lightning/Thunder";
    case CPUFAMILY_ARM_FIRESTORM_ICESTORM:
      return "ARM Firestorm/Icestorm";
    case CPUFAMILY_ARM_BLIZZARD_AVALANCHE:
      return "ARM Blizzard/Avalanche";
    case CPUFAMILY_ARM_EVEREST_SAWTOOTH:
      return "ARM Everest/Sawtooth";

    case CPUFAMILY_INTEL_PENRYN:
      return "Intel Penryn";
    case CPUFAMILY_INTEL_NEHALEM:
      return "Intel Nehalem";
    case CPUFAMILY_INTEL_WESTMERE:
      return "Intel Westmere";
    case CPUFAMILY_INTEL_SANDYBRIDGE:
      return "Intel Sandybridge";
    case CPUFAMILY_INTEL_IVYBRIDGE:
      return "Intel Ivybridge";
    case CPUFAMILY_INTEL_HASWELL:
      return "Intel Haswell";
    case CPUFAMILY_INTEL_BROADWELL:
      return "Intel Broadwell";
    case CPUFAMILY_INTEL_SKYLAKE:
      return "Intel Skylake";

    default:
      return "Unknown";
  }
}

// CPUSUBFAMILY_ARM_HP, etc are defined in mach/machine.h
// in system headers
const char *GetCPUSubFamilyName(int32_t subfamily)
{
  switch(subfamily) {
    case CPUSUBFAMILY_ARM_HP:
      return "ARM HP";
    case CPUSUBFAMILY_ARM_HG:
      return "ARM HG";
    case CPUSUBFAMILY_ARM_M:
      return "ARM M";
    case CPUSUBFAMILY_ARM_HS:
      return "ARM HS";
    case CPUSUBFAMILY_ARM_HC_HD:
      return "ARM HC HD";
    case CPUSUBFAMILY_ARM_HA:
      return "ARM HA";
    default:
      return "Unknown";
  }
}

const char *GetCPUTypeName(cpu_type_t cputype)
{
  switch(cputype) {
    case CPU_TYPE_ARM:
      return "ARM";
    case CPU_TYPE_ARM64:
      return "ARM64";
    case CPU_TYPE_ARM64_32:
      return "ARM64_32"; // wtf?
    case CPU_TYPE_X86_64:
      return "x86_64";
    case CPU_TYPE_X86:
      return "x86";
    default:
      return "Unknown";
  }
}

const char *GetCPUSubTypeName(cpu_subtype_t cpusubtype)
{
  switch(cpusubtype) {
    case CPU_SUBTYPE_X86_64_ALL:
      return "x86-64 (all)";
    case CPU_SUBTYPE_X86_64_H: { // this is 8 which clashes with CPU_SUBTYPE_ARM_XSCALE
      if (CPU_SUBTYPE_X86_64_H == CPU_SUBTYPE_ARM_XSCALE) {
        return "x86-64 Haswell feature subset or ARM XSCALE";
      }
      return "x86-64 Haswell feature subset";
    }

    case CPU_SUBTYPE_ARM_V4T:
      return "ARM V4T";
    case CPU_SUBTYPE_ARM_V6:
      return "ARM V6";
    case CPU_SUBTYPE_ARM_V5TEJ:
      return "ARM V5TEJ";
    case CPU_SUBTYPE_ARM_V7:
      return "ARM V7";
    case CPU_SUBTYPE_ARM_V7F:
      return "ARM V7F";
    case CPU_SUBTYPE_ARM_V7S:
      return "ARM V7S";
    case CPU_SUBTYPE_ARM_V7K:
      return "ARM V7K";
    case CPU_SUBTYPE_ARM_V8:
      return "ARM V8";
    case CPU_SUBTYPE_ARM_V6M:
      return "ARM V6M";
    case CPU_SUBTYPE_ARM_V7M:
      return "ARM V7M";
    case CPU_SUBTYPE_ARM_V7EM:
      return "ARM V7EM";
    case CPU_SUBTYPE_ARM_V8M:
      return "ARM V8M";

    case CPU_SUBTYPE_ARM64_ALL:
      return "ARM64 ALL";
    case CPU_SUBTYPE_ARM64_V8:
      return "ARM64 V8";
    case CPU_SUBTYPE_ARM64E:
      return "ARM64 64E";

    default:
      return "Unknown";
  }
}


// TODO(mudit): Figure out how to report a failed fetch for a name aka
//              val = FAILED_FETCH in a nice human readable manner
void PrintSysctl(const Sysctl *const sysctl)
{
  printf("Processor:\n");
  printf("\tName:                %s (64bit = %s)\n",
         sysctl->machdep.brand_string,sysctl->hw.cpu64bit_capable ? "true" : "false");
  printf("\tCPU family:          %s (subfamily = %s)\n",
         GetCPUFamilyName(sysctl->hw.cpufamily), GetCPUSubFamilyName(sysctl->hw.cpusubfamily));
  printf("\tCPU type:            %s (subtype = %s%s)\n",
         GetCPUTypeName(sysctl->hw.cputype),
         GetCPUSubTypeName(sysctl->hw.cpusubtype),
         sysctl->hw.cputhreadtype != FAILED_FETCH
           ? FormatStr(", threadtype = %d", sysctl->hw.cputhreadtype)
           : "");
  printf("\tNumber of packages:  %d\n",
         sysctl->hw.packages);
  printf("\tPhysical cores:      %d\n",
         sysctl->machdep.cpu.cores_per_package * sysctl->hw.packages);
  printf("\tLogical cores:       %d\n",
         sysctl->machdep.cpu.logical_per_package * sysctl->hw.packages);
  printf("\tThreads:             %d\n",
         sysctl->machdep.cpu.thread_count);

  for (int32_t i = 0; i < sysctl->hw.nperflevels; ++i) {
    printf("\tCore type:           %s\n",
           sysctl->hw.perflevelN[i].name);
    printf("\t\tPhysical:       %d (available = %d [inactive = %d])\n",
           sysctl->hw.perflevelN[i].physicalcpu, sysctl->hw.perflevelN[i].physicalcpu_max,
           (sysctl->hw.perflevelN[i].physicalcpu_max - sysctl->hw.perflevelN[i].physicalcpu));
    printf("\t\tLogical:        %d (available = %d [inactive = %d])\n",
           sysctl->hw.perflevelN[i].logicalcpu, sysctl->hw.perflevelN[i].logicalcpu_max,
           (sysctl->hw.perflevelN[i].logicalcpu_max - sysctl->hw.perflevelN[i].logicalcpu));
    printf("\t\tL1 data cache:  %d KB (%d bytes)\n",
           sysctl->hw.perflevelN[i].l1dcachesize/(KB(1)), sysctl->hw.perflevelN[i].l1dcachesize);
    printf("\t\tL1 inst cache:  %d KB (%d bytes)\n",
           sysctl->hw.perflevelN[i].l1icachesize/(KB(1)), sysctl->hw.perflevelN[i].l1icachesize);
    printf("\t\tL2 cache:       %d MB (%d bytes)\n",
           sysctl->hw.perflevelN[i].l2cachesize/(MB(1)), sysctl->hw.perflevelN[i].l2cachesize);

    if (sysctl->hw.perflevelN[i].l3cachesize != FAILED_FETCH) {
      printf("\t\tL3 cache:       %d bytes\n",
             sysctl->hw.perflevelN[i].l3cachesize);
    }
  }
  printf("\tByte order:          %s Endian (%d)\n",
         sysctl->hw.byteorder == 1234 ? "Little" : "Big", sysctl->hw.byteorder);

  printf("Memory:\n");
  printf("\tTotal physical:      %lld GB\n",
         sysctl->hw.memsize/(GB(1)));
  printf("\tVirtual addr size:   %d bits (user space = %#llx to %#018llx)\n",
         sysctl->machdep.virtual_address_size, (uint64_t)0,
         ((uint64_t)1 << sysctl->machdep.virtual_address_size) - 1);
  printf("\tCache as seen by current process:\n");
  printf("\t\tMax cache line: %lld bytes (but is probably 64 KB)\n",
         sysctl->hw.cachelinesize);
  printf("\t\tL1 data cache:  %lld KB (%lld bytes)\n",
         sysctl->hw.l1dcachesize/(KB(1)), sysctl->hw.l1dcachesize);
  printf("\t\tL1 inst cache:  %lld KB (%lld bytes)\n",
         sysctl->hw.l1icachesize/(KB(1)), sysctl->hw.l1icachesize);
  printf("\t\tL2 cache:       %lld MB (%lld bytes)\n",
         sysctl->hw.l2cachesize/(MB(1)), sysctl->hw.l2cachesize);

  if (sysctl->hw.l3cachesize != FAILED_FETCH) {
    printf("\tL3 cache size:       %lld KB (%lld bytes)\n",
           sysctl->hw.l3cachesize/(KB(1)), sysctl->hw.l3cachesize);
  }


  printf("OS:\n");
  printf("\tTime base frequency: %lld\n",
         sysctl->hw.tbfrequency);
  printf("\tPage size:           %lld KB (%lld bytes)\n",
         sysctl->hw.pagesize/(KB(1)), sysctl->hw.pagesize);
  printf("\tStack size:          %d KB (%d bytes)\n",
         sysctl->kern.stack_size/(KB(1)), sysctl->kern.stack_size);
  printf("\tKernel version:      %s\n",
         sysctl->kern.version);
}

int main(void)
{
  Sysctl sysctl = {0};

  /* CPU */
  GetSystemInfo_("machdep.cpu.brand_string", sysctl.machdep.brand_string, sizeof(sysctl.machdep.brand_string));
  GetSystemInfo("hw.packages", &sysctl.hw.packages);
  GetSystemInfo("hw.nperflevels", &sysctl.hw.nperflevels);

  PerfLevelN perflevelN[sysctl.hw.nperflevels];
  const char *buffers[sysctl.hw.nperflevels][128];

  sysctl.hw.perflevelN = perflevelN;

  for (int32_t i = 0; i < sysctl.hw.nperflevels; ++i) {
    size_t sz = sizeof(*buffers)/sizeof(**buffers);

    sysctl.hw.perflevelN[i].name = (char *)buffers[i];
    memset(sysctl.hw.perflevelN[i].name, 0, sz);

    GetSystemInfo_(FormatStr("hw.perflevel%d.name", i), sysctl.hw.perflevelN[i].name, sz);
    GetSystemInfo(FormatStr("hw.perflevel%d.physicalcpu", i), &sysctl.hw.perflevelN[i].physicalcpu);
    GetSystemInfo(FormatStr("hw.perflevel%d.physicalcpu_max", i), &sysctl.hw.perflevelN[i].physicalcpu_max);
    GetSystemInfo(FormatStr("hw.perflevel%d.logicalcpu", i), &sysctl.hw.perflevelN[i].logicalcpu);
    GetSystemInfo(FormatStr("hw.perflevel%d.logicalcpu_max", i), &sysctl.hw.perflevelN[i].logicalcpu_max);
    GetSystemInfo(FormatStr("hw.perflevel%d.l1dcachesize", i), &sysctl.hw.perflevelN[i].l1dcachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.l1icachesize", i), &sysctl.hw.perflevelN[i].l1icachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.l2cachesize", i), &sysctl.hw.perflevelN[i].l2cachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.l3cachesize", i), &sysctl.hw.perflevelN[i].l3cachesize);
    GetSystemInfo(FormatStr("hw.perflevel%d.cpusperl2", i), &sysctl.hw.perflevelN[i].cpusperl2);
    GetSystemInfo(FormatStr("hw.perflevel%d.cpusperl3", i), &sysctl.hw.perflevelN[i].cpusperl3);
    GetSystemInfo(FormatStr("hw.perflevel%d.l2perflevels", i), &sysctl.hw.perflevelN[i].l2perflevels);
    GetSystemInfo(FormatStr("hw.perflevel%d.l3perflevels", i), &sysctl.hw.perflevelN[i].l3perflevels);
  }

  GetSystemInfo("machdep.cpu.cores_per_package", &sysctl.machdep.cpu.cores_per_package);
  GetSystemInfo("machdep.cpu.logical_per_package", &sysctl.machdep.cpu.logical_per_package);
  GetSystemInfo("machdep.cpu.thread_count", &sysctl.machdep.cpu.thread_count);
  GetSystemInfo("hw.cputype", &sysctl.hw.cputype);
  GetSystemInfo("hw.cpusubtype", &sysctl.hw.cpusubtype);
  GetSystemInfo("hw.cputhreadtype", &sysctl.hw.cputhreadtype);
  GetSystemInfo("hw.cpufamily", &sysctl.hw.cpufamily);
  GetSystemInfo("hw.cpusubfamily", &sysctl.hw.cpusubfamily);
  GetSystemInfo("hw.cpu64bit_capable", &sysctl.hw.cpu64bit_capable);
  GetSystemInfo("hw.byteorder", &sysctl.hw.byteorder);

  /* Memory */
  GetSystemInfo("hw.memsize", &sysctl.hw.memsize);
  GetSystemInfo("hw.cachelinesize", &sysctl.hw.cachelinesize);
  GetSystemInfo("hw.l1dcachesize", &sysctl.hw.l1dcachesize);
  GetSystemInfo("hw.l1icachesize", &sysctl.hw.l1icachesize);
  GetSystemInfo("hw.l2cachesize", &sysctl.hw.l2cachesize);
  GetSystemInfo("hw.l3cachesize", &sysctl.hw.l3cachesize);
  GetSystemInfo("machdep.virtual_address_size", &sysctl.machdep.virtual_address_size);

  /* OS */
  GetSystemInfo("hw.tbfrequency", &sysctl.hw.tbfrequency);
  GetSystemInfo("hw.pagesize", &sysctl.hw.pagesize);
  GetSystemInfo("kern.stack_size", &sysctl.kern.stack_size);
  GetSystemInfo_("kern.version", sysctl.kern.version, sizeof(sysctl.kern.version));

  PrintSysctl(&sysctl);

  // TODO(mudit): Print sizes of all old style built in C types

  return 0;
}
