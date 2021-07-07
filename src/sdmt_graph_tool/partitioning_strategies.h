#ifndef PARTITION_STRATEGY_H
#define PARTITION_STRATEGY_H

/************************************
 * Information                      *
 ************************************/
/*
 * Implementation of multiple partitioning strategies.
 * The strategies of GraphX, PowerGraph, and PowerLyra are implemented.
 * Our engine partitions the graph data using this method.
 */


#include <iostream>
#include <cmath>
#include <limits.h>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <unordered_set>
#include "graph_class.h"

using namespace std;

// basic functions, intersection
template <typename T1>
int set_intersect(const std::set<T1>& A_, const std::set<T1>& B_)
{
    std::unordered_set<int> aSet(A_.begin(), A_.end());
    return std::count_if(B_.begin(), B_.end(), [&](T1 element) { return aSet.find(element) != aSet.end(); });
}

template <typename T1>
int set_intersect(const std::unordered_set<T1>& A_, const std::set<T1>& B_)
{
    return std::count_if(B_.begin(), B_.end(), [&](T1 element) { return A_.find(element) != A_.end(); });
}

/************* Graph X partition strategies *******************/
// 1D-Edge partitioning a edge's src and dst ID.
template <typename T1>
int strategy_1D(const int max_machine, T1 src_, T1 dst_){ // 1 dimension edge partitioning
    return ((int)src_%max_machine);
}

// 1D-Edge partitioning a edge e.
template <typename T1>
int strategy_1D(const int max_machine, edge<T1> & e){ // 1 dimension edge partitioning
    return ((int)((int)e.get_src())%max_machine);
}

// Global 1D-Edge partitioning with strategy_1D(const int max_machine, edge<T1> & e).
template <typename T1>
void local_partition_strategy_1D(graph<T1> & my_local, const int rank, const int size, graph<T1> * others){ // 1 dimension edge partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        to_be_partitioned = strategy_1D(size, *iter);
        others[to_be_partitioned].add_edge(*iter);
    }
}

// 1D-Destination Edge partitioning a edge's src and dst ID.
template <typename T1>
int strategy_1D_DST(const int max_machine, T1 src_, T1 dst_){ // 1 dimension edge partitioning
    return ((int)dst_%max_machine);
}

// 1D-Destination Edge partitioning a edge e.
template <typename T1>
int strategy_1D_DST(const int max_machine, edge<T1> & e){ // 1 dimension edge partitioning
    return ((int)((int)e.get_dst())%max_machine);
}

// Global 1D-Destination Edge partitioning with strategy_1D_DST(const int max_machine, edge<T1> & e).
template <typename T1>
void local_partition_strategy_1D_DST(graph<T1> & my_local, const int rank, const int size, graph<T1> * others){ // 1 dimension edge dst partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        to_be_partitioned = strategy_1D_DST(size, *iter);
        others[to_be_partitioned].add_edge(*iter);
    }
}

// Random Edge partitioning a edge's src and dst ID.
template <typename T1>
int random_v_cut(const int max_machine, T1 src_, T1 dst_){ // Random vertex-cut partitioning
    /*
    var A = (ulong)(a >= 0 ? 2 * (long)a : -2 * (long)a - 1);
    var B = (ulong)(b >= 0 ? 2 * (long)b : -2 * (long)b - 1);
    var C = (long)((A >= B ? A * A + A + B : A + B * B) / 2);
    return a < 0 && b < 0 || a >= 0 && b >= 0 ? C : -C - 1;
    */
    unsigned long A = 2 * src_;
    unsigned long B = 2 * dst_;
    unsigned long C;
    if(A>=B){
        C = (A * A + A + B);
    }  
    else{
        C = (A + B * B)/2;
    }
    int answer = C % max_machine;
    return answer;
}

// Random Edge partitioning a edge e.
template <typename T1>
int random_v_cut(const int max_machine, edge<T1> & e){ // Random vertex-cut partitioning
    T1 src_ = e.get_src();
    T1 dst_ = e.get_dst();
   
    unsigned long A = 2 * src_;
    unsigned long B = 2 * dst_;
    unsigned long C;
    if(A>=B){
        C = (A * A + A + B);
    }  
    else{
        C = (A + B * B)/2;
    }
    int answer = C % max_machine;
    return answer;
}

