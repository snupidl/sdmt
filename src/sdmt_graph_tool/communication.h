#ifndef COMMUNICATION_H
#define COMMUNICATION_H

/************************************
 * Information                      *
 ************************************/
// This file (communication.h) contains some "worker-worker", "worker-other all workers", "all worker-all worker" communication functions.
// For the easy-usage, This file adopts boost archive, serialization and etc.
// Basically, our communcation follows the GAS(*1) (Gather, Apply, Scatter) distributed computation model.
// (*1): Gonzalez, Joseph E., et al. "Powergraph: Distributed graph-parallel computation on natural graphs." 10th {USENIX} Symposium on Operating Systems Design and Implementation ({OSDI} 12). 2012.

#include "global_file_IO.h"
#include "graph_class.h"
#include "partitioning_strategies.h"
#include "global_graph_stat.h"
#include "analyzer.h"
#include <cstring>
#include <string>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <list>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include "mpi.h"
using namespace std;

//Request type
#define GATHER_REQUEST      0
#define GATHER_RESPONSE     1
#define APPLY_DATAPASS      2
#define SCATTER_REQUEST     3
#define SCATTER_RESPONSE    4
#define DATAPASS            5
#define STOP                6
#define AGGREGATE_DATAPASS  7
#define GATHER_ON           8
#define DATA_EXCHANGE       9

//Operation type (value type)
#define SUM         100
#define MULTIPLY    101
#define MAX         104
#define MIN         105
#define MAXLOC      106
#define MINLOC      107

//Operation type (set type)
#define UNION       108

//Operation type (vector type)
#define MERGE       109

//Operation type (bit type)
#define OR          102
#define AND         103

//Neighbor type
#define IN_NEI      1000
#define OUT_NEI     1001
#define BOTH_NEI    1002


#define NUM_OF_All_MESSAGES 1024
#define NUM_OF_All_MESSAGES_8 8

// exchange unit 1
template<typename T1, typename T2>
struct tuple_vertex_value{
    T1 vertex;
    T2 value;
};

// exchange unit 2
template<typename T1, typename T2>
class tuple_vertex_vector_value{
private:
	friend class boost::serialization::access;
	template<class Archieve>
	void serialize(Archieve&ar, const unsigned int version)
	{
		ar& vertex;
		ar& values;
	}  
public:
    T1 vertex;
    vector<T2> values;

    
    tuple_vertex_vector_value(){
        vertex = -1;
        values = vector<T2>();
    }

    // input: vertex_ = ID of a vertex 
    // set the vertex ID to vertex_
    void set_vertex(T1 vertex_){
        vertex = vertex_;
    }

    // input: values_ = some features/properties vector
    // set the values to values_
    void set_values(vector<T2> & values_){
        values = values_;
    }
};

// exchange unit 0's class type
template<typename T1, typename T2>
class tuple_vertex_value_all{
private:
	friend class boost::serialization::access;
	template<class Archieve>
	void serialize(Archieve&ar, const unsigned int version)
	{
		ar& vertex;
		ar& value;
	}  
public:
    T1 vertex;
    T2 value;

    // input: vertex_ = ID of a vertex 
    // set the vertex ID to vertex_
    void set_vertex(T1 vertex_){
        vertex = vertex_;
    }

    // input: values_ = some features/properties vector
    // set the value of the vertex to value_
    void set_values(T2 value_){
        value = value_;
    }
};

// simple function for send the vector send_vec to to_node.
// input:
//  *) send_vec: A pointer of the vector to send.
//  *) to_node: The worker ID that receive the data.
template<typename T1>
void send_vector(vector<T1>* send_vec, int to_node) {
    MPI_Send((void *) send_vec->data(),  send_vec->size() * sizeof(T1), MPI_BYTE, to_node, 0, MPI_COMM_WORLD);
}

