#ifndef GRAPH_CLASS_H
#define GRAPH_CLASS_H

/************************************
 * Introduction                     *
 ************************************/
// This file (graph_class.h) contains graph class (vertex, edge, and graph classes) and its member functions.
// Graph contains vertices and edges.
// Graph member functions are as follows.
// 1) Vertex, edge insertion
// 2) Find vertex/edge with ID
// 3) Print vertex/edge information
// 4) get vertex's degree/neighbor

#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string.h>
#include "mpi.h"

using namespace std;

#define GATHER_REQUEST      0
#define GATHER_RESPONSE     1
#define APPLY_DATAPASS      2
#define SCATTER_REQUEST     3
#define SCATTER_RESPONSE    4
#define DATAPASS            5
#define STOP                6
#define AGGREGATE_DATAPASS  7
#define GATHER_ON           8

#define SUM         100
#define MULTIPLY    101
#define MAX         104
#define MIN         105
#define MAXLOC      106
#define MINLOC      107
//Set
#define UNION       108
//Vector
#define MERGE       109

//Bit
#define OR          102
#define AND         103

#define IN_NEI      1000
#define OUT_NEI     1001
#define BOTH_NEI    1002

/************************************
 * Vertex Class                     *
 ************************************/
template <typename T1>
class vertex{
    protected:
        T1 id;
    public:
        // Constructor
        vertex()            {id = T1();}
        vertex(T1 id_)      {id = id_;}

        // Set/Get
        void set_id(T1 id_) {id = id_;}
        T1 get_id() const     {return id;}

        // Print
        void print()        {std::cout<<"Vertex ID: "<<id;}

        // Operator <
        bool operator <(const vertex<T1> & rhs) const   {return id<rhs.id;}

        // Operator ==
        bool operator ==(const vertex<T1> & rhs) const   {return id==rhs.id;}

        // Operator =
        vertex& operator=(vertex& rhs){
            id = rhs.id;
            return *this;
        }
};

/************************************
 * Edge Class                       *
 ************************************/
template <typename T1>
class edge{
    protected:
        T1 id;
        T1 src;
        T1 dst;
    public:
        // Constructor
        edge()                          {id = T1(); src = T1(); dst = T1();}
        edge(T1 id_)                    {id = id_; src = T1(); dst = T1();}
        edge(T1 id_, T1 src_, T1 dst_)  {id = id_; src = src_; dst = dst_;}

        // Set/Get
        void set_id(T1 id_)             {id = id_;}
        void set_src(T1 src_)           {src = src_;}
        void set_dst(T1 dst_)           {dst = dst_;}
        
        T1 get_id() const                   {return id;}
        T1 get_src() const               {return src;}
        T1 get_dst() const                   {return dst;}

        // Print
        void print()                    {std::cout<<"ID: "<< id << ",Src: "<<src<<", Dst: "<<dst;}

        // Operator <
        bool operator <(const edge<T1> & rhs) const{
            if(src!=rhs.src){
                return src<rhs.src;
            }
            else{
                if(dst!=rhs.dst){
                    return dst<rhs.dst;
                }
                else{
                    return id<rhs.id;
                }
            }
        }

        // Operator ==
        bool operator ==(const vertex<T1> & rhs) const   {return (id==rhs.id)&&(src==rhs.src)&&(dst==rhs.dst);}

        // Operator =
        edge& operator=(const edge& rhs){
            id = rhs.id;
            src = rhs.src;
            dst = rhs.dst;
            return *this;
        }
};

/************************************
 * Graph Class                      *
 ************************************/
template <typename T1>
class graph {
    protected:
        set<vertex<T1>> v_set;
        vector<edge<T1>> e_vec;
        vector<edge<T1>> inverted_e_vec;
        bool direct_T_undirect_F;
        map<T1, pair<int, int>> e_index;
        map<T1, pair<int, int>> inverted_e_index;
    public:
        // Constructor
        graph(){}
        graph(bool direct_T_undirect_F_){
            direct_T_undirect_F = direct_T_undirect_F_;
        }
        // Destructor
        ~graph(){
            v_set.clear();
            e_vec.clear();
            inverted_e_vec.clear();
            e_index.clear();
            inverted_e_index.clear();
        }

        // Vertex iterator
        typedef typename set<T1>::iterator iterator;
        inline iterator v_begin() noexcept { return v_set.begin();}
        inline iterator v_end() noexcept { return v_set.end();}

