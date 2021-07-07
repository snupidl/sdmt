#ifndef GRAPH_DATA_STAT_H
#define GRAPH_DATA_STAT_H

/************************************
 * Information                      *
 ************************************/
// This file contain some code that extract some graph stats.

#include "global_file_IO.h"
#include "graph_class.h"
#include "partitioning_strategies.h"
#include "analyzer.h"
#include "tools.h"
#include <cstring>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
//#include <experimental/algorithm>
#include <limits>
#include <random>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/serialization/utility.hpp>

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

// get the num of edge
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return the number of all edges
template<typename T1>
int get_number_of_global_edge(graph<T1> * g, const int my_rank, int print_or_not, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    int local_edges = g->get_edge_size();
    my_analyzer.time_end(0, temp_time);
    int global_edges = 0;
    my_analyzer.time_start(temp_time);
    MPI_Allreduce(&local_edges, &global_edges, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    my_analyzer.all_update(3, 2, sizeof(int), temp_time);
    if(my_rank==0 && print_or_not==1){
        cout<< "The number of global edges: "<<global_edges<<endl;
    }
    return global_edges;
}

// the num of vertices
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return the number of all master vertices 
template<typename T1>
int get_number_of_global_vertex(graph<T1> * g, const int my_rank, map<T1, vector<int>> * master_to_slave, int print_or_not, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    int local_master_verices = master_to_slave->size();
    my_analyzer.time_end(0, temp_time);
    int global_master_verices = 0;
    my_analyzer.time_start(temp_time);
    MPI_Allreduce(&local_master_verices, &global_master_verices, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    my_analyzer.all_update(3, 2, sizeof(int), temp_time);
    if(my_rank==0 && print_or_not==1){
        cout<< "The number of global vertices: "<<global_master_verices<<endl;
    }
    return global_master_verices;
}


// get IN degree
// input:
//      *) g -> local graph
//      *) input_set -> first on vertices (all local vertices)
// output: 
//      *) return a map (key: vertex ID, value: in-degree of a vertex)
template<typename T1>
map<T1, int> * local_in_degree(graph<T1> * g, set<T1> * input_set){
    map<T1, int> * output = new map<T1, int>();
    int temp_degree;
    for(auto iter=input_set->begin(); iter!=input_set->end(); iter++){
        temp_degree = g->get_vertex_degree(IN_NEI, *iter);
        (*output)[*iter] = temp_degree;
    }
    return output;
}

// get global IN-degree
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return X
//      *) print vertex, In-degree
template<typename T1>
int get_all_vertices_in_degree(graph<T1> * g, const int my_rank, const int size, 
                               map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, int print_or_not, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    set<T1> input_set;
    map<T1, int> vertex_to_indegree;
    for(auto iter=master_to_slave->begin(); iter!= master_to_slave->end(); iter++) {
        input_set.insert(iter->first);
        vertex_to_indegree[iter->first] = 0;
    }
    for(auto iter=slave_to_master->begin(); iter!= slave_to_master->end(); iter++) {
        input_set.insert(iter->first);
        vertex_to_indegree[iter->first] = 0;
    }
    my_analyzer.time_end(0, temp_time);
    gather_phase(slave_to_master, master_to_slave, &input_set,
                 my_rank, size, SUM, boost::bind<void*>(local_in_degree<T1>, g, &input_set), &vertex_to_indegree, my_analyzer);
    if(print_or_not==1){
        int count = 0;
        for(auto iter=vertex_to_indegree.begin(); iter!= vertex_to_indegree.end(); iter++) {
            if(count==3){
                break;
            }
            count++;
            cout<<"MY_RANK: "<< my_rank <<", ID: "<<iter->first<<" , in-degree: "<< iter->second<<endl;    
        }
    }
    return 0;
}

// get global IN-degree
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return X
//      *) print vertex, In-degree
//      *) in_degree -> (key: vertex ID, value: global in-degree)
template<typename T1>
int get_all_vertices_in_degree_after(graph<T1> * g, const int my_rank, const int size, 
                               map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, int> * in_degree, int print_or_not, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    set<T1> input_set;
    map<T1, int> vertex_to_indegree;
    for(auto iter=master_to_slave->begin(); iter!= master_to_slave->end(); iter++) {
        input_set.insert(iter->first);
        vertex_to_indegree[iter->first] = 0;
    }
    for(auto iter=slave_to_master->begin(); iter!= slave_to_master->end(); iter++) {
        input_set.insert(iter->first);
        vertex_to_indegree[iter->first] = 0;
    }
    my_analyzer.time_end(0, temp_time);
    gather_phase(slave_to_master, master_to_slave, &input_set,
                 my_rank, size, SUM, boost::bind<void*>(local_in_degree<T1>, g, &input_set), &vertex_to_indegree, my_analyzer);
    for(auto iter=vertex_to_indegree.begin(); iter!=vertex_to_indegree.end(); iter++){
        (*in_degree)[iter->first] = iter->second;
    }
    if(print_or_not==1){
        int count = 0;
        for(auto iter=vertex_to_indegree.begin(); iter!= vertex_to_indegree.end(); iter++) {
            if(count==3){
                break;
            }
            count++;
            cout<<"MY_RANK: "<< my_rank <<", ID: "<<iter->first<<" , in-degree: "<< iter->second<<endl;    
        }
    }
    return 0;
}

// get OUT degree
// input:
//      *) g -> local graph
//      *) input_set -> first on vertices (all local vertices)
// output: 
//      *) return a map (key: vertex ID, value: out-degree of a vertex)
template<typename T1>
map<T1, int> * local_out_degree(graph<T1> * g, set<T1> * input_set){
    map<T1, int> * output = new map<T1, int>();
    int temp_degree;
    for(auto iter=input_set->begin(); iter!=input_set->end(); iter++){
        temp_degree = g->get_vertex_degree(OUT_NEI, *iter);
        (*output)[*iter] = temp_degree;
    }
    return output;
}

// get global out-degree
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return X
//      *) print vertex, out-degree
//      *) vertex_to_outdegree -> (key: vertex ID, value: global out-degree)
template<typename T1>
void get_all_vertices_out_degree(graph<T1> * g, const int my_rank, const int size, 
                               map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, int> & vertex_to_outdegree, int print_or_not,
                               engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    set<T1> input_set;
    vector<T1> input_vec;
    for(auto iter=master_to_slave->begin(); iter!= master_to_slave->end(); iter++) {
        input_set.insert(iter->first);
        input_vec.push_back(iter->first);
        vertex_to_outdegree[iter->first] = 0;
    }
    for(auto iter=slave_to_master->begin(); iter!= slave_to_master->end(); iter++) {
        input_set.insert(iter->first);
        input_vec.push_back(iter->first);
        vertex_to_outdegree[iter->first] = 0;
    }
    my_analyzer.time_end(0, temp_time);
    gather_phase(slave_to_master, master_to_slave, &input_set,
                 my_rank, size, SUM, boost::bind<void*>(local_out_degree<T1>, g, &input_set), &vertex_to_outdegree, my_analyzer);
    apply_phase(slave_to_master, master_to_slave, &input_vec,
                  my_rank, size, &vertex_to_outdegree, my_analyzer);
    if(print_or_not==1){
        int count = 0;
        for(auto iter=vertex_to_outdegree.begin(); iter!= vertex_to_outdegree.end(); iter++) {
            if(count==3){
                break;
            }
            count++;
            cout<<"MY_RANK: "<< my_rank <<", ID: "<<iter->first<<" , out-degree: "<< iter->second<<endl;    
        }
    }
    return;
}

// local PR, 1 iteration
// input:
//      *) g -> local graph
//      *) previous_PR -> before iteration's PageRank
//      *) out_degrees -> out-degree information, this info is used to compute the PageRank score.
// output: 
//      *) return a map (key: vertex ID, value: PageRank score)
template<typename T1>
map<T1, float> * local_get_PR(graph<T1> * g, map<T1, float> * previous_PR, map<T1, int> * out_degrees){
    // PR(n1)/Degree(n1) + ...
    map<T1, float> * output = new map<T1, float>();
    float temp_part_pr;
    set<vertex<T1>> * v_set = g->get_v_set();
    for(auto iter=v_set->begin(); iter!=v_set->end(); iter++){
        temp_part_pr = 0.0;
        vector<T1> neighbors;
        g->get_vertex_neighbor(IN_NEI, iter->get_id(), &neighbors);
        for(int i = 0; i<neighbors.size(); i++){
            temp_part_pr += ((*previous_PR)[neighbors[i]]/((float)((*out_degrees)[neighbors[i]])));
        }
        (*output)[iter->get_id()] = temp_part_pr;
    }
    return output;
}

// global PR
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) All_vertices_num -> the number of vertex, this is used to compute the PageRank score.
//      *) out_degrees -> out-degree information, this info is used to compute the PageRank score.
//      *) iter_num -> the number of iteration
//      *) dp -> damping ratio (PageRank)
//      *) input_set -> on vertices
//      *) input_vec -> on vertices (vector)
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) out_PR -> a map (key: vertex ID, value: PageRank score after last iteration)
template<typename T1>
void get_all_PageRank(graph<T1> * g, const int my_rank, const int size, 
                    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave,
                    const int All_vertices_num, map<T1, int> * out_degrees, const int iter_num, const float dp, set<T1> * input_set, vector<T1> * input_vec,
                    map<T1, float> * out_PR, int print_or_not, engine_analyzer & my_analyzer){
    // init
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    map<T1, float> previous_PR;
    set<vertex<T1>> * v_set = g->get_v_set();
    float start_value = 1.0/((float)All_vertices_num);
    float intermediate_value = (1.0-dp)/((float)(All_vertices_num));
    for(auto iter=v_set->begin(); iter!=v_set->end(); iter++){
        previous_PR[iter->get_id()] = start_value;
    }
    my_analyzer.time_end(0, temp_time);
    // PR iter
    for(int i = 0; i<iter_num; i++){
        gather_phase(slave_to_master, master_to_slave, input_set,
                 my_rank, size, SUM, boost::bind<void*>(local_get_PR<T1>, g, &previous_PR, out_degrees), &previous_PR, my_analyzer);
        my_analyzer.time_start(temp_time);
        for(auto iter=master_to_slave->begin(); iter!=master_to_slave->end(); iter++){
            previous_PR[iter->first] = intermediate_value + (dp*((previous_PR)[iter->first]));
        }
        my_analyzer.time_end(0, temp_time);
        // apply
        apply_phase(slave_to_master, master_to_slave, input_vec,
                  my_rank, size, &previous_PR, my_analyzer);
    }
    return;
}