// simple function for recv the vector to recv_vec.
// input:
//  *) recv_vec: A pointer of the vector to recv.
//  *) from_node: The worker ID that send the data.
//  *) my_rank: ID of the worker who called this function
//  *) recv_size: the size of the vector to receive
template<typename T1>
void recv_vector(vector<T1>* recv_vec, int from_node, int my_rank, int recv_size) {
    recv_vec -> resize(recv_size);
    MPI_Recv((void *)recv_vec->data(),  recv_size*sizeof(T1), MPI_BYTE, from_node, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

// activated vertex's value will be aggregated.
// method pick the aggregation type. (SUM, MULTIPLY, MIN, MAX, ...)
// the aggregated output is pushed to vertex_out_data.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) method: Operation type(SUM, MAX, MIN, ..., please check the #define part in the front of this file)
//  *) activated: Activated workers
//  *) vertex_in_data: input data
//  *) (out) vertex_out_data: output data
template<typename T1, typename T2>
void SSSP_aggregate_slave_to_master_only_activate(const int my_rank, const int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
                                                  const int method, set<T1> * activated, map<T1, T2> * vertex_in_data, map<T1, T2> * vertex_out_data){
    // A function that simply transfers something from the master node to the slave.
    //init
    MPI_Request request;
    T1 temp_vertex;
    T2 temp_value;
    T1 local_finish;
    T1 finished_num;
    MPI_Status temp_status;
    int count;
    int flag;
    int sendflag;
    int max_receive = 0;
    int max_send = 0; 
    for(auto iter = activated->begin(); iter!=activated->end(); iter++){
        auto iter2 = master_to_slave->find(*iter);
        auto iter3 = slave_to_master->find(*iter);
        if(iter2!=master_to_slave->end()){
            max_receive += iter2->second.size();
        }
        if(iter3!=slave_to_master->end()){
            max_send += 1;
        }
    }
    int this_receive = 0;
    int this_send = 0;
    auto out_iter = activated->begin();
    int send_check = 0;
    tuple_vertex_value<T1, T2> receive_tu;
    tuple_vertex_value<T1, T2> send_tu;
    //main
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, AGGREGATE_DATAPASS, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==AGGREGATE_DATAPASS)
            {   
                MPI_Recv(&receive_tu, sizeof(tuple_vertex_value<T1, T2>), MPI_BYTE, temp_status.MPI_SOURCE, AGGREGATE_DATAPASS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                this_receive++;
                switch(method)
                {
                    case SUM:{
                        (*vertex_out_data)[receive_tu.vertex] += receive_tu.value;
                        break;
                    }
                    case MULTIPLY:{
                        (*vertex_out_data)[receive_tu.vertex] *= receive_tu.value;
                        break;
                    }
                    case MAX:{
                        if((*vertex_out_data)[receive_tu.vertex]<receive_tu.value){
                            (*vertex_out_data)[receive_tu.vertex] = receive_tu.value;
                        }
                        break;
                    }
                    case MIN:{
                        if((*vertex_out_data)[receive_tu.vertex]>receive_tu.value){
                            (*vertex_out_data)[receive_tu.vertex] = receive_tu.value;
                        }
                        break;
                    }
                    default:{
                        cout << "Unknown Tag in aggregate_SSSP method" << endl;
                    }
                }
            }
            else{
                cout << "Unknown Tag in aggregate_SSSP tag" << endl;
                cout<< "the tag: "<<temp_status.MPI_TAG<<endl;
            }
        }
        else{
            if(send_check ==1){
                MPI_Test(&request, &sendflag, MPI_STATUS_IGNORE);
                if(sendflag){
                    this_send++;
                    send_check = 0;
                }
                else{
                    continue;
                }
            }
            if(send_check == 0 && out_iter==activated->end()){
                if(this_send==max_send && this_receive==max_receive){
                    break;
                }
                continue;
            }

            if(master_to_slave->find(*out_iter)!=master_to_slave->end()){
                out_iter++;
                continue;
            }
            if(slave_to_master->find(*out_iter)==slave_to_master->end()){
                out_iter++;
                continue;
            }
            send_tu.vertex = *out_iter;
            send_tu.value = (*vertex_in_data)[send_tu.vertex]; 
            MPI_Isend(&send_tu, sizeof(tuple_vertex_value<T1, T2>), MPI_BYTE, slave_to_master->at(*out_iter), AGGREGATE_DATAPASS, MPI_COMM_WORLD, &request);
            
            out_iter++;
            send_check = 1;
            
        }
    }
}

