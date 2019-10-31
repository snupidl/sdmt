SDMT(Simulation process and Data Management Tool) library
===
[TODO] write an abstract

---

download and install
===
[TODO] write environment, how to downdload and install

 `git clone [git address]`
 `git submodule init`
 `git submodule update`
 `mkdir build`
 `cd build`
 `CXX=clang++ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..`
 `make`
 `make install`

---

examples
===
[TODO] write how to run examples

run unit test
`cd /path/to/sdmt/build`

- checkpoint test
`../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit\_test --gtest\_filter=RecoveryTest.\*`

- restart test
`../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit\_test --gtest\_filter=RestartTest.1st`
`../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit\_test --gtest\_filter=RestartTest.2nd`

run python example
`cd /path/to/sdmt/example`
`../thirdparty/openmpi/bin/mpirun -n 4 python ./test\_recovery.py`

---

Acknowledgement
===
This work was supported by Next-Generation Information Computing Development Program through
the National Research Foundation of Korea(NRF) funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)

Authors
- Ilju Lee, ijlee@kdb.snu.ac.kr
- Jinyon Kim, jykim@kdb.snu.ac.kr
- Hyerim Jeon, hrjeon@kdb.snu.ac.kr