// GC
// local graph coloring, 1 step
// input:
//      *) g -> local graph
//      *) colors -> each step's vertex color (key: vertex ID, value: vertex color)
//      *) on_vertex -> on vertex vector
// output: 
//      *) return a map (key: vertex ID, value: next step's vertex color)
template<typename T1>
map<T1, vector<int>> * local_get_neighbor_colors(graph<T1> * g, map<T1, int> * colors, vector<T1> * on_vertex){
    map<T1, vector<int>> * output = new map<T1, vector<int>>();
    for(int i = 0; i<on_vertex->size(); i++){
        if((*on_vertex)[i]==-1){
            continue;
        }
        vector<T1> neighbors;
        g->get_vertex_neighbor(BOTH_NEI, (*on_vertex)[i], &neighbors);
        for(int j = 0; j<neighbors.size(); j++){
            (*output)[(*on_vertex)[i]].push_back((*colors)[neighbors[j]]);
        }
    }
    return output;
}

// GC
// local graph coloring, 1 step
// input:
//      *) g -> local graph
//      *) colors -> each step's vertex color (key: vertex ID, value: vertex color)
//      *) on_vertex -> on vertex set
// output: 
//      *) return a map (key: vertex ID, value: next step's vertex color)
template<typename T1>
map<T1, vector<int>> * local_get_neighbor_colors_set(graph<T1> * g, map<T1, int> * colors, set<T1> * on_vertex){
    map<T1, vector<int>> * output = new map<T1, vector<int>>();


    for(auto iter = on_vertex->begin(); iter!=on_vertex->end(); iter++){
        vector<T1> neighbors;
        g->get_vertex_neighbor(BOTH_NEI, *iter, &neighbors);
        if(output->find(*iter)==output->end()){
            (*output)[*iter] = vector<int>();
        }
        for(int j = 0; j<neighbors.size(); j++){
            if(colors->find(neighbors[j])!=colors->end()){
                (*output)[*iter].push_back((*colors)[neighbors[j]]);
            }
        }
    }
    return output;
}