// Global Random Edge partitioning with random_v_cut(const int max_machine, edge<T1> & e).
template <typename T1>
void local_partition_Random(graph<T1> & my_local, const int rank, const int size, graph<T1> * others){ // Random vertex-cut partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        to_be_partitioned = random_v_cut(size, *iter);
        others[to_be_partitioned].add_edge(*iter);
    }
}

// Canonical Random Edge partitioning a edge's src and dst ID.
template <typename T1>
int canonical_random_v_cut(const int max_machine, T1 src_, T1 dst_){ // Canonical random vertex-cut partitioning // PowerGraph Random
    if(dst_>src_){
        return random_v_cut(max_machine, src_, dst_);
    }
    else {
        return random_v_cut(max_machine, dst_, src_);
    }
}

// Global Canonical Random Edge partitioning with canonical_random_v_cut(const int max_machine, T1 src_, T1 dst_).
template <typename T1>
void local_partition_Canonical_Random(graph<T1> & my_local, const int rank, const int size, graph<T1> * others){ // Cano Random vertex-cut partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        if(iter->get_dst()>iter->get_src()){
            to_be_partitioned = random_v_cut(size, iter->get_src(), iter->get_dst());
        }
        else {
            to_be_partitioned = random_v_cut(size, iter->get_dst(), iter->get_src());
        }
        others[to_be_partitioned].add_edge(*iter);
    }
}

// 2D Edge partitioning a edge's src and dst ID.
template <typename T1>
int strategy_2D(const int max_machine, T1 src_, T1 dst_){ // 2 dimension edge partitioning // PowerGraph Grid
    int ceilSqrtMaxMachine = ceil(sqrt(max_machine));
    if(max_machine == ceilSqrtMaxMachine * ceilSqrtMaxMachine){
        int col = src_ % ceilSqrtMaxMachine;
        int row = dst_ % ceilSqrtMaxMachine;
        return (col * ceilSqrtMaxMachine + row) % max_machine;
    }
    else{
        int cols = ceilSqrtMaxMachine;
        int rows = (max_machine + cols -1) / cols;
        int lastRowsColoumn = max_machine - (rows-1) * (cols);
        int row = (src_ % max_machine) / cols;
        int col;
        if (row<rows-1){
            col = dst_ % cols;
        }
        else{
            col = dst_% lastRowsColoumn;
        }
        return (cols * row) + col;
    }
}

// 2 dimension edge partitioning
template <typename T1>
int strategy_2D(const int max_machine, edge<T1> & e){ // PowerGraph Grid
    T1 src_ = e.get_src();
    T1 dst_ = e.get_dst();
    int ceilSqrtMaxMachine = ceil(sqrt(max_machine));
    if(max_machine == ceilSqrtMaxMachine * ceilSqrtMaxMachine){
        int col = src_ % ceilSqrtMaxMachine;
        int row = dst_ % ceilSqrtMaxMachine;
        return (col * ceilSqrtMaxMachine + row) % max_machine;
    }
    else{
        int cols = ceilSqrtMaxMachine;
        int rows = (max_machine + cols -1) / cols;
        int lastRowsColoumn = max_machine - (rows-1) * (cols);
        int row = (src_ % max_machine) / cols;
        int col;
        if (row<rows-1){
            col = dst_ % cols;
        }
        else{
            col = dst_% lastRowsColoumn;
        }
        return (cols * row) + col;
    }
}

// Global 2D-Edge partitioning with strategy_2D(const int max_machine, edge<T1> & e).
template <typename T1>
void local_partition_2D(graph<T1> & my_local, const int rank, const int size, graph<T1> * others){ // 2D vertex-cut partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        to_be_partitioned = strategy_2D(size, *iter);
        others[to_be_partitioned].add_edge(*iter);
    }
}