// activated vertex's value will be aggregated.
// more fast version
// method pick the aggregation type. (SUM, MULTIPLY, MIN, MAX, ...)
// the aggregated output is pushed to vertex_out_data.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) method: Operation type(SUM, MAX, MIN, ..., please check the #define part in the front of this file)
//  *) activated: Activated workers
//  *) vertex_in_data: input data
//  *) (out) vertex_out_data: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void SSSP_aggregate_slave_to_master_only_activate_more_good(const int my_rank, const int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
                                                  const int method, set<T1> * activated, map<T1, T2> * vertex_in_data, map<T1, T2> * vertex_out_data,
                                                  engine_analyzer & my_analyzer){
    // A function that simply transfers something from the master node to the slave.
    //init
    T1 temp_vertex;
    T2 temp_value;
    T1 local_finish;
    T1 finished_num;
    MPI_Status temp_status;
    int count;
    int flag;
    int max_receive = 0; 
    int max_send = 0; 
    for(auto iter = activated->begin(); iter!=activated->end(); iter++){
        auto iter2 = master_to_slave->find(*iter);
        auto iter3 = slave_to_master->find(*iter);
        if(iter2!=master_to_slave->end()){
            max_receive += iter2->second.size();
            (*vertex_out_data)[*iter] = (*vertex_in_data)[*iter];
        }
        if(iter3!=slave_to_master->end()){
            max_send += 1;
        }
    }
    int this_receive = 0;
    int this_send = 0;
    auto out_iter = activated->begin();
    int send_check = 0;
    vector<int> send_flags(NUM_OF_All_MESSAGES, 1);
    vector<tuple_vertex_value<T1, T2>> send_datas(NUM_OF_All_MESSAGES);
    vector<MPI_Request> requests(NUM_OF_All_MESSAGES);
    queue<int> send_indices;
    list<int> off_indices;
    int send_slot;
    int send_slot_out;
    for(int i = 0; i<NUM_OF_All_MESSAGES; i++){
        send_indices.push(i);
    }
    tuple_vertex_value<T1, T2> receive_tu;
    std::chrono::high_resolution_clock::time_point temp_time;
    //main
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, AGGREGATE_DATAPASS, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==AGGREGATE_DATAPASS)
            {   
                my_analyzer.time_start(temp_time);
                MPI_Recv(&receive_tu, sizeof(tuple_vertex_value<T1, T2>), MPI_BYTE, temp_status.MPI_SOURCE, AGGREGATE_DATAPASS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_analyzer.add_data_amount(1, sizeof(tuple_vertex_value<T1, T2>));
                this_receive++;
                switch(method)
                {
                    case SUM:{
                        (*vertex_out_data)[receive_tu.vertex] += receive_tu.value;
                        break;
                    }
                    case MULTIPLY:{
                        (*vertex_out_data)[receive_tu.vertex] *= receive_tu.value;
                        break;
                    }
                    case MAX:{
                        if((*vertex_out_data)[receive_tu.vertex]<receive_tu.value){
                            (*vertex_out_data)[receive_tu.vertex] = receive_tu.value;
                        }
                        break;
                    }
                    case MIN:{
                        if((*vertex_out_data)[receive_tu.vertex]>receive_tu.value){
                            (*vertex_out_data)[receive_tu.vertex] = receive_tu.value;
                        }
                        break;
                    }
                    default:{
                        cout << "Unknown Tag in aggregate_SSSP method" << endl;
                    }
                }
                my_analyzer.time_end(2, temp_time);
            }
            else{
                cout << "Unknown Tag in aggregate_SSSP tag" << endl;
                cout<< "the tag: "<<temp_status.MPI_TAG<<endl;
            }
        }
        else{
            if(!off_indices.empty()){
                auto iter = off_indices.begin();
                while (iter != off_indices.end()){
                    MPI_Test(&(requests[*iter]), &(send_flags[*iter]), MPI_STATUS_IGNORE);
                    if(send_flags[*iter]){
                        this_send++;
                        send_slot_out = *iter;
                        off_indices.erase(iter++);
                        send_indices.push(send_slot_out);
                    }
                    else{
                        ++iter;
                    }
                }
            }
            if(send_check == 0 && out_iter==activated->end()){
                if(this_send==max_send && this_receive==max_receive){
                    break;
                }
                continue;
            }

            if(master_to_slave->find(*out_iter)!=master_to_slave->end()){
                out_iter++;
                continue;
            }
            if(slave_to_master->find(*out_iter)==slave_to_master->end()){
                out_iter++;
                continue;
            }
            if(!send_indices.empty()){
                my_analyzer.time_start(temp_time);
                send_slot = send_indices.front();
                send_indices.pop();
                off_indices.push_back(send_slot);
                send_datas[send_slot].vertex = *out_iter;
                send_datas[send_slot].value = (*vertex_in_data)[*out_iter];
                MPI_Isend(&(send_datas[send_slot]), sizeof(tuple_vertex_value<T1, T2>), MPI_BYTE, slave_to_master->at(*out_iter), AGGREGATE_DATAPASS, MPI_COMM_WORLD, &(requests[send_slot]));
                my_analyzer.add_data_amount(0, sizeof(tuple_vertex_value<T1, T2>));
                
                out_iter++;
                my_analyzer.time_end(1, temp_time);
            }            
        }
    }
}