// GC
// find minimum color of the pivot vertex
// input:
//      *) g -> local graph
//      *) neigh_colors -> neighbor colors (key: vertex ID, value: vertex color)
//      *) pivot -> pivot vertex
// output: 
//      *) return int value (the smallest next color value)
template<typename T1>
int local_find_pivot_colors(graph<T1> * g, map<T1, vector<int>> * neigh_colors, const T1 & pivot){
    int start = 0;

    if(neigh_colors->find(pivot)!=neigh_colors->end() && (*neigh_colors)[pivot].size()>0){
        set<T1> s((*neigh_colors)[pivot].begin(), (*neigh_colors)[pivot].end()); 
        for(auto iter = s.begin(); iter!=s.end(); iter++){
            if(*iter==start){
                start++;
            }
            else{
                break;
            }
        }
    }
    return start;
}

// get neighbor vertices
// input:
//      *) g -> local graph
// output: 
//      *) return a map (key:vertex ID, value: neighbor vertices)
template<typename T1>
map<T1, vector<T1>> * local_get_neighbors(graph<T1> * g){
    map<T1, vector<T1>> * output = new map<T1, vector<T1>>();
    set<vertex<T1>> * temp_v_set_p = g->get_v_set();
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        (*output)[iter->get_id()] = vector<T1>();
    }
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        vector<T1> neighbors;
        T1 temp_v = iter->get_id();
        g->get_vertex_neighbor(BOTH_NEI, temp_v, &neighbors);
        for(int j = 0; j<neighbors.size(); j++){
            (*output)[temp_v].push_back(neighbors[j]);
        }
    }
    return output;
}

