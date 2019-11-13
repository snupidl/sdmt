# SDMT(Simulation process and Data Management Tool) library

SDMT(Simulation Process and Data Management Tool) library provides APIs
for managing processes and data to scientific applications in
High-Performance Computing Environment. Applications can request memory
allocation for the data to be used in the simulation. The SDMT module
manages the allocated memory segment and allows to record intermediate
snapshots of the memory as persistent checkpoints in the middle of
long-running simulations. Applications can also recover the simulation
process and data from snapshots and restart iterative loop operations
as needed. In other words, the SDMT library helps to eliminate the need
to be restarted from scratch of long-running scientific applications
when a system fault occurs or parameter reset is required.

---

## download and install
* download
```
 git clone [git address]
 git submodule init
 git submodule update
```

* build and install fti library
```
 cd /path/to/sdmt/thirdparty/fti-src
 mkdir build && cd build
 cmake -DCMAKE_INSTALL_PREFIX=/path/to/sdmt/thirdparty/fti ..
 make && make install
```

* build and install sdmt

```
 cd /path/to/sdmt
 mkdir build && cd build
 cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..
 make && make install
```
---

## run unit test
### cpp test
```
cd /path/to/sdmt/build/test/cpp
```
  - checkpoint test
  ```
  mpirun -n 4 ./unit_test --gtest_filter=RecoveryTest.*
  ```

  - iteration test
  ```
  mpirun -n 4 ./unit_test --gtest_filter=IterTest.*
  ```

  - restart test
  ```
  mpirun -n 4 ./unit_test --gtest_filter=RestartTest.1st
  mpirun -n 4 ./unit_test --gtest_filter=RestartTest.2nd
  ```

### python test
```
cd /path/to/sdmt/build/test/python
```
  - checkpoint test
  ```
  mpirun -n 4 python ./test_recovery.py
  ```
  - iteration test
  ```
  mpirun -n 4 python ./test_iter.py
  ```
  - restart test
  ```
  mpirun -n 4 python ./test_restart.py 1
  mpirun -n 4 python ./test_restart.py 2
  ```
---

## run examples
###  cpp example
```
cd /path/to/sdmt/build/example/cpp
```
  - monte carlo
  ```
  mpirun -n 4 ./monte_carlo
  ```
### python example
```
cd /path/to/sdmt/build/example/python
```
  - monte carlo
  ```
  mpirun -n 4 python ./monte_carlo.py
  ```
  - mnist
  ```
  mpirun -n 4 python ./mnist.py
  ```

---
## Acknowledgement
This work was supported by Next-Generation Information Computing Development Program through
the National Research Foundation of Korea(NRF) funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)

Authors
- Ilju Lee, ijlee@kdb.snu.ac.kr
- Jinyon Kim, jykim@kdb.snu.ac.kr
- Hyerim Jeon, hrjeon@kdb.snu.ac.kr
