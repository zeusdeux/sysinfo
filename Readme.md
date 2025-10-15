# sysinfo

A small tool to print system information on Apple machines.

## Quick start

```
curl -L https://github.com/zeusdeux/sysinfo/releases/download/v1.0.1/sysinfo -o ./sysinfo
chmod +x ./sysinfo
./sysinfo
```

> Using `curl` to download from releases to get rid of the quarantine bit that gets set
> when downloading via a browser.

### Build locally

```sh
git clone git@github.com:zeusdeux/sysinfo.git
# OR
git clone https://github.com/zeusdeux/sysinfo.git

cd sysinfo
make -B
./.build/sysinfo
```

You can then symlink `sysinfo` from a folder in your `$PATH` to `<path
to sysinfo repo clone>/.build/sysinfo` or add `<path to sysinfo repo
clone>/.build/sysinfo` to your `$PATH`.

Example output on Apple Silicon:

```
Processor:
	Name:                Apple M1 (64bit = true)
	CPU family:          ARM Firestorm/Icestorm (subfamily = ARM HG)
	CPU type:            ARM64 (subtype = ARM64 64E)
	Number of packages:  1
	Physical cores:      8
	Logical cores:       8
	Threads:             8
	Core type:           Performance
		    Physical:       4 (available = 4 [inactive = 0])
		    Logical:        4 (available = 4 [inactive = 0])
		    L1 data cache:  128 KB (131072 bytes)
		    L1 inst cache:  192 KB (196608 bytes)
		    L2 cache:       12 MB (12582912 bytes, shared by 4 CPUs)
	Core type:           Efficiency
		    Physical:       4 (available = 4 [inactive = 0])
		    Logical:        4 (available = 4 [inactive = 0])
		    L1 data cache:  64 KB (65536 bytes)
		    L1 inst cache:  128 KB (131072 bytes)
		    L2 cache:       4 MB (4194304 bytes, shared by 4 CPUs)
	Byte order:          Little Endian (1234)

Memory:
	Total physical:      16 GB
	Virtual addr size:   47 bits (user space = 0x0 to 0x00007fffffffffff)
	Cache as seen by current process:
		    Max cache line: 128 bytes (cache line is probably 64 KB)
		    L1 data cache:  64 KB (65536 bytes)
		    L1 inst cache:  128 KB (131072 bytes)
		    L2 cache:       4 MB (4194304 bytes)

OS:
	Version:             14.7.6
	Time base frequency: 24000000
	Page size:           16 KB (16384 bytes)
	Stack size:          16 KB (16384 bytes)
	Kernel version:      <BIG KERNEL VERSION STRING>

Online Displays:
	Display ID:          1 (main display)
		    Current res:    1680px x 1050px
		    Refresh rate:   60.0hz
		    Size:           286.39mm x 178.99mm
		    Builtin:        true
		    Active:         true
		    Asleep:         false
		    Model:          xxxxx
		    Serial:         yyyyyyyyyy
		    Vendor no:      zzzz

```

Example output on Rosetta (`arch -x86_64 sysinfo`):

> NOTE: Cores don't match up as Rosetta is an x86_64 emulation on apple silicon

```
Processor:
	Name:                Apple M1 (64bit = true)
	CPU family:          Intel Westmere (subfamily = Unknown)
	CPU type:            x86 (subtype = x86-arch1)
	Number of packages:  1
	Physical cores:      8
	Logical cores:       8
	Threads:             8
	Core type:           Performance
		    Physical:       4 (available = 4 [inactive = 0])
		    Logical:        4 (available = 4 [inactive = 0])
		    L1 data cache:  128 KB (131072 bytes)
		    L1 inst cache:  192 KB (196608 bytes)
		    L2 cache:       12 MB (12582912 bytes)
	Byte order:          Little Endian (1234)

Memory:
	Total physical:      16 GB
	Virtual addr size:   47 bits (user space = 0x0 to 0x00007fffffffffff)
	Cache as seen by current process:
		    Max cache line: 64 bytes
		    L1 data cache:  64 KB (65536 bytes)
		    L1 inst cache:  128 KB (131072 bytes)
		    L2 cache:       4 MB (4194304 bytes)

OS:
	Version:             14.7.6
	Time base frequency: 24000000
	Page size:           4 KB (4096 bytes)
	Stack size:          16 KB (16384 bytes)
	Kernel version:      <BIG KERNEL VERSION STRING>

Online Displays:
	Display ID:          1 (main display)
		    Current res:    1680px x 1050px
		    Refresh rate:   60.0hz
		    Size:           286.39mm x 178.99mm
		    Builtin:        true
		    Active:         true
		    Asleep:         false
		    Model:          xxxxx
		    Serial:         yyyyyyyyyy
		    Vendor no:      zzz
```