// activated vertex's values (vector) will be aggregated.
// method is fixed to MERGE mode.
// the aggregated output is pushed to vertex_out_data.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) method: Operation type(SUM, MAX, MIN, ..., please check the #define part in the front of this file)
//  *) activated: Activated workers
//  *) vertex_in_data: input data
//  *) (out) vertex_out_data: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void SSSP_aggregate_vector_slave_to_master_only_activate_more_good(const int my_rank, const int nproc, 
    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
    const int method, set<T1> * activated, map<T1, vector<T2>> * vertex_in_data, map<T1, vector<T2>> * vertex_out_data,
    engine_analyzer & my_analyzer){

    T1 temp_vertex;
    T2 temp_value;
    T1 local_finish;
    T1 finished_num;
    MPI_Status temp_status;
    int count;
    int flag;
    int max_receive = 0;
    int max_send = 0; 
    for(auto iter = activated->begin(); iter!=activated->end(); iter++){
        auto iter2 = master_to_slave->find(*iter);
        auto iter3 = slave_to_master->find(*iter);
        if(iter2!=master_to_slave->end()){
            max_receive += iter2->second.size();
            if(vertex_out_data->find(*iter)==vertex_out_data->end()){
                (*vertex_out_data)[*iter] = vector<T2>();
            }
            if((*vertex_in_data)[*iter].size()>0){
                (*vertex_out_data)[*iter].insert(std::end((*vertex_out_data)[*iter]), 
                                             std::begin((*vertex_in_data)[*iter]), std::end((*vertex_in_data)[*iter])
                                            );
            }
        }
        if(iter3!=slave_to_master->end()){
            max_send += 1;
            if(vertex_in_data->find(*iter)==vertex_in_data->end()){
                (*vertex_in_data)[*iter] = vector<T2>();
            }
        }
    }
    int this_receive = 0;
    int this_send = 0;
    auto out_iter = activated->begin();
    int send_check = 0;
    vector<int> send_flags(NUM_OF_All_MESSAGES_8, 1);
    vector<std::string> send_datas(NUM_OF_All_MESSAGES_8);
    vector<MPI_Request> requests(NUM_OF_All_MESSAGES_8);
    queue<int> send_indices;
    list<int> off_indices;
    int send_slot;
    int send_slot_out;
    for(int i = 0; i<NUM_OF_All_MESSAGES_8; i++){
        send_indices.push(i);
    }
    std::chrono::high_resolution_clock::time_point temp_time;
    
    //main
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, AGGREGATE_DATAPASS, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==AGGREGATE_DATAPASS)
            {   
                tuple_vertex_vector_value<T1, T2> receive_tu;
                int number_amount =0;
                my_analyzer.time_start(temp_time);
                MPI_Get_count(&temp_status, MPI_CHAR, &number_amount);
                char * recv_string = new char[number_amount];
                MPI_Recv(recv_string, number_amount, MPI_CHAR, temp_status.MPI_SOURCE, AGGREGATE_DATAPASS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                string recv_to_string(recv_string, number_amount);
                my_analyzer.add_data_amount(1, sizeof(char)*number_amount);
                boost::iostreams::basic_array_source<char> device(recv_to_string.data(), number_amount);
                boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s(device);
                boost::archive::binary_iarchive ia(s);
                ia >> receive_tu;
                this_receive++;
                switch(method)
                {
                    case MERGE:{
                        if(receive_tu.values.size()>0){
                            (*vertex_out_data)[receive_tu.vertex].insert(std::end((*vertex_out_data)[receive_tu.vertex]), 
                                                                     std::begin(receive_tu.values), std::end(receive_tu.values)
                                                                    );
                        }
                        break;
                    }
                    default:{
                        cout << "Unknown Tag in aggregate_SSSP method" << endl;
                    }
                }
                delete [] recv_string;
                my_analyzer.time_end(2, temp_time);
            }
            else{
                cout << "Unknown Tag in aggregate_SSSP tag" << endl;
                cout<< "the tag: "<<temp_status.MPI_TAG<<endl;
            }
        }
        else{
            if(!off_indices.empty()){
                auto iter = off_indices.begin();
                while (iter != off_indices.end()){
                    MPI_Test(&(requests[*iter]), &(send_flags[*iter]), MPI_STATUS_IGNORE);
                    if(send_flags[*iter]){
                        this_send++;
                        send_slot_out = *iter;
                        off_indices.erase(iter++);
                        send_indices.push(send_slot_out);
                    }
                    else{
                        ++iter;
                    }
                }
            }
            if(send_check == 0 && out_iter==activated->end()){
                if(this_send==max_send && this_receive==max_receive){
                    break;
                }
                continue;
            }

            if(master_to_slave->find(*out_iter)!=master_to_slave->end()){
                out_iter++;
                continue;
            }
            if(slave_to_master->find(*out_iter)==slave_to_master->end()){
                out_iter++;
                continue;
            }
            if(!send_indices.empty()){
                tuple_vertex_vector_value<T1, T2> send_tu;
                my_analyzer.time_start(temp_time);
                send_slot = send_indices.front();
                send_indices.pop();
                off_indices.push_back(send_slot);
                send_datas[send_slot].clear();
                send_datas[send_slot]="";
                send_tu.vertex = *out_iter;
                if((*vertex_in_data)[send_tu.vertex].size()>0){
                    for(int i = 0; i<(*vertex_in_data)[send_tu.vertex].size(); i++){
                        send_tu.values.push_back((*vertex_in_data)[send_tu.vertex][i]);
                    }
                }
                else{
                    send_tu.values = vector<T2>();
                }
                boost::iostreams::back_insert_device<std::string> inserter(send_datas[send_slot]);
                boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
                boost::archive::binary_oarchive oa(s);
                oa << send_tu;
                s.flush();
                MPI_Isend(send_datas[send_slot].c_str(), send_datas[send_slot].size(), MPI_CHAR, slave_to_master->at(*out_iter), AGGREGATE_DATAPASS, MPI_COMM_WORLD, &(requests[send_slot]));
                my_analyzer.add_data_amount(0, sizeof(char)*(send_datas[send_slot].size()));
                
                out_iter++;
                my_analyzer.time_end(1, temp_time);
            }
                   
        }
    }
}

