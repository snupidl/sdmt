#ifndef ANALYZER_H
#define ANALYZER_H
#include "global_file_IO.h"
#include "graph_class.h"
#include "partitioning_strategies.h"
#include "tools.h"
#include <cstring>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <random>
#include <iostream>
#include <chrono>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "mpi.h"

/************************************
 * Information                      *
 ************************************/
// Class::engine_analyzer is designed to check the performance of the engine. 
// Especially, execution time performance, partitioning performance and the amount of the data exchangement.
// Usage is as follows.
// 1) define a class object
// 2) time_start function logs the timestamp when call this class function.
// 3) time_end, i is consumed time's type. Check the elapsed time. 
//    Type 0: local execution, Type 1: data send, Type 2: data recv, Type 3: send & recv, Type 4: waiting
// 4) add_data_amount update the data send/recv amount.

class engine_analyzer{
    private:
	friend class boost::serialization::access;
	template<class Archieve>
	void serialize(Archieve&ar, const unsigned int version)
	{
		ar& local_execution_time;
		ar& send_time;
        ar& recv_time;
        ar& send_data_amount;
        ar& recv_data_amount;
        ar& wait_time;
	}  

    public:
    long long local_execution_time;
    long long send_time;
    long long recv_time;
    long long send_data_amount;
    long long recv_data_amount;
    long long wait_time;
    //std::chrono::time_point temp_time;

    engine_analyzer(){
        local_execution_time =0;
        send_time=0;
        recv_time=0;
        send_data_amount=0;
        recv_data_amount=0;
        wait_time=0;
    }
    void time_start(std::chrono::high_resolution_clock::time_point & temp_time){
        temp_time = std::chrono::high_resolution_clock::now();
    }
    void time_end(int i, std::chrono::high_resolution_clock::time_point & temp_time){
        auto elapsed = std::chrono::high_resolution_clock::now() - temp_time;   
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();  
        if(i==0){
            local_execution_time += microseconds;
        }
        else if(i==1){
            send_time += microseconds;
        }
        else if(i==2){
            recv_time += microseconds;
        }
        else if(i==3){
            send_time += microseconds/2;
            recv_time += microseconds/2;
        }
        else if(i==4){
            wait_time += microseconds;
        }
    }
    void add_data_amount(int i, int j){
        if(i==0){
            send_data_amount += (long long)j;
        }
        else if(i==1){
            recv_data_amount += (long long)j;
        }
        else if(i==2){
            send_data_amount += (long long)j;
            recv_data_amount += (long long)j;
        }
    }
    void all_update(int time_case, int data_case, int data_amount, std::chrono::high_resolution_clock::time_point & temp_time){
        time_end(time_case, temp_time);
        add_data_amount(data_case, data_amount);
    }
    void init(){
        local_execution_time =0;
        send_time=0;
        recv_time=0;
        send_data_amount=0;
        recv_data_amount=0;
        wait_time = 0;
    }
};

#endif