/************* PowerGraph partition strategies *******************/
// Oblivious partitioning a edge e. This function needs to update vertex_to_workers and worker_to_edge_num maps.
// In each iteration(for edges), each machine has the new vertex set and the number of edges.
template <typename T1>
int powerGraph_Oblivious(const int rank, const int max_machine, edge<T1> & e, 
                        map<T1, set<int>> * vertex_to_workers, map<int, int> * worker_to_edge_num){ // Oblivious
    T1 src_ = e.get_src();
    T1 dst_ = e.get_dst();
    
    // case 1, has intersection
    set<int> intersect;
    set_intersection((*vertex_to_workers)[src_].begin(),(*vertex_to_workers)[src_].end(),
                     (*vertex_to_workers)[dst_].begin(),(*vertex_to_workers)[dst_].end(),
                     std::inserter(intersect,intersect.begin()));
    if(intersect.size()>0){
        auto iter = intersect.begin();
        int min_worker = *iter;
        int min_edge_num = (*worker_to_edge_num)[*iter];
        for(iter = intersect.begin(); iter != intersect.end(); iter++){
            if(min_edge_num>(*worker_to_edge_num)[*iter]){
                min_edge_num = (*worker_to_edge_num)[*iter];
                min_worker = *iter;
            }
        }
        (*worker_to_edge_num)[min_worker]++;
        return min_worker;
    }

    // case 2, not empty and do not intersect
    else if(intersect.size()==0 && (*vertex_to_workers)[src_].size()>0 && (*vertex_to_workers)[dst_].size()>0){
        auto iter = (*vertex_to_workers)[src_].begin();
        int min_worker = *iter;
        int min_edge_num = (*worker_to_edge_num)[*iter];
        for(iter = (*vertex_to_workers)[src_].begin(); iter != (*vertex_to_workers)[src_].end(); iter++){
            if(min_edge_num>(*worker_to_edge_num)[*iter]){
                min_edge_num = (*worker_to_edge_num)[*iter];
                min_worker = *iter;
            }
        }

        auto iter2 = (*vertex_to_workers)[dst_].begin();
        for(iter2 = (*vertex_to_workers)[dst_].begin(); iter2 != (*vertex_to_workers)[dst_].end(); iter2++){
            if(min_edge_num>(*worker_to_edge_num)[*iter2]){
                min_edge_num = (*worker_to_edge_num)[*iter2];
                min_worker = *iter2;
            }
        }
        (*vertex_to_workers)[src_].insert(min_worker);
        (*vertex_to_workers)[dst_].insert(min_worker);
        (*worker_to_edge_num)[min_worker]++;
        return min_worker;
    }

    // case 3, If only one of the two vertices has been assigned, then choose a machine from the assigned vertex
    else if( ((*vertex_to_workers)[src_].size()>0) && ((*vertex_to_workers)[dst_].size()==0) ){
        auto iter = (*vertex_to_workers)[src_].begin();
        int min_worker = *iter;
        int min_edge_num = (*worker_to_edge_num)[*iter];
        for(iter = (*vertex_to_workers)[src_].begin(); iter != (*vertex_to_workers)[src_].end(); iter++){
            if(min_edge_num>(*worker_to_edge_num)[*iter]){
                min_edge_num = (*worker_to_edge_num)[*iter];
                min_worker = *iter;
            }
        }
        (*vertex_to_workers)[src_].insert(min_worker);
        (*vertex_to_workers)[dst_].insert(min_worker);
        (*worker_to_edge_num)[min_worker]++;
        return min_worker;
    }
    else if( ((*vertex_to_workers)[dst_].size()>0) && ((*vertex_to_workers)[src_].size()==0) ){
        auto iter = (*vertex_to_workers)[dst_].begin();
        int min_worker = *iter;
        int min_edge_num = (*worker_to_edge_num)[*iter];
        for(iter = (*vertex_to_workers)[dst_].begin(); iter != (*vertex_to_workers)[dst_].end(); iter++){
            if(min_edge_num>(*worker_to_edge_num)[*iter]){
                min_edge_num = (*worker_to_edge_num)[*iter];
                min_worker = *iter;
            }
        }
        (*vertex_to_workers)[src_].insert(min_worker);
        (*vertex_to_workers)[dst_].insert(min_worker);
        (*worker_to_edge_num)[min_worker]++;
        return min_worker;
    }
    else{ // If neither vertex has been assigned, then assign the edge to the least loaded machine.
        auto iter = worker_to_edge_num->begin();
        int min_worker = iter->first;
        int min_edge_num =  iter->second;
        for(iter = worker_to_edge_num->begin(); iter != worker_to_edge_num->end(); iter++){
            if(min_edge_num>iter->second){
                min_edge_num = iter->second;
                min_worker = iter->first;
            }
        }
        (*vertex_to_workers)[src_].insert(min_worker);
        (*vertex_to_workers)[dst_].insert(min_worker);
        (*worker_to_edge_num)[min_worker]++;
        return min_worker;
    }
}

