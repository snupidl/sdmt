SDMT(Simulation process and Data Management Tool) library
===
[TODO] write an abstract

---

download and install
===
[TODO] write environment, how to downdload and install

$ git clone [git address]
$ git submodule init
$ git submodule update
$ mkdir build
$ cd build
$ CXX=clang++ cmake -DCMAKE\_INSTALL\_PREFIX=/path/to/install ..
$ make
$ make install

---

examples
===
[TODO] write how to run examples

run unit test
$ cd /path/to/sdmt/build

- checkpoint test
$ ../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit\_test --gtest\_filter=RecoveryTest.\*

- restart test
$ ../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit\_test --gtest\_filter=RestartTest.1st
$ ../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit\_test --gtest\_filter=RestartTest.2nd

run python example
$ cd /path/to/sdmt/example
$ ../thirdparty/openmpi/bin/mpirun -n 4 python ./test\_recovery.py

---

Acknowledgement
===
[TODO] This work has been supprted by ~~~~~