// activated vertex's value is applied to other workers.
// the applied output is updated to vertex_out_data.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) changed_vec: vertices that need updating 
//  *) (out) vertex_to_output: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void SSSP_apply_active(const int my_rank, const int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
                       vector<T1> * changed_vec, map<T1, T2> * vertex_to_output, engine_analyzer & my_analyzer){
    // A function that simply transfers something from the master node to the slave.
    //init
    MPI_Request * requests;
    T1 temp_vertex;
    T1 local_finish;
    T1 finished_num;
    MPI_Status temp_status;
    int count;
    int flag;
    int max_send = 0; 
    int max_receive = 0; 
    T1 changed_ver;
    for(int i = 0; i<changed_vec->size(); i++){
        changed_ver = (*changed_vec)[i];
        auto mas = master_to_slave->find(changed_ver);
        if(mas!=master_to_slave->end()){
            max_send += mas->second.size();
        }
        if(slave_to_master->find(changed_ver)!=slave_to_master->end()){
            max_receive += 1;
        }
    }
    int this_receive = 0;
    int this_send = 0;
    int changed_vec_idx = 0;
    int send_check = 0;
    int wait_size;
    int send_flag;
    std::chrono::high_resolution_clock::time_point temp_time;
    tuple_vertex_value<T1, T2> send_tu;
    //main
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, APPLY_DATAPASS, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==APPLY_DATAPASS){
                my_analyzer.time_start(temp_time);
                tuple_vertex_value<T1, T2> tu;
                MPI_Recv(&tu, sizeof(tuple_vertex_value<T1, T2>), MPI_BYTE, temp_status.MPI_SOURCE, APPLY_DATAPASS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_analyzer.add_data_amount(1, sizeof(tuple_vertex_value<T1, T2>));
                this_receive++;
                (*vertex_to_output)[tu.vertex] = tu.value;
                my_analyzer.time_end(2, temp_time);
            }
            else{
                cout << "Unknown Tag in APPLY" << endl;
            }
        }
        else{
            if(send_check==1){
                MPI_Testall(wait_size, requests, &send_flag, MPI_STATUSES_IGNORE);
                if(send_flag){
                    this_send = this_send + wait_size;
                    delete [] requests;
                    send_check = 0;
                }
                continue;
            }
            if(changed_vec_idx==changed_vec->size()){
                if(this_send==max_send && this_receive==max_receive){
                    break;
                }
                else{
                    continue;
                }
            }
            auto t_iter = master_to_slave->find((*changed_vec)[changed_vec_idx]);
            if(t_iter==master_to_slave->end()){
                changed_vec_idx++;
                continue;
            }
            else if(t_iter->second.size()<=0){
                changed_vec_idx++;
                continue;
            }
            

            my_analyzer.time_start(temp_time);
            send_tu.vertex = (*changed_vec)[changed_vec_idx];
            send_tu.value = (*vertex_to_output)[send_tu.vertex];
            wait_size =  (t_iter->second).size();
            requests = new MPI_Request[wait_size];
            for(int i = 0; i<wait_size; i++){
                MPI_Isend(&send_tu, sizeof(tuple_vertex_value<T1, T2>), MPI_BYTE, (t_iter->second)[i], APPLY_DATAPASS, MPI_COMM_WORLD, &(requests[i]));
                my_analyzer.add_data_amount(0, sizeof(tuple_vertex_value<T1, T2>));
            }
            send_check = 1;
            changed_vec_idx++;
            my_analyzer.time_end(1, temp_time);
        }
    }
}

// vertex's values (vector) are applied to other workers.
// the applied output is updated to vertex_out_data.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) changed_vec: vertices that need updating 
//  *) (out) vertex_to_output: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void SSSP_apply_vector_active(const int my_rank, const int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
                       vector<T1> * changed_vec, map<T1, vector<T2>> * vertex_to_output, engine_analyzer & my_analyzer){
    // A function that simply transfers something from the master node to the slave.
    //init
    MPI_Request * requests;
    T1 temp_vertex;
    T1 local_finish;
    T1 finished_num;
    MPI_Status temp_status;
    int count;
    int flag;
    int max_send = 0; 
    int max_receive = 0; 
    T1 changed_ver;
    for(int i = 0; i<changed_vec->size(); i++){
        changed_ver = (*changed_vec)[i];
        auto mas = master_to_slave->find(changed_ver);
        if(mas!=master_to_slave->end()){
            max_send += mas->second.size();
        }
        if(slave_to_master->find(changed_ver)!=slave_to_master->end()){
            max_receive += 1;
        }
    }
    int this_receive = 0;
    int this_send = 0;
    int changed_vec_idx = 0;
    int send_check = 0;
    int wait_size;
    tuple_vertex_vector_value<T1, T2> receive_tu;
    tuple_vertex_vector_value<T1, T2> send_tu;
    std::chrono::high_resolution_clock::time_point temp_time;
    std:string send_string;
    int number_amount;
    int send_flag;
    //main
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, APPLY_DATAPASS, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==APPLY_DATAPASS){
                my_analyzer.time_start(temp_time);
                MPI_Get_count(&temp_status, MPI_CHAR, &number_amount);
                char * recv_string = new char[number_amount];
                MPI_Recv(recv_string, number_amount, MPI_CHAR, temp_status.MPI_SOURCE, APPLY_DATAPASS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_analyzer.add_data_amount(1, sizeof(char)*number_amount);
                boost::iostreams::basic_array_source<char> device(recv_string, number_amount);
                boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
                boost::archive::binary_iarchive ia(s);
                ia >> receive_tu;
                this_receive++;
                (*vertex_to_output)[receive_tu.vertex] = vector<T2>();
                (*vertex_to_output)[receive_tu.vertex].insert(std::end((*vertex_to_output)[receive_tu.vertex]), 
                                                                     std::begin(receive_tu.values), std::end(receive_tu.values)
                                                                    );
                delete [] recv_string;
                my_analyzer.time_end(2, temp_time);
            }
            else{
                cout << "Unknown Tag in APPLY" << endl;
            }
        }
        else{
            if(send_check==1){
                MPI_Testall(wait_size, requests, &send_flag, MPI_STATUSES_IGNORE);
                if(send_flag){
                    this_send = this_send+wait_size;
                    delete [] requests;
                    send_check = 0;
                }
                continue;
            }
            if(changed_vec_idx==changed_vec->size()){
                if(this_send==max_send && this_receive==max_receive){
                    break;
                }
                else{
                    continue;
                }
            }
            auto t_iter = master_to_slave->find((*changed_vec)[changed_vec_idx]);
            if(t_iter==master_to_slave->end()){
                changed_vec_idx++;
                continue;
            }
            else if(t_iter->second.size()<=0){
                changed_vec_idx++;
                continue;
            }
            my_analyzer.time_start(temp_time);
            send_string="";
            boost::iostreams::back_insert_device<std::string> inserter(send_string);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
            boost::archive::binary_oarchive oa(s);
            send_tu.vertex = (*changed_vec)[changed_vec_idx];
            send_tu.values = (*vertex_to_output)[send_tu.vertex];
            oa << send_tu;
            s.flush();

            wait_size =  (t_iter->second).size();
            requests = new MPI_Request[wait_size];
            for(int i = 0; i<wait_size; i++){
                MPI_Isend(send_string.c_str(), send_string.size(), MPI_CHAR, (t_iter->second)[i], APPLY_DATAPASS, MPI_COMM_WORLD, &(requests[i]));
                my_analyzer.add_data_amount(1, sizeof(char)*(send_string.size()));
            }
            send_check = 1;
            changed_vec_idx++;
            my_analyzer.time_end(1, temp_time);
        }
    }
}

