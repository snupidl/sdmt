#ifndef GLOBAL_GRAPH_H
#define GLOBAL_GRAPH_H

/************************************
 * Information                      *
 ************************************/
// This code offers the global functions for the graph construction, initialization, and some global functions. 

#include "global_file_IO.h"
#include "graph_class.h"
#include "partitioning_strategies.h"
#include "communication.h"
#include <mpi.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;

// This function globally construct a global graph.
// Each worker has each ones local graph.
// From the graph data file (file_path), this function initialize a global graph g.
template<typename T1>
void global_read_textfile(string file_path, const int rank, const int size,
               graph<T1> * g){
    MPI_File in;
    int ierr;
    std::ifstream readFile;
    readFile.open(file_path);
    if(!readFile.is_open()) {
        cout<< "Rank "<<to_string(rank)<<" ierr on"<<endl;
        if (rank == 0) fprintf(stderr, "Couldn't open file %s\n", file_path);
        MPI_Finalize();
        exit(2);
    }
    vector<string> lines;
    int nlines;
    readlines(readFile, rank, size, lines, &nlines);
    readFile.close();
    //cout<< "Rank "<<to_string(rank)<<" has "<<to_string(nlines)<< " lines."<<endl;
    g->convert_string_to_graph(lines, nlines, rank);
}