// Global Oblivious partitioning with powerGraph_Oblivious function.
template <typename T1>
void local_powerGraph_Oblivious(graph<T1> & my_local, const int rank, const int size, graph<T1> * others){ // 2D vertex-cut partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    map<T1, set<int>> vertex_to_workers;
    map<int, int> worker_to_edge_num;

    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        to_be_partitioned = powerGraph_Oblivious(rank, size, *iter, 
                        &vertex_to_workers, &worker_to_edge_num);
        others[to_be_partitioned].add_edge(*iter);
    }
}

// HDRF (High-Degree Replicated First) partitioning a edge e. This function needs to update vertex_to_workers and worker_to_edge_num maps.
// In each iteration(for edges), each machine has the new vertex set and the number of edges.
// First, locally partition the edges to vertex_to_PD, and then transmit this data to other workers.
template <typename T1>
int powerGraph_HDRF(const int max_machine, edge<T1> & e, float lamda,
                    map<T1, int> * vertex_to_PD, map<T1, set<int>> * vertex_to_workers, map<int, int> * worker_to_edge_num){ // Oblivious
    T1 src_ = e.get_src();
    T1 dst_ = e.get_dst();

    (*vertex_to_PD)[src_]++;
    (*vertex_to_PD)[dst_]++;

    float theta_src = (float)((*vertex_to_PD)[src_])/(float)((*vertex_to_PD)[src_]+(*vertex_to_PD)[dst_]);
    float theta_dst = 1.0 - theta_src;
    float g_src;
    float g_dst;
    float g_hdrf;
    int max_size = 0;
    int min_size = 0; 
    int max_worker = 0;
    int temp_size;
    float max_score = 0.0;
    float c_rep;
    float c_bal;
    float epsilon = 0.01;
    auto iter = worker_to_edge_num->begin();
    max_size = iter->second;
    min_size = iter->second;
    for(iter = worker_to_edge_num->begin(); iter != worker_to_edge_num->end(); iter ++){
        temp_size = iter->second;
        if(max_size<temp_size){
            max_size = temp_size;
        }
        else if(min_size>temp_size){
            min_size = temp_size;
        }
    }

    for(int i = 0; i<max_machine; i++){
        // REP
        g_src = 0.0;
        g_dst = 0,0;
        if((*vertex_to_workers)[src_].find(i)!=(*vertex_to_workers)[src_].end()){
            g_src = 2.0 - theta_src;
        }
        if((*vertex_to_workers)[dst_].find(i)!=(*vertex_to_workers)[dst_].end()){
            g_dst = 2.0 - theta_dst;
        }
        c_rep = g_src + g_dst;
        // BAL
        c_bal = lamda * (max_size-(*worker_to_edge_num)[i]) / (epsilon + max_size-min_size);
        g_hdrf = c_rep + c_bal;
        if(i==0){
            max_score = g_hdrf;
            max_worker = i;
        }
        else{
            if(max_score<g_hdrf){
                max_score = g_hdrf;
                max_worker = i;
            }
        }
    }

    (*vertex_to_workers)[src_].insert(max_worker);
    (*vertex_to_workers)[dst_].insert(max_worker);
    (*worker_to_edge_num)[max_worker]++;

    return max_worker;
}