// activated vertex's values (vector) are applied to other workers.
// the applied output is updated to vertex_out_data.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) changed_set: vertices that need updating 
//  *) (out) vertex_to_output: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void SSSP_apply_vector_active(const int my_rank, const int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
                       set<T1> * changed_set, map<T1, vector<T2>> * vertex_to_output, engine_analyzer & my_analyzer){
    // A function that simply transfers something from the master node to the slave.
    //init
    MPI_Request * requests;
    T1 temp_vertex;
    T1 local_finish;
    T1 finished_num;
    MPI_Status temp_status;
    int count;
    int flag;
    int max_send = 0; 
    int max_receive = 0; 
    T1 changed_ver;
    for(auto iter = changed_set->begin(); iter!=changed_set->end(); iter++){
        changed_ver = *iter;
        auto mas = master_to_slave->find(changed_ver);
        if(mas!=master_to_slave->end()){
            max_send += mas->second.size();
        }
        if(slave_to_master->find(changed_ver)!=slave_to_master->end()){
            max_receive += 1;
        }
    }
    int this_receive = 0;
    int this_send = 0;
    int send_check = 0;
    int wait_size;
    
    
    std:string send_string;
    int number_amount;
    int send_flag;
    auto iter = changed_set->begin();
    std::chrono::high_resolution_clock::time_point temp_time;
    //main
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, APPLY_DATAPASS, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==APPLY_DATAPASS){
                tuple_vertex_vector_value<T1, T2> receive_tu;
                my_analyzer.time_start(temp_time);
                MPI_Get_count(&temp_status, MPI_CHAR, &number_amount);
                char * recv_string = new char[number_amount];
                MPI_Recv(recv_string, number_amount, MPI_CHAR, temp_status.MPI_SOURCE, APPLY_DATAPASS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_analyzer.add_data_amount(1, sizeof(char)*number_amount);
                boost::iostreams::basic_array_source<char> device(recv_string, number_amount);
                boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
                boost::archive::binary_iarchive ia(s);
                ia >> receive_tu;
                this_receive++;
                (*vertex_to_output)[receive_tu.vertex] = vector<T2>();
                for(int i = 0; i<receive_tu.values.size(); i++){
                    (*vertex_to_output)[receive_tu.vertex].push_back(receive_tu.values[i]);
                }
                delete [] recv_string;
                my_analyzer.time_end(2, temp_time);
            }
            else{
                cout << "Unknown Tag in APPLY" << endl;
            }
        }
        else{
            if(send_check==1){
                MPI_Testall(wait_size, requests, &send_flag, MPI_STATUSES_IGNORE);
                if(send_flag){
                    this_send = this_send+wait_size;
                    delete [] requests;
                    send_check = 0;
                }
                continue;
            }
            if(iter == changed_set->end()){
                if(this_send==max_send && this_receive==max_receive){
                    break;
                }
                else{
                    continue;
                }
            }
            auto t_iter = master_to_slave->find(*iter);
            if(t_iter==master_to_slave->end()){
                iter++;
                continue;
            }
            else if(t_iter->second.size()<=0){
                iter++;
                continue;
            }

            my_analyzer.time_start(temp_time);
            send_string.clear();
            boost::iostreams::back_insert_device<std::string> inserter(send_string);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
            boost::archive::binary_oarchive oa(s);
            tuple_vertex_vector_value<T1, T2> send_tu;
            send_tu.vertex = *iter;
            send_tu.values = (*vertex_to_output)[send_tu.vertex];
            oa << send_tu;
            s.flush();

            wait_size =  (t_iter->second).size();
            requests = new MPI_Request[wait_size];
            for(int i = 0; i<wait_size; i++){
                MPI_Isend(send_string.c_str(), send_string.size(), MPI_CHAR, (t_iter->second)[i], APPLY_DATAPASS, MPI_COMM_WORLD, &(requests[i]));
                my_analyzer.add_data_amount(0, sizeof(char)*(send_string.size()));
            }
            send_check = 1;
            iter++;
            my_analyzer.time_end(1, temp_time);
        }
    }
}