// global greedy graph coloring
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) out_color -> a map (key: vertex ID, value: vertex color)
template<typename T1>
void get_greedy_coloring(graph<T1> * g, const int my_rank, const int size, 
                    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave,
                    map<T1, int> * out_color, int print_or_not, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    // init

    set<T1> input_set;
    map<T1, vector<T1>> my_neighbors;
    set<vertex<T1>> * temp_v_set_p = g->get_v_set();
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        input_set.insert(iter->get_id());
    }
    my_analyzer.time_end(0, temp_time);
    
    gather_vector_phase(slave_to_master, master_to_slave, &input_set,
                        my_rank, size, MERGE, boost::bind<void*>(local_get_neighbors<T1>, g),
                        &my_neighbors, my_analyzer);

    set<T1> on_vertices;
    map<T1, int> temp_color;
    my_analyzer.time_start(temp_time);
    set<vertex<T1>> * v_set = g->get_v_set();
    int cycle_num = 0;
    int my_count = 0;
    for(auto iter=v_set->begin(); iter!=v_set->end(); iter++){
        temp_color[iter->get_id()] = 0;
    }
    for(auto iter=master_to_slave->begin(); iter!=master_to_slave->end(); iter++){
        on_vertices.insert(iter->first);
    }
    auto my_on_set_iter = on_vertices.begin();
    T1 my_on_vertex;

    vector<T1> on_vertex_gather;
    on_vertex_gather.resize(size);
    vector<int> last_colors;
    last_colors.resize(size);
    int * can_array = new int [size];
    my_analyzer.time_end(0, temp_time);

    while(global_check_activate(&on_vertices, my_rank, cycle_num, my_count, my_analyzer)>0){
        cycle_num++;
        if(my_on_set_iter != on_vertices.end()){
            my_on_vertex = *my_on_set_iter;
        }
        else{
            my_on_vertex = -1;
        }
        map<T1, vector<int>> gather_colors;
        my_analyzer.time_start(temp_time);
        MPI_Allgather(&my_on_vertex, sizeof(T1), MPI_BYTE, (void*)(on_vertex_gather.data()), sizeof(T1), MPI_BYTE, MPI_COMM_WORLD);
        my_analyzer.time_end(3, temp_time);
        my_analyzer.add_data_amount(0, sizeof(T1));
        my_analyzer.add_data_amount(1, size*sizeof(T1));
        set<T1> s(on_vertex_gather.begin(), on_vertex_gather.end());
        auto it = s.find (-1);
        if(it!=s.end()){
            s.erase (it); 
        }

        gather_vector_phase(slave_to_master, master_to_slave, &s,
                         my_rank, size, MERGE, boost::bind<void*>(local_get_neighbor_colors_set<T1>, g, &temp_color, &s),
                         &gather_colors, my_analyzer);
        
        my_analyzer.time_start(temp_time);
        int my_new_color = -1;
        if(my_on_vertex!=-1){
            my_new_color= local_find_pivot_colors(g, &gather_colors, my_on_vertex);
        }
        my_analyzer.time_end(0, temp_time);

        my_analyzer.time_start(temp_time);
        MPI_Allgather(&my_new_color, 1, MPI_INT, (void*)(last_colors.data()), 1, MPI_INT, MPI_COMM_WORLD);
        my_analyzer.time_end(3, temp_time);
        my_analyzer.add_data_amount(0, sizeof(int));
        my_analyzer.add_data_amount(1, size*sizeof(int));
        my_analyzer.time_start(temp_time);
        
        int can_do_next = 1;
        for(int i = 0; i<size; i++){
            if(i!=my_rank && on_vertex_gather[i]<my_on_vertex && my_on_vertex!=-1 && on_vertex_gather[i]!=-1){
                auto iter = find((my_neighbors)[my_on_vertex].begin(), (my_neighbors)[my_on_vertex].end(), on_vertex_gather[i]);
                if (iter != (my_neighbors)[my_on_vertex].end()){ 
                    if(my_new_color==last_colors[i]) can_do_next=0;
                }
            }
        }

        MPI_Allgather(&can_do_next, 1, MPI_INT, can_array, 1, MPI_INT, MPI_COMM_WORLD);

        
        for(int i = 0; i<size; i++){
            if(temp_color.find(on_vertex_gather[i])!=temp_color.end() && last_colors[i]!=-1 && can_array[i]==1){
                temp_color[on_vertex_gather[i]] = last_colors[i];
            }
        }
        
        
        if( my_on_set_iter != on_vertices.end() && can_do_next==1){
            my_on_set_iter++;
            my_count++;
        }
        my_analyzer.time_end(0, temp_time);
        my_analyzer.time_start(temp_time);
        MPI_Barrier(MPI_COMM_WORLD);
        my_analyzer.time_end(4, temp_time);
    }
    for(auto iter=master_to_slave->begin(); iter!=master_to_slave->end(); iter++){
        (*out_color)[iter->first] = temp_color[iter->first];
    }

    if(my_rank==0 && print_or_not==1){
        int count = 0;
        for(auto iter=out_color->begin(); iter!= out_color->end(); iter++) {
            if(count==3){
                break;
            }
            count++;
            cout<<"ID: "<<iter->first<<" , Color: "<< iter->second<<endl;    
        }
    }
    delete [] can_array;
    return;
}

