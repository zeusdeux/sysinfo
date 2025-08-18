# sysinfo

A small tool to print system information on Apple machines.

## Quick start

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
		L2 cache:       12 MB (12582912 bytes)
	Core type:           Efficiency
		Physical:       4 (available = 4 [inactive = 0])
		Logical:        4 (available = 4 [inactive = 0])
		L1 data cache:  64 KB (65536 bytes)
		L1 inst cache:  128 KB (131072 bytes)
		L2 cache:       4 MB (4194304 bytes)
	Byte order:          Little Endian (1234)
Memory:
	Total physical:      16 GB
	Virtual addr size:   47 bits (user space = 0 to 0x00007fffffffffff)
	Cache as seen by current process:
		Max cache line: 128 bytes (but is probably 64 KB)
		L1 data cache:  64 KB (65536 bytes)
		L1 inst cache:  128 KB (131072 bytes)
		L2 cache:       4 MB (4194304 bytes)
OS:
	Time base frequency: 24000000
	Page size:           16 KB (16384 bytes)
	Stack size:          16 KB (16384 bytes)
	Kernel version:      <BIG KERNEL VERSION STRING>
```

Example output on Rosetta:

```
Processor:
	Name:                Apple M1 (64bit = true)
	CPU family:          Intel Westmere (subfamily = Unknown)
	CPU type:            x86 (subtype = Unknown)
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
	Virtual addr size:   47 bits (user space = 0 to 0x00007fffffffffff)
	Cache as seen by current process:
		Max cache line: 64 bytes (but is probably 64 KB)
		L1 data cache:  64 KB (65536 bytes)
		L1 inst cache:  128 KB (131072 bytes)
		L2 cache:       4 MB (4194304 bytes)
OS:
	Time base frequency: 24000000
	Page size:           4 KB (4096 bytes)
	Stack size:          16 KB (16384 bytes)
	Kernel version:      <BIG KERNEL VERSION STRING>
```