        // Print with pivot source vertex
        void graph_source_out_print(T1 source){
            if(v_set.find(vertex<T1>(source))!=v_set.end()){
                for(auto iter=e_vec.begin(); iter!=e_vec.end(); iter++){
                    if((*iter).get_src()==source){
                        cout<< "source: "<<source<<", dst: "<<(*iter).get_dst()<<endl;
                    }
                }
            }
        }

        // Get (inverted) edge index pointer
        map<T1, pair<int, int>> * get_inverted_e_index_p(){
            return (&inverted_e_index);
        }
        map<T1, pair<int, int>> * get_e_index_p(){
            return (&e_index);
        }

        // Get graph`s direction
        bool get_direct_undirect(){
            return direct_T_undirect_F;
        }

        // Get (inverted) edge vector pointer
        vector<edge<T1>> * get_e_vec_p(){
            return &e_vec;
        }
        vector<edge<T1>> * get_inverted_e_vec_p(){
            return &inverted_e_vec;
        }

        // Add vertex
        void add_vertex(T1 id_){
            v_set.insert(vertex<T1>(id_));
        }
        void add_vertex(vertex<T1> &v){
            v_set.insert(v);
        }

        // Vertex find
        bool find_vertex(T1 vid_){
            if(v_set.find(vertex<T1>(vid_))!=v_set.end()){
                return true;
            }
            else{
                return false;
            }
        }

        bool find_vertex(vertex<T1> & v){
            if(v_set.find(v)!=v_set.end()){
                return true;
            }
            else{
                return false;
            }
        }

        // Add edge
        void add_edge(edge<T1> & e){
            e_vec.push_back(e);
        }
        void add_edge(T1 id_, T1 src_, T1 dst_){
            edge<T1> e(id_, src_, dst_);
            e_vec.push_back(e);
        }
        void add_edge_with_vertex(T1 id_, T1 src_, T1 dst_){
            if(!find_vertex(src_)){
                add_vertex(src_);
            }
            if(!find_vertex(dst_)){
                add_vertex(dst_);
            }
            add_edge(id_, src_, dst_);
        }
        void add_edge_with_vertex(T1 id_, edge<T1> & e){
            T1 src_ = e.get_src();
            T1 dst_ = e.get_dst();
            if(!find_vertex(src_)){
                add_vertex(src_);
            }
            if(!find_vertex(dst_)){
                add_vertex(dst_);
            }
            add_edge(id_, src_, dst_);
        }
        void add_reverse_edge(T1 id_, edge<T1> & e){
            T1 src_ = e.get_src();
            T1 dst_ = e.get_dst();
            add_edge(id_, dst_, src_);
        }

        // Add inverted edge
        void add_inverted_edge(edge<T1> & e, T1 new_id_){
            edge<T1> temp_e(new_id_, e.get_dst(), e.get_src());
            inverted_e_vec.push_back(temp_e);
        }

        void convert_string_to_graph(char ** lines, int nlines, const int rank){
            int a, b;
            for (int i=0; i<nlines; i++) {
                if(lines[i][0]!='#'){ // # -> annotation
                    sscanf(lines[i],"%d %d", &a, &b);
                    //printf("%d: <%s>: %f + %f = %f\n", rank, lines[i], a, b, a+b);
                    add_edge_with_vertex(-1, a, b);
                }
            }
        }

        // change string vector to graph data (a, b = a -> b)
        void convert_string_to_graph(vector<string> lines, int nlines, const int rank){
            int a, b;
            int count = 0;
            for (int i=0; i<nlines; i++) {
                if(lines[i][0]!='#'){ // # -> annotation
                    sscanf(lines[i].c_str(), "%d %d" , &a, &b);
                    add_edge_with_vertex(-1, a, b);
                }
            }
        }

        // get vertex set
        set<vertex<T1>> * get_v_set(){
            return &v_set;
        }

        // get edge vector size
        int get_edge_size(){
            return e_vec.size();
        }

        // make inverted edge list, only for directed graph
        void make_inverted_e_vec(){
            if(direct_T_undirect_F==false){
                cout<<"This graph is undirected graph!!!! Cancel this code"<<endl;
                abort();
            }
            else{
                for(int i = 0; i<e_vec.size(); i++){
                    inverted_e_vec.push_back(edge<T1>(e_vec[i].get_id(), e_vec[i].get_dst(), e_vec[i].get_src())); //(T1 id_, T1 src_, T1 dst_)
                }
            }
        }

        // from a edge vector, make a graph structure
        void make_graph_from_e_vec(vector<edge<T1>> * e_vec_){
            for(int i = 0; i<e_vec_->size(); i++){
                add_edge((*e_vec_)[i]);
                add_vertex((*e_vec_)[i].get_src());
                add_vertex((*e_vec_)[i].get_dst());
            }
        }