/************* APCN *****************/
// APCN computation unit
template<typename T1>
class v_and_master{
private:
	friend class boost::serialization::access;
	template<class Archieve>
	void serialize(Archieve&ar, const unsigned int version)
	{
		ar& vertex;
		ar& master_worker;
	}  
public:
    T1 vertex;
    int master_worker;

    void set_all(T1 v_, int mw_){
        vertex = v_;
        master_worker = mw_;
    }
};

// get local out-neighbors
// input:
//      *) g -> local graph
//      *) myrank -> rank of each worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
// output: 
//      *) return a map (key: T1 - vertex ID, value: vector<v_and_master<T1>> - out neighbors and it's master worker ID)
template<typename T1>
map<T1, vector<v_and_master<T1>>> * local_get_outneighbors(graph<T1> * g, const int myrank, 
    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave){

    map<T1, vector<v_and_master<T1>>> * output = new map<T1, vector<v_and_master<T1>>>();
    set<vertex<T1>> * temp_v_set_p = g->get_v_set();
    
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        vector<T1> neighbors;
        T1 temp_v = iter->get_id();
        g->get_vertex_neighbor(OUT_NEI, temp_v, &neighbors);
        if(neighbors.size()==0){
            (*output)[temp_v] = vector<v_and_master<T1>>();
            continue;
        }
        for(int j = 0; j<neighbors.size(); j++){
            v_and_master<T1> temp;
            if(slave_to_master->find(neighbors[j])!=slave_to_master->end()){
                temp.set_all(neighbors[j], (*slave_to_master)[neighbors[j]]);
                (*output)[temp_v].push_back(temp);
            }
            else{ // i am master worker
                temp.set_all(neighbors[j], myrank);
                (*output)[temp_v].push_back(temp);
            }
        }
    }
    return output;
}

