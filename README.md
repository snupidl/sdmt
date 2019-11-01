SDMT(Simulation process and Data Management Tool) library
===
[TODO] write an abstract

---

download and install
===
[TODO] write environment, how to downdload and install

- download
```
 git clone [git address]
 git submodule init
 git submodule update
```

- build and install fti library
```
 cd /path/to/sdmt/thirdparty/fti-src
 mkdir build && cd build
 cmake -DCMAKE_INSTALL_PREFIX=/path/to/sdmt/thirdparty/fti ..
 make && make install
```

- build and install sdmt

```
 cd /path/to/sdmt
 mkdir build && cd build
 cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..
 make && make install
 
```
---

examples
===
[TODO] write how to run examples

run unit test
```
cd /path/to/sdmt/build
```

- checkpoint test
```
../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit_test --gtest_filter=RecoveryTest.*
```

- restart test
```
../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit_test --gtest_filter=RestartTest.1st
../thirdparty/openmpi/bin/mpirun -n 4 ./test/unit_test --gtest_filter=RestartTest.2nd
```

run example
```
cd /path/to/sdmt/example
```

- checkpoint test
```
../thirdparty/openmpi/bin/mpirun -n 4 python ./test_recovery.py
```

- restart test
```
../thirdparty/openmpi/bin/mpirun -n 4 python ./test_restart.py 1
../thirdparty/openmpi/bin/mpirun -n 4 python ./test_restart.py 2
```

---

Acknowledgement
===
This work was supported by Next-Generation Information Computing Development Program through
the National Research Foundation of Korea(NRF) funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)

Authors
- Ilju Lee, ijlee@kdb.snu.ac.kr
- Jinyon Kim, jykim@kdb.snu.ac.kr
- Hyerim Jeon, hrjeon@kdb.snu.ac.kr