        // Only for undirected graph
        // In the case of undirected graph, the edge a -> b is copied with inversed direction such as edge b -> a.
        void make_graph_from_e_vec_for_only_undirect(const int myrank, const int size){
            if(direct_T_undirect_F!=false){
                cout<<"This graph is directed graph!!!! Cancel this code"<<endl;
                abort();
            }
            int vec_size = e_vec.size();
            for(int i = 0; i<vec_size; i++){
                add_reverse_edge((vec_size+i)*size+myrank, e_vec[i]);
            }
        }

        // Print simple graph information
        void print_graph_info(int world_rank){
            cout<<"My rank: "<<world_rank << ", Vertex #: "<<v_set.size()<<", Edge #: "<<e_vec.size()<<endl;
        }

        void sort_e_vec(){
            std::sort(e_vec.begin(), e_vec.end());
        }

        void sort_inverted_e_vec(){
            std::sort(inverted_e_vec.begin(), inverted_e_vec.end());
        }

        // For the fast search time, our graph structure make the edge index structure for the source vertex ID.
        // In O(1), the graph engine can find the edge index that point the start of the edge with source vertex ID.
        void make_e_index(){
            int start, end;
            int start_can = 1;
            if(e_vec.size()==0){
                return;
            }
            T1 ver_id = e_vec[0].get_src();
            for(int i=0; i<e_vec.size(); i++){
                if(ver_id==e_vec[i].get_src()){
                    if(start_can==1){
                        start = i;
                        start_can = 0;
                    }
                }
                else{
                    end = i - 1;
                    e_index.insert(make_pair(ver_id, make_pair(start, end)));
                    ver_id = e_vec[i].get_src();
                    start = i;
                }

                if(i==e_vec.size()-1){
                    end = i;
                    e_index.insert(make_pair(ver_id, make_pair(start, end)));
                }
            }
        }

        // For the fast search time, our graph structure make the inverted edge index structure for the destination vertex ID.
        // In O(1), the graph engine can find the edge index that point the start of the inverted edge with source vertex ID.
        void make_inverted_e_index(){
            if(direct_T_undirect_F==false){
                cout<<"This graph is undirected graph!!!! Cancel this code"<<endl;
                return;
            }
            int start, end;
            int start_can = 1;
            if(inverted_e_vec.size()==0){
                return;
            }
            T1 ver_id = inverted_e_vec[0].get_src();
            for(int i=0; i<inverted_e_vec.size(); i++){
                if(ver_id==inverted_e_vec[i].get_src()){
                    if(start_can==1){
                        start = i;
                        start_can = 0;
                    }
                }
                else{
                    end = i - 1;
                    inverted_e_index.insert(make_pair(ver_id, make_pair(start, end)));
                    ver_id = inverted_e_vec[i].get_src();
                    start = i;
                }

                if(i==e_vec.size()-1){
                    end = i;
                    inverted_e_index.insert(make_pair(ver_id, make_pair(start, end)));
                }
            }
        }

        // input: to_this_or_to_out,  out/in/both direction degree (IN_NEI, OUT_NEI, BOTH_NEI)
        //        pivot_ver_id, a pivot vertex that the center of some edge.
        int get_vertex_degree(int to_this_or_to_out, const T1 pivot_ver_id){
            int degree_sum = 0;
            if(!find_vertex(pivot_ver_id)){ //없다면
                return 0;
            }
            if(direct_T_undirect_F==false){ //undi
                if(e_index.find(pivot_ver_id)!=e_index.end()){
                    return e_index[pivot_ver_id].second - e_index[pivot_ver_id].first + 1;
                }
                else{
                    return 0;
                }
            }
            else{
                if(to_this_or_to_out==IN_NEI){
                    if(inverted_e_index.find(pivot_ver_id)!=inverted_e_index.end()){
                        return inverted_e_index[pivot_ver_id].second - inverted_e_index[pivot_ver_id].first + 1;
                    }
                    else{
                        return 0;
                    }
                }
                else if(to_this_or_to_out==OUT_NEI){ // OUT_NEI
                    if(e_index.find(pivot_ver_id)!=e_index.end()){
                        return e_index[pivot_ver_id].second - e_index[pivot_ver_id].first + 1;
                    }
                    else{
                        return 0;
                    }
                }
                else if(to_this_or_to_out==BOTH_NEI){
                    int a = 0;
                    if(inverted_e_index.find(pivot_ver_id)!=inverted_e_index.end()){
                        a += (inverted_e_index[pivot_ver_id].second - inverted_e_index[pivot_ver_id].first + 1);
                    }
                    if(e_index.find(pivot_ver_id)!=e_index.end()){
                        a += (e_index[pivot_ver_id].second - e_index[pivot_ver_id].first + 1);
                    }
                    return a;
                }
                else{
                    cout<<"the input TO_THIS_OR_TO_OUT in function get_d_set_num_neighbors is error"<<endl;
                    abort();
                }
            }
        }