// global all pair common neighbors
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) v_to_neighbors -> a map (key: pair<T1, T1> - two vertices pair, 
//                                  value: set<T1> - common vertices)
template<typename T1>
void get_all_pair_common_neighbor(graph<T1> * g, const int my_rank, const int size, 
                    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave,
                    map<pair<T1, T1>, set<T1>> * v_to_neighbors, int print_or_not, engine_analyzer & my_analyzer){
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    // init
    set<T1> input_set;
    map<T1, vector<T1>> global_neighbors;
    set<vertex<T1>> * temp_v_set_p = g->get_v_set();
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        input_set.insert(iter->get_id());
    }
    my_analyzer.time_end(0, temp_time);
    
    gather_vector_phase(slave_to_master, master_to_slave, &input_set,
                        my_rank, size, MERGE, boost::bind<void*>(local_get_neighbors<T1>, g),
                        &global_neighbors, my_analyzer);
    my_analyzer.time_start(temp_time);
    int m_size = master_to_slave->size();
    int m_count = 0;
    for(auto iter=master_to_slave->begin(); iter!=master_to_slave->end(); iter++){
        if(m_count%10000==0){
            cout<<"In APCN, myrank: "<<my_rank<< ", ["<<m_count<<"/"<<m_size<<"]"<<endl;
        }
        T1 now_v = iter->first;
        int this_vec_size = global_neighbors[now_v].size();
        map<pair<T1, T1>, set<T1>> temp_v_to_neighbors;
        for(int i = 0; i<this_vec_size; i++){
            for(int j = 0; j<this_vec_size; j++){
                if(i!=j){
                    temp_v_to_neighbors[std::make_pair(global_neighbors[now_v][i], global_neighbors[now_v][j])].insert(now_v);
                }
            }
        }
        m_count++;
    }
    my_analyzer.time_end(0, temp_time);

    if(my_rank==0 && print_or_not==1){
        int count = 0;
        for(auto iter=v_to_neighbors->begin(); iter!= v_to_neighbors->end(); iter++) {
            if(count==3){
                break;
            }
            count ++;
            cout<<"SRC 1: "<<iter->first.first<< ", SRC 2: "<<iter->first.second<<", CNs: ";
            for(auto t_iter=iter->second.begin(); t_iter!=iter->second.end(); t_iter++){
                cout<< *t_iter<<" ";
            }
            cout<<endl;
        }
    }
    return;
}