/*
 *  local_func()
 */
/*
 * in gather phase, each machine run local function.
 * after that aggregate each value.
 */
// input:
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) input_set: activated vertices
//  *) rank: ID of the worker who called this function
//  *) size: The number of all workers
//  *) aggregation_method: Operation type(SUM, MAX, MIN, ..., please check the #define part in the front of this file)
//  *) f: local function
//  *) (out) vertex_out_data: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void gather_phase(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, set<T1> * input_set,
                  const int rank, const int size, const int aggregation_method, boost::function<void*() > f, map<T1, T2> * vertex_out_data,
                  engine_analyzer & my_analyzer){
    // local phase
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    map<T1, T2> * local_output = (map<T1, T2> *)f();
    my_analyzer.time_end(0, temp_time);
    // Aggregate
    SSSP_aggregate_slave_to_master_only_activate_more_good(rank, size, slave_to_master, master_to_slave, 
                                                 aggregation_method, input_set, local_output, vertex_out_data, my_analyzer);
    delete local_output;
}

/*
 * in gather phase, each machine run local function.
 * after that aggregate each vector value.
 */
// input:
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) input_set: activated vertices
//  *) rank: ID of the worker who called this function
//  *) size: The number of all workers
//  *) aggregation_method: Operation type(SUM, MAX, MIN, ..., please check the #define part in the front of this file)
//  *) f: local gather computation function
//  *) (out) vertex_out_data: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void gather_vector_phase(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, set<T1> * input_set,
                         const int rank, const int size, const int aggregation_method, boost::function<void*() > f, map<T1, vector<T2>> * vertex_out_data,
                         engine_analyzer & my_analyzer){
    // local phase
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    map<T1, vector<T2>> * local_output = (map<T1, vector<T2>> *)f();
    my_analyzer.time_end(0, temp_time);
    // Aggregate
    SSSP_aggregate_vector_slave_to_master_only_activate_more_good(rank, size, 
        slave_to_master, master_to_slave, 
        aggregation_method, input_set, local_output, vertex_out_data, my_analyzer);
    delete local_output;
}


/*
 * apply function (simple wrapper function)
 */
// input:
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) update_id: vertices to be updated
//  *) rank: ID of the worker who called this function
//  *) size: The number of all workers
//  *) (out) vertex_out_data: output data
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void apply_phase(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, vector<T1> * update_id,
                  const int rank, const int size, map<T1, T2> * vertex_out_data, engine_analyzer & my_analyzer){
     SSSP_apply_active(rank, size, slave_to_master, master_to_slave, 
                       update_id, vertex_out_data, my_analyzer);
}

/*
 * scatter function
 * check the aggregated value, and if more work is needed, some vertices is added to next iteration's job queue.
 */
// input:
//  *) slave_to_master: A map pointer. The key of this map is replicated vertex ID, and the value of this map is worker ID that contain the master vertex of replicated vertex.
//  *) master_to_slave: A map pointer. The key of this map is master vertex ID, and the value of this map is a vector that contain the worker IDs containing replicated vertices.
//  *) update_id: vertices to be updated
//  *) rank: ID of the worker who called this function
//  *) size: The number of all workers
//  *) f: local scatter function
//  *) next_step_id: The IDs of vertices that will be computed in the next step.
//  *) my_analyzer: A object profiling this function.
template<typename T1, typename T2>
void scatter_phase(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, vector<T1> * update_id,
                  const int rank, const int size, boost::function<vector<T1>*()> f, queue<T1> * next_step_id, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    vector<T1> * next_step_vertices = f();
    for(int i = 0; i<next_step_vertices->size(); i++){
        next_step_id->push_back((*next_step_vertices)[i]);
    }
    my_analyzer.time_end(0, temp_time);
}