// This function globally construct a global graph.
// Each worker has each ones local graph.
// From the graph data file (file_path), this function initialize a global graph g.
// The difference with global_read_textfile function is that this function adopts
// partitioning strategy (partitioning_strategy_id) and partitions the graph data to workers.
// The number of Vertex and Edge are also arguments of this function for the fast partitioning.  
template<typename T1>
void global_read_textfile_with_partitioning_strategy(string file_path, 
    const int rank, const int size, bool direction, int partitioning_strategy_id, int & allV, int & allE, graph<T1> * g){
    // 1. read
    // 2. partition
    // 3. transfer
    // 4. make final-state local graphs

    // 1
    graph<T1> first(direction);
    if(rank==0){
        //cout<< "Read start"<<endl;
    }
    global_read_textfile(file_path, rank, size, &first);
    if(rank==0){
        //cout<< "Read OK"<<endl;
    }

    // 2
    if(rank==0){
        //cout<< "Partition start"<<endl;
    }
    graph<T1> * graphs = new graph<T1>[size];
    if(partitioning_strategy_id==0){
        local_partition_strategy_1D(first, rank, size, graphs);
    }
    else if(partitioning_strategy_id==1){
        local_partition_strategy_1D_DST(first, rank, size, graphs);
    }
    else if(partitioning_strategy_id==2){
        local_partition_Random(first, rank, size, graphs);
    }
    else if(partitioning_strategy_id==3){
        local_partition_Canonical_Random(first, rank, size, graphs);
    }
    else if(partitioning_strategy_id==4){
        local_partition_2D(first, rank, size, graphs);
    }
    else if(partitioning_strategy_id==5){ // hybrid
        graph<T1> second(direction);
        local_partition_strategy_1D_DST(first, rank, size, graphs);
        for(auto iter=graphs[rank].get_e_vec_p()->begin(); iter!=graphs[rank].get_e_vec_p()->end(); iter++){
            second.add_edge_with_vertex(-1, *iter);
        }
        int * each_evec_size = new int[size];
        int my_recv_size;
        for(int i = 0; i<size; i++){
            if(rank==i){ // source
                for(int j = 0; j<size; j++){
                    each_evec_size[j] = graphs[j].local_edge_num();
                }
                MPI_Scatter(each_evec_size, 1, MPI_INT, 
                            &my_recv_size, 1, MPI_INT, 
                            i, MPI_COMM_WORLD);
                for(int j = 0; j<size; j++){
                    if(each_evec_size[j]!=0 && rank!=j){
                        send_vector(graphs[j].get_e_vec_p(), j);
                    }
                }
            }
            else{
                MPI_Scatter(each_evec_size, 1, MPI_INT, 
                            &my_recv_size, 1, MPI_INT, 
                            i, MPI_COMM_WORLD);
                if(my_recv_size!=0){
                    vector<edge<T1>> recv_vec;
                    recv_vector(&recv_vec, i, rank, my_recv_size);
                    for(auto iter=recv_vec.begin(); iter!=recv_vec.end(); iter++){
                        second.add_edge_with_vertex(-1, *iter);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

        delete [] graphs;
        delete [] each_evec_size;
        graphs = new graph<T1>[size];
        local_hybrid_second_phase(second, rank, size, graphs, 100);
    }
    else if(partitioning_strategy_id==6){
        local_powerGraph_Oblivious(first, rank, size, graphs);
    }
    else if(partitioning_strategy_id==7){
        local_powerGraph_HDRF(first, rank, size, graphs, 10.0);
    }
    else if(partitioning_strategy_id==8){
        local_powerGraph_HDRF(first, rank, size, graphs, 20.0);
    }
    else if(partitioning_strategy_id==9){
        local_powerGraph_HDRF(first, rank, size, graphs, 50.0);
    }
    else if(partitioning_strategy_id==10){
        local_powerGraph_HDRF(first, rank, size, graphs, 100.0);
    }
    else if(partitioning_strategy_id==11){
        graph<T1> second(direction);
        local_partition_strategy_1D_DST(first, rank, size, graphs);
        for(auto iter=graphs[rank].get_e_vec_p()->begin(); iter!=graphs[rank].get_e_vec_p()->end(); iter++){
            second.add_edge_with_vertex(-1, *iter);
        }
        int * each_evec_size = new int[size];
        int my_recv_size;
        for(int i = 0; i<size; i++){
            if(rank==i){ // source
                for(int j = 0; j<size; j++){
                    each_evec_size[j] = graphs[j].local_edge_num();
                }
                MPI_Scatter(each_evec_size, 1, MPI_INT, 
                            &my_recv_size, 1, MPI_INT, 
                            i, MPI_COMM_WORLD);
                for(int j = 0; j<size; j++){
                    if(each_evec_size[j]!=0 && rank!=j){
                        send_vector(graphs[j].get_e_vec_p(), j);
                    }
                }
            }
            else{
                MPI_Scatter(each_evec_size, 1, MPI_INT, 
                            &my_recv_size, 1, MPI_INT, 
                            i, MPI_COMM_WORLD);
                if(my_recv_size!=0){
                    vector<edge<T1>> recv_vec;
                    recv_vector(&recv_vec, i, rank, my_recv_size);
                    for(auto iter=recv_vec.begin(); iter!=recv_vec.end(); iter++){
                        second.add_edge_with_vertex(-1, *iter);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        delete [] graphs;
        delete [] each_evec_size;
        graphs = new graph<T1>[size];
        local_ginger_second_phase(second, rank, size, graphs, 100,
                                  allV, allE);
    }
    else{
        abort();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
        //cout<< "Partition End"<<endl;
    }

    // 3
    int * each_evec_size = new int[size];
    int my_recv_size;
    int my_edge_num = 0;
    for(auto iter=graphs[rank].get_e_vec_p()->begin(); iter!=graphs[rank].get_e_vec_p()->end(); iter++){
        g->add_edge_with_vertex(my_edge_num*size+rank, *iter);
        my_edge_num++;
    }
    for(int i = 0; i<size; i++){
        if(rank==i){ // source
            for(int j = 0; j<size; j++){
                each_evec_size[j] = graphs[j].local_edge_num();
            }
            MPI_Scatter(each_evec_size, 1, MPI_INT, 
                        &my_recv_size, 1, MPI_INT, 
                        i, MPI_COMM_WORLD);
            for(int j = 0; j<size; j++){
                if(each_evec_size[j]!=0 && rank!=j){
                    send_vector(graphs[j].get_e_vec_p(), j);
                }
            }
        }
        else{
            MPI_Scatter(each_evec_size, 1, MPI_INT, 
                        &my_recv_size, 1, MPI_INT, 
                        i, MPI_COMM_WORLD);
            if(my_recv_size!=0){
                vector<edge<T1>> recv_vec;
                recv_vector(&recv_vec, i, rank, my_recv_size);
                for(auto iter=recv_vec.begin(); iter!=recv_vec.end(); iter++){
                    g->add_edge_with_vertex(my_edge_num*size+rank, *iter);
                    my_edge_num++;
                }
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // 4
    if(direction==false){
        g->make_graph_from_e_vec_for_only_undirect(rank, size);
        MPI_Barrier(MPI_COMM_WORLD);
        g->sort_e_vec();
        MPI_Barrier(MPI_COMM_WORLD);
        g->make_e_index();
        MPI_Barrier(MPI_COMM_WORLD);
    }
    else{
        g->make_inverted_e_vec();
        g->sort_e_vec();
        g->sort_inverted_e_vec();
        g->make_e_index();
        g->make_inverted_e_index();
    }
    //g->print_graph_info(rank);
    delete [] each_evec_size;
    delete [] graphs;
}

// return the global max value of the vertex ID
template<typename T1>
T1 get_global_maxid_vertex(graph<T1> * graph_p, const int rank, const int size){
    T1 each_local_max = graph_p->get_maxid_vertex();
    T1 * recv_id = new T1 [size];
    T1 global_max;
    MPI_Gather(&each_local_max, 1*sizeof(T1), MPI_BYTE, 
               recv_id, 1*sizeof(T1), MPI_BYTE, 0, MPI_COMM_WORLD);
    if(rank==0){
        global_max = recv_id[0];
        for(int i = 1; i<size; i++){
            if(recv_id[i]>global_max){
                global_max = recv_id[i];
            }
        }
    }
    MPI_Bcast(&global_max, sizeof(T1), MPI_BYTE, 0, MPI_COMM_WORLD);
    delete [] recv_id;
    return global_max;
}

// There is no replicated edge, but there are replicated vertex in our engine, because we adopts the vertex-cut model.
// The replicated vertices must be rightly handled for the GAS computation model.
// This function selects a master vertex in replicated vertices that has the same vertex ID.
// The vertex that is not a master vertex is a slave, replicated vertex.
// slave_to_master and master_to_slave are the output of this function.
// slave_to_master contains each worker's replicated/slave vertex IDs as key and worker IDs that contain this vertex as a master vertex. 
// master_to_slave  contains each worker's master vertex IDs as key and every worker IDs as value that contain this vertex as a slave vertex.
template<typename T1>
void initialization_MS_communication_master_slave_list(graph<T1> * graph_p, int my_rank, int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave){
    int my_slave_num = 0;
    int * cluster_slave_receive_num = new int [nproc];
    bool * cluster_have_that_vertex = new bool [nproc];
    
    T1 global_max = get_global_maxid_vertex(graph_p, my_rank, nproc);
    T1 temp_num = 0;
    bool temp_is_there;

    while(temp_num<= global_max){
        if(temp_num % 10000 == 0){
            if(my_rank==0){
                cout<<"initialization_MS: [ "<<temp_num<<" / "<<global_max<<" ]"<<endl;
            }
        }
        temp_is_there = graph_p->find_vertex(temp_num);
        MPI_Allgather(&temp_is_there, sizeof(bool), MPI_BYTE, cluster_have_that_vertex, sizeof(bool), MPI_BYTE, MPI_COMM_WORLD);
        vector<int> temp_vertex_worker;
        for(int j = 0; j<nproc; j++){
            if(cluster_have_that_vertex[j]==true){
                temp_vertex_worker.push_back(j);
            }
        }
        if(temp_vertex_worker.size()==0){
            temp_num++;
        }
        else if(temp_vertex_worker.size()==1){
            if(my_rank==temp_vertex_worker.at(0)){
                (*master_to_slave)[temp_num] = vector<int>();
            }
            temp_num++;
        }
        else{
            int min_worker_idx;
            int min_receive = std::numeric_limits<int>::max();
            MPI_Allgather(&my_slave_num, 1, MPI_INT, cluster_slave_receive_num, 1, MPI_INT, MPI_COMM_WORLD);
            for(int j = 0; j<temp_vertex_worker.size(); j++){
                if(min_receive>cluster_slave_receive_num[temp_vertex_worker[j]]){
                    min_receive=cluster_slave_receive_num[temp_vertex_worker[j]];
                    min_worker_idx = temp_vertex_worker[j];
                }
            }
            if(my_rank==min_worker_idx){
                my_slave_num += (temp_vertex_worker.size()-1);
                (*master_to_slave)[temp_num] = vector<int>();
                for(int j = 0; j<temp_vertex_worker.size(); j++){
                    if(my_rank!=temp_vertex_worker[j]){
                        (*master_to_slave)[temp_num].push_back(temp_vertex_worker[j]);
                    }
                }
            }
            else{
                if(temp_is_there==true){
                    (*slave_to_master)[temp_num] = min_worker_idx;
                }
            }
            temp_num++;
        }
    }

    delete [] cluster_slave_receive_num;
    delete [] cluster_have_that_vertex;
    return;
}

// There is no replicated edge, but there are replicated vertex in our engine, because we adopts the vertex-cut model.
// The replicated vertices must be rightly handled for the GAS computation model.
// This function selects a master vertex in replicated vertices that has the same vertex ID.
// The vertex that is not a master vertex is a slave, replicated vertex.
// slave_to_master and master_to_slave are the output of this function.
// slave_to_master contains each worker's replicated/slave vertex IDs as key and worker IDs that contain this vertex as a master vertex. 
// master_to_slave  contains each worker's master vertex IDs as key and every worker IDs as value that contain this vertex as a slave vertex.
// More fast version.
template<typename T1>
void initialization_MS_communication_master_slave_list_master(graph<T1> * graph_p, int my_rank, int nproc, map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave){
    int my_master_num = 0;
    int * cluster_slave_receive_num = new int [nproc];
    bool * cluster_have_that_vertex = new bool [nproc];
    
    T1 global_max = get_global_maxid_vertex(graph_p, my_rank, nproc);
    T1 temp_num = 0;
    bool temp_is_there;

    while(temp_num<= global_max){
        if(temp_num % 10000 == 0){
            if(my_rank==0){
                //cout<<"initialization_MS: [ "<<temp_num<<" / "<<global_max<<" ]"<<endl;
            }
        }
        temp_is_there = graph_p->find_vertex(temp_num);
        MPI_Allgather(&temp_is_there, sizeof(bool), MPI_BYTE, cluster_have_that_vertex, sizeof(bool), MPI_BYTE, MPI_COMM_WORLD);
        vector<int> temp_vertex_worker;
        for(int j = 0; j<nproc; j++){
            if(cluster_have_that_vertex[j]==true){
                temp_vertex_worker.push_back(j);
            }
        }
        if(temp_vertex_worker.size()==0){
            temp_num++;
        }
        else if(temp_vertex_worker.size()==1){
            if(my_rank==temp_vertex_worker.at(0)){
                (*master_to_slave)[temp_num] = vector<int>();
                my_master_num++;
            }
            temp_num++;
        }
        else{
            int min_worker_idx;
            int min_receive = std::numeric_limits<int>::max();
            MPI_Allgather(&my_master_num, 1, MPI_INT, cluster_slave_receive_num, 1, MPI_INT, MPI_COMM_WORLD);
            for(int j = 0; j<temp_vertex_worker.size(); j++){
                if(min_receive>cluster_slave_receive_num[temp_vertex_worker[j]]){
                    min_receive=cluster_slave_receive_num[temp_vertex_worker[j]];
                    min_worker_idx = temp_vertex_worker[j];
                }
            }
            if(my_rank==min_worker_idx){
                my_master_num ++;
                (*master_to_slave)[temp_num] = vector<int>();
                for(int j = 0; j<temp_vertex_worker.size(); j++){
                    if(my_rank!=temp_vertex_worker[j]){
                        (*master_to_slave)[temp_num].push_back(temp_vertex_worker[j]);
                    }
                }
            }
            else{
                if(temp_is_there==true){
                    (*slave_to_master)[temp_num] = min_worker_idx;
                }
            }
            temp_num++;
        }
    }

    delete [] cluster_slave_receive_num;
    delete [] cluster_have_that_vertex;
    return;
}

#endif