/************* TRIANGLE *****************/
// the number of global triangles
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return float -> the number of global triangles
template<typename T1>
float get_triangles(graph<T1> * g, const int my_rank, const int size, 
                    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, int print_or_not, engine_analyzer & my_analyzer){
    // init
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    set<T1> input_set;
    map<T1, vector<T1>> global_neighbors;
    set<vertex<T1>> * temp_v_set_p = g->get_v_set();
    float my_triangle = 0.0;
    float global_triangle = 0.0;
    int edge_num;
    //cout<<"my rank: "<< my_rank <<", triangle, START"<<endl;
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        input_set.insert(iter->get_id());
    }
    my_analyzer.time_end(0, temp_time);
    //cout<<"my rank: "<< my_rank <<", triangle, gather START"<<endl;
    gather_vector_phase(slave_to_master, master_to_slave, &input_set,
                        my_rank, size, MERGE, boost::bind<void*>(local_get_neighbors<T1>, g),
                        &global_neighbors, my_analyzer);
    //cout<<"my rank: "<< my_rank <<", triangle, gather OK"<<endl;
    SSSP_apply_vector_active(my_rank, size, slave_to_master, master_to_slave, 
                       &input_set, &global_neighbors, my_analyzer);
    //cout<<"my rank: "<< my_rank <<", triangle, neighbor exchange OK"<<endl;
    my_analyzer.time_start(temp_time);
    map<T1, set<T1>> global_neighbors_set;
    for(auto iter=global_neighbors.begin(); iter!=global_neighbors.end(); iter++){
        for(auto iter2=iter->second.begin(); iter2!=iter->second.end(); iter2++){
            if(iter->first!=*iter2){
                global_neighbors_set[iter->first].insert(*iter2);
            }
        }
    }
    my_analyzer.time_end(0, temp_time);    
 
    my_analyzer.time_start(temp_time);
    vector<edge<T1>> * edge_vec = g->get_e_vec_p();
    int e_vec_size = edge_vec->size();
    int temp_counter = 0;
    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        edge_num = 0;
        T1 src = iter->get_src();
        T1 dst = iter->get_dst();
        if(src==dst){
            continue;
        }
        std::vector<T1> v3;
        std::set_intersection(global_neighbors_set[src].begin(),global_neighbors_set[src].end(),
                          global_neighbors_set[dst].begin(),global_neighbors_set[dst].end(),
                          back_inserter(v3));
        for(auto iter_nei=global_neighbors[src].begin(); iter_nei!=global_neighbors[src].end(); iter_nei++){
            if(*iter_nei==dst){
                edge_num++;
            }
        }
        if(edge_num==0 && print_or_not == 1){
            cout<<"my rank: "<< my_rank<<", ERROR DIVIDE 0, ESRC: "<<src<< ", EDST: "<<dst<<endl;
        }
        my_triangle += ((float)(v3.size())/(float)(edge_num));
        temp_counter++;
        if(temp_counter%10000==0 && print_or_not == 1){
            cout<<"my rank: "<< my_rank <<", triangle iter ["<<temp_counter<<"/"<<e_vec_size<<"]"<<endl;
        }
    }
    my_analyzer.time_end(0, temp_time);
    if(my_rank==0 && print_or_not == 1){
        cout<<"triangle, all intersection OK"<<endl;
    }
    my_analyzer.time_start(temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    my_analyzer.time_end(4, temp_time);
    my_analyzer.time_start(temp_time);
    MPI_Allreduce(&my_triangle, &global_triangle, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    my_analyzer.all_update(3, 2, sizeof(float), temp_time);

    if(print_or_not==1 && my_rank==0){
        cout<< "Global triangles: " << global_triangle/3.0<<endl;
    }
    
    return global_triangle/3.0;
}

/************* Global Clustering Coefficient *****************/
// get the global clustering coefficient
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) return float -> global clutering coefficient
template<typename T1>
float get_GCC(graph<T1> * g, const int my_rank, const int size, 
                    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, int print_or_not, engine_analyzer & my_analyzer){
    int num_all_v = get_number_of_global_vertex(g, my_rank, master_to_slave, 0, my_analyzer);
    float all_triplet = (float)(num_all_v/ (float)(3*2)) * (float)(num_all_v-1) * (float)(num_all_v-2);
    int all_triangle = get_triangles(g, my_rank, size, slave_to_master, master_to_slave, 0, my_analyzer);

    float gcc = ((float)all_triangle)/(all_triplet);
    if(print_or_not==1 && my_rank==0){
        cout<< "Global Clustering Coefficient, triangles: " << all_triangle<<endl;
        cout<< "Global Clustering Coefficient, triplet: " << all_triplet<<endl;
        cout<< "Global Clustering Coefficient, num_all_v: " << num_all_v<<endl;
        cout<< "Global Clustering Coefficient: " << gcc<<endl;
    }
    return gcc;
}

/**************** RW **************/
// Random Walk computation unit
template<typename T1>
class RW_tuple{
private:
	friend class boost::serialization::access;
	template<class Archieve>
	void serialize(Archieve&ar, const unsigned int version)
	{
		ar& vertex;
		ar& master_worker;
        ar& now_pivot;
		ar& walknum;
	}  
public:
    T1 vertex;
    int master_worker;
    T1 now_pivot;
    int walknum;

    void set_all(T1 v_, int mw_, T1 np_, int wn_){
        vertex = v_;
        master_worker = mw_;
        now_pivot = np_;
        walknum = wn_;
    }
};

template<typename T1>
class source_and_new_path{
private:
	friend class boost::serialization::access;
	template<class Archieve>
	void serialize(Archieve&ar, const unsigned int version)
	{
		ar& source;
        ar& new_path;
	}  
public:
    T1 source;
    T1 new_path;

    void set_all(T1 S_, T1 np_){
        source = S_;
        new_path = np_;
    }
};

