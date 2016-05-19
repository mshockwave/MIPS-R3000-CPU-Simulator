MIPS R3000 CPU Simulator
=====================
This is a series of homework in one of the Computer Architecture courses(10420CS 410001) in [CS department of National Tsing Hua University](http://web.cs.nthu.edu.tw/).

The first homework would implement a single cycle MIPS CPU simulator.
We use [reduced set of MIPS R3000 ISA](doc/MIPS_R3000_ISA.pdf).<br/>
The tests/test_dataset folder contains subdirectories of test binary files.

Build
-------------
This project use **CMake**, the minimum version required is 2.8.
The main programming language is C++11.
* Initialize cmake files:<br/>
  `mkdir build`<br/>
  `cd build`<br/>
  `cmake -DCMAKE_BUILD_TYPE=<Debug/Release> ..`<br/>
  `Release` build type use `-Os` compiler optimization flag since `-O3` is proved to break the programs execution logic.

* Install test data folders:<br/>
  `make install-test-data`<br/>
  **Note: `make clean` would also clean these test data folders**

* Build main program:<br/>
  `make ArchiHW1`

* Build unit tests main program:<br/>
  `make ArchiHW1Tests`

* Or just build all stuff:<br/>
  `make`

Usage
-----
* `./ArchiHW1 <Path of folder contains test binaries>`<br/>
  E.g. `./ArchiHW1 test_dataset/func` Where test_dataset/func contains the desired binary files: `iimage.bin` and `dimage.bin`

* `./ArchiHW1` would find those two binary files in current path.

The result output files, `snapshot.rpt` and `error_dump.rpt`, would put in current path.