        // input: to_this_or_to_out,  out/in/both direction degree (IN_NEI, OUT_NEI, BOTH_NEI)
        //        pivot_ver_id, a pivot vertex that the center of some edge.
        // output: output_neighbor_vector, output vertex ID vector, neighbors of pivot_ver_id
        void get_vertex_neighbor(int to_this_or_to_out, const T1 pivot_ver_id, vector<T1> * output_neighbor_vector){
            int degree_sum = 0;
            if(!find_vertex(pivot_ver_id)){ //없다면
                return;
            }
            if(direct_T_undirect_F==false){ //undi
                if(e_index.find(pivot_ver_id)!=e_index.end()){
                    for(int i = e_index[pivot_ver_id].first; i<=e_index[pivot_ver_id].second; i++){
                        output_neighbor_vector->push_back(e_vec[i].get_dst());
                    }
                }
                return;
            }
            else{
                if(to_this_or_to_out==IN_NEI){
                    if(inverted_e_index.find(pivot_ver_id)!=inverted_e_index.end()){
                        for(int i = inverted_e_index[pivot_ver_id].first; i<=inverted_e_index[pivot_ver_id].second; i++){
                            output_neighbor_vector->push_back(inverted_e_vec[i].get_dst());
                        }
                    }
                }
                else if(to_this_or_to_out==OUT_NEI){ // OUT_NEI
                    if(e_index.find(pivot_ver_id)!=e_index.end()){
                        for(int i = e_index[pivot_ver_id].first; i<=e_index[pivot_ver_id].second; i++){
                            output_neighbor_vector->push_back(e_vec[i].get_dst());
                        }
                    }
                }
                else if(to_this_or_to_out==BOTH_NEI){
                    if(inverted_e_index.find(pivot_ver_id)!=inverted_e_index.end()){
                        for(int i = inverted_e_index[pivot_ver_id].first; i<=inverted_e_index[pivot_ver_id].second; i++){
                            output_neighbor_vector->push_back(inverted_e_vec[i].get_dst());
                        }
                    }
                    if(e_index.find(pivot_ver_id)!=e_index.end()){
                        for(int i = e_index[pivot_ver_id].first; i<=e_index[pivot_ver_id].second; i++){
                            output_neighbor_vector->push_back(e_vec[i].get_dst());
                        }
                    }
                }
                else{
                    cout<<"the input TO_THIS_OR_TO_OUT in function get_d_set_num_neighbors is error"<<endl;
                    abort();
                }
                return;
            }
        }

        // return the number of local vertex.
        int local_vertex_num(){
            return v_set.size();
        }

        // return the number of local edge.
        int local_edge_num(){
            return e_vec.size();
        }

        // return the max vertex ID in the local graph.
        T1 get_maxid_vertex(){
            T1 max_element = 0; 
            if (!v_set.empty()) 
                max_element = ((*v_set.rbegin()).get_id()); 
            // return the maximum element 
            return max_element; 
        }

        // this function is used in find_edge.
        // for the comparison of two edges.
        bool custom_function(edge<T1> & obj, T1 dst)  {
            return obj.get_dst() < dst; 
        }

        // find a local edge which has source vertex ID src and destination vertex ID dst.
        // if any_direction is true, then engine find both the original edge and inverted edge.
        bool find_edge(bool any_direction, T1 src, T1 dst){
            if(any_direction==true){
                bool temp = false;
                if(e_index[src].first!=-1){
                    if(binary_search(e_vec.begin()+e_index[src].first, e_vec.begin()+e_index[src].second+1, dst, custom_function)){
                        temp = true;
                    }
                }
                if(inverted_e_index[src].first!=-1){
                    if(binary_search(inverted_e_vec.begin()+inverted_e_index[src].first, 
                                     inverted_e_vec.begin()+inverted_e_index[src].second+1, dst, custom_function)){
                        temp = true;
                    }
                }
                return temp;
            }
            else{ // only true direction
                if(e_index[src].first!=-1){
                    if(binary_search(e_vec.begin()+e_index[src].first, e_vec.begin()+e_index[src].second+1, dst, custom_function)){
                        return true;
                    }
                    else{
                        return false;
                    }
                }
                else{
                    return false;
                }
            }
        }
};


#endif