// check the number of activated vertices.
// my_count is track each worker's the number of done vertices.
// input:
//  *) active_vertice_set: vertices set that need to be updated.
//  *) my_rank: ID of the worker who called this function
//  *) cycle_num: the number of step.
//  *) my_count: the number of computation that is completed
//  *) my_analyzer: A object profiling this function.
template<typename T1>
int global_check_activate(set<T1> * active_vertice_set, const int my_rank, int cycle_num, int my_count, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    int active_len = active_vertice_set->size() - my_count;
    int active_sum = 0;
    my_analyzer.time_start(temp_time);
    MPI_Allreduce(&active_len, &active_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    my_analyzer.all_update(3, 2, sizeof(int), temp_time);
    if(my_rank==0){
        if(cycle_num%10000==0){
            //cout << "The all active vertices sum is: "<< active_sum << endl;
        }
    }
    return active_sum;
}

// check the number of activated vertices.
// input:
//  *) active_queue: A queue that contain vertices that need to be updated.
//  *) my_rank: ID of the worker who called this function
//  *) cycle_num: the number of step.
//  *) my_count: the number of computation that is completed
//  *) my_analyzer: A object profiling this function.
template<typename T1>
int global_check_activate(queue<T1> * active_queue, const int my_rank, int cycle_num, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    int active_len = active_queue->size();
    int active_sum = 0;
    my_analyzer.time_start(temp_time);
    MPI_Allreduce(&active_len, &active_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    my_analyzer.all_update(3, 2, sizeof(int), temp_time);
    if(my_rank==0){
        if(cycle_num%10000==0){
            //cout << "The all active vertices sum is: "<< active_sum << endl;
        }
    }
    return active_sum;
}

// exchange vectors globally.
// each worker has each one's send_data & recv data.
// send_data's key is other worker's ID and value is a vector that is need to be trasmitted to the key.
// input:
//  *) my_rank: ID of the worker who called this function
//  *) nproc: The number of all workers
//  *) send_data: A pointer of the vector to send.
//  *) recv_data: A pointer of the vector to recv.
//  *) my_analyzer: A object profiling this function.
template<typename T1>
void exchange_vec_data(const int my_rank, const int nproc, map<int, vector<T1>> * send_data, vector<T1> * recv_data, engine_analyzer & my_analyzer){
    //init
    int * send_or_not = new int[nproc];
    int * all_reduce  = new int[nproc];
    int my_all_send = 0;
    int my_now_send = 0;
    int my_all_recv = 0;
    int my_now_recv = 0;
    for(int i = 0; i<nproc; i++){
        send_or_not[i] = 0;
        all_reduce[i]  = 0;
        if(i!=my_rank){
            if((*send_data)[i].size()>0){
                send_or_not[i] = 1;
                my_all_send++;
            }
        }
    }
    MPI_Allreduce(send_or_not, all_reduce, nproc, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    my_all_recv = all_reduce[my_rank];
    for(int i = 0; i<(*send_data)[my_rank].size(); i++){
        recv_data->push_back((*send_data)[my_rank][i]);
    }
    
    int send_index = 0;

    MPI_Status temp_status;
    int flag;
    vector<int> send_flags(NUM_OF_All_MESSAGES_8, 1);
    vector<std::string> send_datas(NUM_OF_All_MESSAGES_8);
    vector<MPI_Request> requests(NUM_OF_All_MESSAGES_8);
    queue<int> send_indices;
    list<int> off_indices;
    int send_slot;
    int send_slot_out;
    int number_amount;
    std::chrono::high_resolution_clock::time_point temp_time;
    
    for(int i = 0; i<NUM_OF_All_MESSAGES_8; i++){
        send_indices.push(i);
    }
    while(true){
        MPI_Iprobe(MPI_ANY_SOURCE, DATA_EXCHANGE, MPI_COMM_WORLD, &flag, &temp_status);
        if(flag){
            if(temp_status.MPI_TAG==DATA_EXCHANGE)
            {   
                my_analyzer.time_start(temp_time);
                MPI_Get_count(&temp_status, MPI_CHAR, &number_amount);
                char * recv_string = new char[number_amount];
                MPI_Recv(recv_string, number_amount, MPI_CHAR, temp_status.MPI_SOURCE, DATA_EXCHANGE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_analyzer.add_data_amount(1, sizeof(char)*number_amount);
                {
                    vector<T1> recv_vec;
                    boost::iostreams::basic_array_source<char> device(recv_string, number_amount);
                    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
                    boost::archive::binary_iarchive ia(s);
                    ia >> recv_vec;
                    for(int i = 0; i<recv_vec.size(); i++){
                        recv_data->push_back(recv_vec[i]);
                    }
                }
                my_now_recv++;
                delete [] recv_string;
                my_analyzer.time_end(2, temp_time);
            }
            else{
                cout << "Unknown Tag in aggregate_SSSP tag" << endl;
                cout<< "the tag: "<<temp_status.MPI_TAG<<endl;
            }
        }
        else{
            if(!off_indices.empty()){
                auto iter = off_indices.begin();
                while (iter != off_indices.end()){
                    MPI_Test(&(requests[*iter]), &(send_flags[*iter]), MPI_STATUS_IGNORE);
                    if(send_flags[*iter]){
                        my_now_send++;
                        send_slot_out = *iter;
                        off_indices.erase(iter++);
                        send_indices.push(send_slot_out);
                    }
                    else{
                        ++iter;
                    }
                }
            }
            if(my_all_send == my_now_send && my_all_recv==my_now_recv){
                break;
            }
            if(send_index>=nproc){
                continue;
            }
            if(!send_indices.empty()){
                if(send_or_not[send_index]==1){ // send
                    my_analyzer.time_start(temp_time);
                    send_slot = send_indices.front();
                    send_indices.pop();
                    off_indices.push_back(send_slot);
                    send_datas[send_slot].clear();
                    send_datas[send_slot]="";
                    boost::iostreams::back_insert_device<std::string> inserter(send_datas[send_slot]);
                    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
                    boost::archive::binary_oarchive oa(s);
                    oa << (*send_data)[send_index];
                    s.flush();
                    MPI_Isend(send_datas[send_slot].c_str(), send_datas[send_slot].size(), MPI_CHAR, send_index, 
                            DATA_EXCHANGE, MPI_COMM_WORLD, &(requests[send_slot]));
                    my_analyzer.add_data_amount(0, sizeof(char)*(send_datas[send_slot].size()));
                    
                    send_index++;
                    my_analyzer.time_end(1, temp_time);
                }
                else{
                    send_index++;
                }
            }         
        }
    }
    delete [] send_or_not;
    delete [] all_reduce;
}

#endif