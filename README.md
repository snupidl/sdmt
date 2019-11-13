# SDMT(Simulation process and Data Management Tool) library
[TODO] write an abstract

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

---
## Acknowledgement
This work was supported by Next-Generation Information Computing Development Program through
the National Research Foundation of Korea(NRF) funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)

Authors
- Ilju Lee, ijlee@kdb.snu.ac.kr
- Jinyon Kim, jykim@kdb.snu.ac.kr
- Hyerim Jeon, hrjeon@kdb.snu.ac.kr
