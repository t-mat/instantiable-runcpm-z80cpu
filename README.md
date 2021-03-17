# Instantiable RunCPM/cpu.h Z80 CPU emulator

This is a modified version of `cpu.h` from [RunCPM](MockbaTheBorg/RunCPM) which emulates Z80 CPU.

The point of modifiation is `struct` (`class`) which allows to instantiate multiple CPU cores in a single process.

Since my purpose is verification of Z80 emulator, I also include [`zexall.com`](http://mdfs.net/Software/Z80/Exerciser/).


## Building

- g++

```
g++ -std=c++17 -O3 src/instantiable-runcpm-z80cpu.cpp
./a.out external/zexall/zexall.com
```

- clang++

```
clang++ -std=c++17 -O3 src/instantiable-runcpm-z80cpu.cpp
./a.out external/zexall/zexall.com
```

- VC++2019

```
.\build.bat
x64\Release\instantiable-runcpm-z80cpu.exe external\zexall\zexall.com
```
