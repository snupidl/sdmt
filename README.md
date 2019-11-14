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
 git clone https://github.com/snupidl/sdmt
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

  - parameter reset test
  ```
  mpirun -n 4 ./unit_test --gtest_filter=ParameterTest.1st
  mpirun -n 4 ./unit_test --gtest_filter=ParameterTest.2nd
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


## multi-level checkpoint manual
###   
FTI is a multi safety level checkpointing interface.  
SDMT provides an api which is easy to apply to enable the user select the checkpointing strategy which fits best th the problem.
###  
- ##### Definition

```
SDMT_Code checkpoint(int level)
```
- ##### Input

|  <center>Level</center> |  <center>Type</center> |  <center>Description</center> |
|:--------:|:--------:|:--------|
|**0** | <center>Multi-level checkpoint</center> |User can set checkpointing frequnecy per level in configuration file. (../checkpoint/config.fti) |
|**1** | <center>L1 checkpoint</center> |The checkpoint of each process is written on the local SSD of the respective node. This fast but possess the drawback, that in case of a data loss and corrupted checkpoint data even in only one node, the execution cannot successfully restarted.|
|**2** | <center>L2 checkpoint</center> |On initialisation, FTI creates a virtual ring for each group of nodes. The first step of L2 is just a L1 checkpoint. In the second step, the checkpoints are duplicated and the copies stored on the neighboring node in the group.That means in case of a failure and data loss in the nodes, the execution still can be successfully restrated, as long as the data loss does not happen on two neighboring nodes at the same time.|
|**3** | <center>L3 checkpoint</center> |The checkpoint data trunks from each node getting encoded via Reed-Solomon(RS) erasure code. It enables tolerate the breakdown and data loss in half of the nodes. In contrast to the L2, L3 is irrelevant which of nodes encounters the failure.|
|**4** | <center>L4 checkpoint</center> |All the checkpoint files are flushed to the parallel file system.|


- ##### Configuration file
```
...
ckpt_l1                        = 3
ckpt_l2                        = 5
ckpt_l3                        = 7
ckpt_l4                        = 11
...
```
The numeric value on the left means the freuency of each checkpoint level.




---
## Acknowledgement
This work was supported by Next-Generation Information Computing Development Program through
the National Research Foundation of Korea(NRF) funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)

Authors
- Ilju Lee, ijlee@kdb.snu.ac.kr
- Jinyon Kim, jykim@kdb.snu.ac.kr
- Hyerim Jeon, hrjeon@kdb.snu.ac.kr