// get global random walk samples (for each center(pivot) vertex)
// input:
//      *) g -> local graph
//      *) my_rank -> rank of each worker
//      *) size -> the number of worker
//      *) slave_to_master -> slave vertex to master worker map
//      *) master_to_slave -> master vertex to slave workers map
//      *) walk_num -> each sample's walk size
//      *) print_or_not -> if 1 -> then print the detail information
//      *) my_analyzer -> call by reference of custum analyzer (for testing the performance)
// output: 
//      *) v_to_walk -> a map (key: T1 - vertex ID, value: vector<T1> - vertex IDs (walk sample path)
template<typename T1>
void global_RW(graph<T1> * g, const int my_rank, const int size, 
                    map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, 
                    map<T1, vector<T1>> * v_to_walk, const int walk_num, int print_or_not, engine_analyzer & my_analyzer){
    // start
    std::chrono::high_resolution_clock::time_point temp_time;
    my_analyzer.time_start(temp_time);
    queue<RW_tuple<T1>> my_queue;
    RW_tuple<T1> temp;
    RW_tuple<T1> insert_temp;
    for(auto iter = master_to_slave->begin(); iter!=master_to_slave->end(); iter++){
        temp.set_all(iter->first, my_rank, iter->first, walk_num);
        my_queue.push(temp);
        (*v_to_walk)[iter->first].push_back(iter->first);
    }

    // 1) gather all out nei and its master
    set<T1> input_set;
    map<T1, vector<v_and_master<T1>>> global_neighbors;
    
    source_and_new_path<T1> snp_temp;
    
    set<vertex<T1>> * temp_v_set_p = g->get_v_set();
    for(auto iter = temp_v_set_p->begin(); iter!=temp_v_set_p->end(); iter++){
        input_set.insert(iter->get_id());
    }
    my_analyzer.time_end(0, temp_time);
    gather_vector_phase(slave_to_master, master_to_slave, &input_set,
                        my_rank, size, MERGE, boost::bind<void*>(local_get_outneighbors<T1>, g, my_rank, slave_to_master, master_to_slave),
                        &global_neighbors, my_analyzer);
    // 2) each has queue -> 하나씩 빼면서 랜덤하게 해주기, 이 큐에는 source v, master worker, now pivot, walknum 이 있을것이다.
    int cycle_num = 0;
    while(global_check_activate(&my_queue, my_rank, cycle_num, my_analyzer)>0){
        map<int, vector<RW_tuple<T1>>> worker_to_RWT;
        map<int, vector<source_and_new_path<T1>>> worker_to_SNP;
        while(!my_queue.empty()){
            my_analyzer.time_start(temp_time);
            std::vector<v_and_master<T1>> out;
            temp = my_queue.front();
            my_queue.pop();
            if(global_neighbors[temp.now_pivot].size()>0){
                std::sample(
                    global_neighbors[temp.now_pivot].begin(),
                    global_neighbors[temp.now_pivot].end(),
                    std::back_inserter(out),
                    1,
                    std::mt19937{std::random_device{}()}
                );
                if(temp.walknum>2){
                    insert_temp.set_all(temp.vertex, temp.master_worker, out[0].vertex, temp.walknum-1);
                    worker_to_RWT[out[0].master_worker].push_back(insert_temp);
                }
                snp_temp.set_all(temp.vertex, out[0].vertex);
                worker_to_SNP[temp.master_worker].push_back(snp_temp);
            }
            my_analyzer.time_end(0, temp_time);
        }
        my_analyzer.time_start(temp_time);
        MPI_Barrier(MPI_COMM_WORLD);
        my_analyzer.time_end(4, temp_time);
        vector<RW_tuple<T1>> rwt_temp;
        exchange_vec_data(my_rank, size, &worker_to_RWT, &rwt_temp, my_analyzer);
        for(int i = 0; i<rwt_temp.size(); i++){
            my_queue.push(rwt_temp[i]);
        }
        my_analyzer.time_start(temp_time);
        MPI_Barrier(MPI_COMM_WORLD);
        my_analyzer.time_end(4, temp_time);
        vector<source_and_new_path<T1>> recv_snp_temp;
        exchange_vec_data(my_rank, size, &worker_to_SNP, &recv_snp_temp, my_analyzer);
        for(int i = 0; i<recv_snp_temp.size(); i++){
            (*v_to_walk)[recv_snp_temp[i].source].push_back(recv_snp_temp[i].new_path);
        }
        cycle_num++;
        my_analyzer.time_start(temp_time);
        MPI_Barrier(MPI_COMM_WORLD);
        my_analyzer.time_end(4, temp_time);
    }
    // 3) 큐에서 빼고, 다음 버텍스를 찾기. 그리고 source v, master worker, new pivot, walknum -1 을 넣어주기
    // 4) 마스터로 보내기.
    if(my_rank==0 && print_or_not==1){
        int iter_count = 0; 
        for(auto iter=v_to_walk->begin(); iter!= v_to_walk->end(); iter++) {
            if(iter_count==1){
                break;
            }
            cout<<"RW: ";
            for(int i = 0; i<iter->second.size(); i++){
                cout<<iter->second[i]<< " ";
            }
            cout<<endl;
            iter_count++;
        }
    }
}

#endif
