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

Design
-------------
The work would pretty resemble to building an interpreter.

I use global dispatch table to implement op code handling. Each op code handlers is a lambda function object, they're stored in a map indexed by its corresponding op code
(Actually raw op codes would map to a task id first, then map to the real handlers).
I use C++11 STL's `unordered_map` to do the mapping job, which is said to have `O(1)` query performance in average cases and `O(n)` in worst cases.

This kind of dispatch table based interpreter implementation is used in many large projects nowadays.
For example, Interpreters in [V8 Javascript Engine(Ignition)](http://mshockwave.blogspot.tw/2016/03/ignition-interpreter-in-v8-javascript.html) and [Android ART](http://mshockwave.blogspot.tw/2016/03/interpreter-in-android-art.html) to name a few.
This approach can overcome some of the deadliest performance degrading(e.g. Pipeline stalling) in traditional interpreter designs
  which mostly use `switch` or `goto` to implement task dispatching.