// Global HDRF partitioning with powerGraph_HDRF.
template <typename T1>
void local_powerGraph_HDRF(graph<T1> & my_local, const int rank, const int size, graph<T1> * others, float lamda){ 
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    int to_be_partitioned;
    map<T1, int> vertex_to_PD;
    map<T1, set<int>> vertex_to_workers;
    map<int, int> worker_to_edge_num;

    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        to_be_partitioned = powerGraph_HDRF(size, *iter, lamda,
                                            &vertex_to_PD, &vertex_to_workers, &worker_to_edge_num);
        others[to_be_partitioned].add_edge(*iter);
    }
}

// hybrid first -> hash 1D DST

/************* PowerLyra partition strategies *******************/
// Global Hybrid.
template <typename T1>
void local_hybrid_second_phase(graph<T1> & my_local, const int rank, const int size, graph<T1> * others, const int threshold){
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    map<T1, vector<edge<T1>>> vset;
    int to_be_partitioned;

    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        vset[iter->get_dst()].push_back(*iter);
    }
    for(auto iter=vset.begin(); iter!=vset.end(); iter++){
        if(iter->second.size()<=threshold){
            for(int i = 0; i<iter->second.size(); i++){
                others[rank].add_edge(iter->second[i]);
            }
        }
        else{ //iter->second.size() > threshold
            for(int i = 0; i<iter->second.size(); i++){
                others[((int)(iter->second[i].get_src()))%size].add_edge(iter->second[i]);
            }
        }
    }
}

// Global Ginger.
template <typename T1>
void local_ginger_second_phase(graph<T1> & my_local, const int rank, const int size, graph<T1> * others, const int threshold,
                               const int allV, const int allE){ // 2D vertex-cut partitioning
    vector<edge<T1>> * edge_vec = my_local.get_e_vec_p();
    map<T1, vector<edge<T1>>> vset;
    int to_be_partitioned;
    T1 e_src;
    map<T1, set<T1>> v_dst_to_src_nei;
    set<vertex<T1>> * temp_v_set; 
    set<T1> Vp; 
    float score;
    float max_score;
    int numVp;
    int numEp;

    for(auto iter=edge_vec->begin(); iter!=edge_vec->end(); iter++){
        vset[iter->get_dst()].push_back(*iter);
        v_dst_to_src_nei[iter->get_dst()].insert(iter->get_src());
    }
    for(auto iter=vset.begin(); iter!=vset.end(); iter++){
        if(iter->second.size()<=threshold){
            for(int i = 0; i<size; i++){
                Vp.clear();
                temp_v_set = others[i].get_v_set();
                for(auto iter2=temp_v_set->begin(); iter2!=temp_v_set->end(); iter2++){
                    Vp.insert(iter2->get_id());
                }
                set<T1> intersect;
                set_intersection(Vp.begin(),Vp.end(),
                                v_dst_to_src_nei[iter->first].begin(),v_dst_to_src_nei[iter->first].end(),
                                std::inserter(intersect,intersect.begin()));
                numVp = others[i].local_vertex_num();
                numEp = others[i].local_edge_num();
                if(i==0){
                    to_be_partitioned = i;
                    max_score = (float)(intersect.size()) - ((float)numVp + ((float)allV)*((float)numEp)/((float)allE))/2.0;
                }
                else{
                    score = (float)(intersect.size()) - ((float)numVp + ((float)allV)*((float)numEp)/((float)allE))/2.0;
                    if(score>max_score){
                        max_score = score;
                        to_be_partitioned = i;
                    }
                }
            }
            for(int i = 0; i<iter->second.size(); i++){
                others[to_be_partitioned].add_edge_with_vertex(-1, iter->second[i]);
            }
        }
        else{ //iter->second.size() > threshold
            for(int i = 0; i<iter->second.size(); i++){
                to_be_partitioned = strategy_1D(size, iter->second[i]);
                others[to_be_partitioned].add_edge_with_vertex(-1, iter->second[i]);
            }
        }
    }
}


#endif

