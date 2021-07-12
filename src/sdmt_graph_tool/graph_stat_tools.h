#ifndef GRAPH_STAT_TOOLS_H
#define GRAPH_STAT_TOOLS_H

/************************************
 * Information                      *
 ************************************/
// in this code, there are functions related to extracting the graph data features (|V|, |E|, out/in-degree, ...) 
// and formmatting these features to TextTable class or save as text file.

#include <iostream>
#include <time.h>
#include <fstream>
#include <map>
#include <vector>
#include <string.h>
#include <chrono>
#include <sstream>

#include "mpi.h"
#include "partitioning_strategies.h"
#include "communication.h"
#include "global_file_IO.h"
#include "analyzer.h"
#include "global_graph_stat.h"
#include "global_graph.h"
#include "graph_class.h"
#include "TextTable.h"
#include "tools.h"
#include <boost/serialization/utility.hpp>
#include <boost/function.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>


#define IN_NEI      1000
#define OUT_NEI     1001
#define BOTH_NEI    1002

using namespace std;
namespace fs = boost::filesystem;

// input: graph_data_path = graph data's path
//        direct_true_or_undirect_false = if graph data is directed -> true, else -> false
//        world_rank, my_id, wid = worker's ID
//        world_size, size = the number of all workers
//        frequency_table = The name of the file to store the frequency tables for in/out-degrees. Two files are created later: frequency_table+"in.txt" and frequency_table+"out.txt".
//        graph_feature_file = This is the name of the file that stores the graph features used to recommend the partitioning strategy.
//        code_analysis_input = This is the file name. This file save the graph features used for code analysis.
// extract the graph data's features and formmat to TextTable and text files.
void get_graph_stat(string graph_data_path, bool direct_true_or_undirect_false, int world_rank, int world_size, bool print_on, bool dump_on, string dump_folder){
    // string graph_feature_file, string code_analysis_input
    
    
    engine_analyzer my_analyzer;
    engine_analyzer large_analyzer;

    /*
    string file_name = "";
    string only_file_name = "";
    string only_file_extension = "";

    int start_file_name = graph_data_path.rfind("/", graph_data_path.length());
    if (start_file_name != string::npos) {
        file_name = (graph_data_path.substr(start_file_name+1, graph_data_path.length() - start_file_name));
    }

    int start_file_extension = file_name.rfind(".", file_name.length());
    if (start_file_extension != string::npos) {
        only_file_extension = (file_name.substr(start_file_extension+1, file_name.length() - start_file_extension));
    }
    */

    int ps_num = 5;
    // "Hybrid";
    
    std::chrono::high_resolution_clock::time_point my_temp_time;
    std::chrono::high_resolution_clock::time_point large_temp_time;
    int allV;
    int allE;

    int temp_v=0;
    int temp_e=0;
    vector<pair<engine_analyzer, string>> analyzer_vec;
    vector<pair<engine_analyzer, string>> large_analyzer_vec;
    my_analyzer.init();
    large_analyzer.init();

    //data load
    my_analyzer.time_start(my_temp_time);
    large_analyzer.time_start(large_temp_time);
    if(world_rank==0){
        cout<< "Graph Analysis Start, graph name: "<< graph_data_path<<endl;
    }
    graph<int> g(direct_true_or_undirect_false);
    global_read_textfile_with_partitioning_strategy(graph_data_path, world_rank, world_size, 
                                                    direct_true_or_undirect_false, ps_num, temp_v, temp_e, &g);
    my_analyzer.time_end(0, my_temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "partition"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "partition_large"));
    MPI_Barrier(MPI_COMM_WORLD);

    // setting
    map<int, int> slave_to_master;
    map<int, vector<int>> master_to_slave;
    my_analyzer.init();
    large_analyzer.init();
    my_analyzer.time_start(my_temp_time);
    large_analyzer.time_start(large_temp_time);
    initialization_MS_communication_master_slave_list_master(&g, world_rank, world_size, &slave_to_master, &master_to_slave);
    my_analyzer.time_end(0, my_temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "init_setting"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "init_setting_large"));

    // simple
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    allV = get_number_of_global_vertex(&g, world_rank, &master_to_slave, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "get_#_of_V"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "get_#_of_V_large"));

    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    allE = get_number_of_global_edge(&g, world_rank, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "get_#_of_E"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "get_#_of_E_large"));
    
    map<int, int> in_degrees;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_all_vertices_in_degree_after(&g, world_rank, world_size, 
                        &slave_to_master, &master_to_slave, &in_degrees, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "In-degree"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "In-degree_large"));

    float tuple_num = my_count(&slave_to_master, &master_to_slave, &in_degrees);
    MPI_Barrier(MPI_COMM_WORLD);
    float in_degree_mean = my_mean(&slave_to_master, &master_to_slave, &in_degrees, tuple_num);
    MPI_Barrier(MPI_COMM_WORLD);
    float in_degree_std = my_std(&slave_to_master, &master_to_slave, &in_degrees, tuple_num, in_degree_mean);
    MPI_Barrier(MPI_COMM_WORLD);
    float in_degree_skewness = my_skeness(&slave_to_master, &master_to_slave, &in_degrees, tuple_num, in_degree_mean, in_degree_std);
    MPI_Barrier(MPI_COMM_WORLD);
    float in_degree_kurtosis = my_kurtosis(&slave_to_master, &master_to_slave, &in_degrees, tuple_num, in_degree_mean, in_degree_std);
    MPI_Barrier(MPI_COMM_WORLD);
    int in_degree_max = my_max(&slave_to_master, &master_to_slave, &in_degrees, world_rank, world_size, 0);
    MPI_Barrier(MPI_COMM_WORLD);
    //string indegree_frequency_table = frequency_table+"_indegree_histogram.txt";
    //historam(world_rank, &slave_to_master, &master_to_slave, &in_degrees, indegree_frequency_table);
    TextTable indegree_histogram = historam_table(world_rank, &slave_to_master, &master_to_slave, &in_degrees);


    map<int, int> vertex_to_outdegree;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_all_vertices_out_degree(&g, world_rank, world_size, 
                        &slave_to_master, &master_to_slave, vertex_to_outdegree, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "Out-degree"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "Out-degree_large"));
    
    float out_degree_mean = my_mean(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num);
    float out_degree_std = my_std(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num, out_degree_mean);
    float out_degree_skewness = my_skeness(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num, out_degree_mean, out_degree_std);
    float out_degree_kurtosis = my_kurtosis(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num, out_degree_mean, out_degree_std);
    int out_degree_max = my_max(&slave_to_master, &master_to_slave, &vertex_to_outdegree, world_rank, world_size, 0);
    //string outdegree_frequency_table = frequency_table+"_outdegree_histogram.txt";
    //historam(world_rank, &slave_to_master, &master_to_slave, &vertex_to_outdegree, outdegree_frequency_table);
    TextTable outdegree_histogram = historam_table(world_rank, &slave_to_master, &master_to_slave, &vertex_to_outdegree);

    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    float triangles = get_triangles(&g, world_rank, world_size, 
            &slave_to_master, &master_to_slave, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "Triangle_Count"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "Triangle_Count_large"));
    
    int num_all_v = allV;
    float all_triplet = (float)(num_all_v/ (float)(3*2)) * (float)(num_all_v-1) * (float)(num_all_v-2);
    float GCC = ((triangles))/all_triplet;

    map<int, int> vertex_to_color;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_greedy_coloring(&g, world_rank, world_size, 
            &slave_to_master, &master_to_slave,
            &vertex_to_color, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "Coloring"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "Coloring_large"));
    int color_max = my_max(&slave_to_master, &master_to_slave, &vertex_to_color, world_rank, world_size, 0);

    vector<string> features;
    vector<string> values;
    vector<string> graph_feature_values;
    vector<string> code_analysis_inputs;

    if(world_rank==0){
        features.push_back("Features");
        features.push_back("|V|");
        features.push_back("|E|");

        features.push_back("In-degree, mean");
        features.push_back("In-degree, std");
        features.push_back("In-degree, skewness");
        features.push_back("In-degree, kurtosis");
        features.push_back("In-degree, max");

        features.push_back("Out-degree, mean");
        features.push_back("Out-degree, std");
        features.push_back("Out-degree, skewness");
        features.push_back("Out-degree, kurtosis");
        features.push_back("Out-degree, max");

        features.push_back("Number of triangles");
        features.push_back("Global clustering coefficient");
        features.push_back("Graph coloring, max color");
        features.push_back("Direction");
    }

    if(world_rank==0){
        values.push_back("Values");
        values.push_back(to_string(allV));
        values.push_back(to_string(allE));
        values.push_back(to_string(in_degree_mean));
        values.push_back(to_string(in_degree_std));
        values.push_back(to_string(in_degree_skewness));
        values.push_back(to_string(in_degree_kurtosis));
        values.push_back(to_string(in_degree_max));
        values.push_back(to_string(out_degree_mean));
        values.push_back(to_string(out_degree_std));
        values.push_back(to_string(out_degree_skewness));
        values.push_back(to_string(out_degree_kurtosis));
        values.push_back(to_string(out_degree_max));
        values.push_back(to_string(triangles));
        values.push_back(to_string(GCC));
        values.push_back(to_string(color_max));
        if(direct_true_or_undirect_false==true){
            values.push_back("directed");
        }
        else{
            values.push_back("undirected");
        }
    }
    TextTable t( '-', '|', '+' );
    if(world_rank==0){
        for(int feature_print=0; feature_print<features.size(); feature_print++){
            t.add(features[feature_print]);
            t.add(values[feature_print]);
            t.endOfRow();
        }
    }

    if(world_rank==0){
        graph_feature_values.push_back(to_string(allV));
        graph_feature_values.push_back(to_string(allE));
        graph_feature_values.push_back(to_string(out_degree_mean));
        graph_feature_values.push_back(to_string(out_degree_std));
        graph_feature_values.push_back(to_string(out_degree_skewness));
        graph_feature_values.push_back(to_string(out_degree_kurtosis));
        graph_feature_values.push_back(to_string(in_degree_mean));
        graph_feature_values.push_back(to_string(in_degree_std));
        graph_feature_values.push_back(to_string(in_degree_skewness));
        graph_feature_values.push_back(to_string(in_degree_kurtosis));
        if(direct_true_or_undirect_false==true){
            graph_feature_values.push_back("directed");
        }
        else{
            graph_feature_values.push_back("undirected");
        }
    }
    if(world_rank==0){
        fs::path graph_file(graph_data_path);
        string dump_file_name = graph_file.stem().string() + "_data_feature.txt";
        fs::path dump_file_path = fs::path(dump_folder) / fs::path(dump_file_name);
        fs::ofstream ofs(dump_file_path);
        //fs::path full_path (graph_feature_file);
        //cout<<graph_feature_file<<endl;
        //fs::ofstream ofs(full_path);
        for(int i =0 ; i<graph_feature_values.size(); i++){
            if(i==0){
                ofs<<graph_feature_values[i];
            }
            else{
                ofs<<","<<graph_feature_values[i];
            }
        }
        ofs<<"\n";
        ofs.close();
    }

    if(world_rank==0){
        code_analysis_inputs.push_back("0");
        code_analysis_inputs.push_back(to_string(allV));
        code_analysis_inputs.push_back(to_string(allE));
        code_analysis_inputs.push_back(to_string(in_degree_mean));
        code_analysis_inputs.push_back(to_string(out_degree_mean));

        if(direct_true_or_undirect_false==true){
            code_analysis_inputs.push_back("directed");
        }
        else{
            code_analysis_inputs.push_back("undirected");
        }
    }
    if(world_rank==0){
        //fs::path full_path (code_analysis_input);
        //cout<<code_analysis_input<<endl;
        //fs::ofstream ofs(full_path);
        fs::path graph_file(graph_data_path);
        string dump_file_name = graph_file.stem().string() + "_codeanalysis_graph_features.txt";
        fs::path dump_file_path = fs::path(dump_folder) / fs::path(dump_file_name);
        fs::ofstream ofs(dump_file_path);
        ofs<<code_analysis_inputs[0]<<","<<code_analysis_inputs[1]<<","<<code_analysis_inputs[2]<<","<<code_analysis_inputs[3]<<","<<code_analysis_inputs[4]<<","<<code_analysis_inputs[5]<<"\n";
        ofs.close();
    }

    if(world_rank==0 && print_on==true){
        cout<< "GRAPH STAT, GRAPH DATA: "<<graph_data_path<<", STATS:"<<endl;
        cout<< t << "\n";
	cout<< "Histogram of in-degree: \n" << indegree_histogram <<endl;
        cout<< "Histogram of out-degree: \n" << outdegree_histogram <<endl;
    }

    if(world_rank==0 && dump_on==true){
        fs::path graph_file(graph_data_path);
        string dump_file_name = graph_file.stem().string() + "_stat_dump.txt";
        fs::path dump_file_path = fs::path(dump_folder) / fs::path(dump_file_name);
        fs::ofstream ofs(dump_file_path);
        //std::cout << "filename and extension : " << p.filename() << std::endl; // file.ext
        //std::cout << "filename only          : " << p.stem() << std::endl;     // file
        //fs::path full_path (dump_folder);
        //fs::path dump_path = full_path / fs::path();
        //fs::ofstream ofs(full_path);
        ofs << "GRAPH STAT, GRAPH DATA: "<<graph_data_path<<", STATS: \n";
        ofs << t << "\n";
        ofs << "Histogram of in-degree: \n" << indegree_histogram << "\n";
        ofs << "Histogram of out-degree: \n" << outdegree_histogram << "\n";
        ofs.close();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    
    return;
}

// input: graph_data_path = graph data's path
//        direct_true_or_undirect_false = if graph data is directed -> true, else -> false
//        world_rank, my_id, wid = worker's ID
//        world_size, size = the number of all workers
// output: out_file_name = output file's path
// extract the graph data's features and save this information to out_file_name.
void get_graph_features(string graph_data_path, bool direct_true_or_undirect_false, int world_rank, int world_size, string out_file_name){
    namespace fs = boost::filesystem;
    engine_analyzer my_analyzer;
    engine_analyzer large_analyzer;
    int ps_num = 5;
    // "Hybrid";
    std::chrono::high_resolution_clock::time_point my_temp_time;
    std::chrono::high_resolution_clock::time_point large_temp_time;
    int allV;
    int allE;

    int temp_v=0;
    int temp_e=0;
    vector<pair<engine_analyzer, string>> analyzer_vec;
    vector<pair<engine_analyzer, string>> large_analyzer_vec;
    my_analyzer.init();
    large_analyzer.init();

    //data load
    my_analyzer.time_start(my_temp_time);
    large_analyzer.time_start(large_temp_time);
    graph<int> g(direct_true_or_undirect_false);
    global_read_textfile_with_partitioning_strategy(graph_data_path, world_rank, world_size, 
                                                    direct_true_or_undirect_false, ps_num, temp_v, temp_e, &g);
    my_analyzer.time_end(0, my_temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "partition"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "partition_large"));
    MPI_Barrier(MPI_COMM_WORLD);

    // setting
    map<int, int> slave_to_master;
    map<int, vector<int>> master_to_slave;
    my_analyzer.init();
    large_analyzer.init();
    my_analyzer.time_start(my_temp_time);
    large_analyzer.time_start(large_temp_time);
    initialization_MS_communication_master_slave_list_master(&g, world_rank, world_size, &slave_to_master, &master_to_slave);
    my_analyzer.time_end(0, my_temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "init_setting"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "init_setting_large"));

    // simple
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    allV = get_number_of_global_vertex(&g, world_rank, &master_to_slave, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "get_#_of_V"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "get_#_of_V_large"));

    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    allE = get_number_of_global_edge(&g, world_rank, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "get_#_of_E"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "get_#_of_E_large"));
    
    map<int, int> in_degrees;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_all_vertices_in_degree_after(&g, world_rank, world_size, 
                        &slave_to_master, &master_to_slave, &in_degrees, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "In-degree"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "In-degree_large"));

    float tuple_num = my_count(&slave_to_master, &master_to_slave, &in_degrees);
    float in_degree_mean = my_mean(&slave_to_master, &master_to_slave, &in_degrees, tuple_num);
    float in_degree_std = my_std(&slave_to_master, &master_to_slave, &in_degrees, tuple_num, in_degree_mean);
    float in_degree_skewness = my_skeness(&slave_to_master, &master_to_slave, &in_degrees, tuple_num, in_degree_mean, in_degree_std);
    float in_degree_kurtosis = my_kurtosis(&slave_to_master, &master_to_slave, &in_degrees, tuple_num, in_degree_mean, in_degree_std);

    map<int, int> vertex_to_outdegree;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_all_vertices_out_degree(&g, world_rank, world_size, 
                        &slave_to_master, &master_to_slave, vertex_to_outdegree, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "Out-degree"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "Out-degree_large"));
    
    float out_degree_mean = my_mean(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num);
    float out_degree_std = my_std(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num, out_degree_mean);
    float out_degree_skewness = my_skeness(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num, out_degree_mean, out_degree_std);
    float out_degree_kurtosis = my_kurtosis(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num, out_degree_mean, out_degree_std);

    vector<string> values;

    if(world_rank==0){
        values.push_back(to_string(allV));
        values.push_back(to_string(allE));

        values.push_back(to_string(out_degree_mean));
        values.push_back(to_string(out_degree_std));
        values.push_back(to_string(out_degree_skewness));
        values.push_back(to_string(out_degree_kurtosis));

        values.push_back(to_string(in_degree_mean));
        values.push_back(to_string(in_degree_std));
        values.push_back(to_string(in_degree_skewness));
        values.push_back(to_string(in_degree_kurtosis));

        if(direct_true_or_undirect_false==true){
            values.push_back("directed");
        }
        else{
            values.push_back("undirected");
        }
    }
    if(world_rank==0){
        fs::path full_path (out_file_name);
        cout<<out_file_name<<endl;
        fs::ofstream ofs(full_path);
        for(int i =0 ; i<values.size(); i++){
            if(i==0){
                ofs<<values[i];
            }
            else{
                ofs<<","<<values[i];
            }
        }
        ofs<<"\n";
        ofs.close();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    return;
}

// input: graph_data_path = graph data's path
//        direct_true_or_undirect_false = if graph data is directed -> true, else -> false
//        world_rank, my_id, wid = worker's ID
//        world_size, size = the number of all workers
// output: out_file_name = output file's path
// extract the graph data's features (that only needed to extract the algorithm features) and save this information to out_file_name.
void get_graph_code_analysis_features(string graph_data_path, bool direct_true_or_undirect_false, int world_rank, int world_size, string out_file_name){
    namespace fs = boost::filesystem;
    engine_analyzer my_analyzer;
    engine_analyzer large_analyzer;
    int ps_num = 5;
    // "Hybrid";
    std::chrono::high_resolution_clock::time_point my_temp_time;
    std::chrono::high_resolution_clock::time_point large_temp_time;
    int allV;
    int allE;

    int temp_v=0;
    int temp_e=0;
    vector<pair<engine_analyzer, string>> analyzer_vec;
    vector<pair<engine_analyzer, string>> large_analyzer_vec;
    my_analyzer.init();
    large_analyzer.init();

    //data load
    my_analyzer.time_start(my_temp_time);
    large_analyzer.time_start(large_temp_time);
    graph<int> g(direct_true_or_undirect_false);
    global_read_textfile_with_partitioning_strategy(graph_data_path, world_rank, world_size, 
                                                    direct_true_or_undirect_false, ps_num, temp_v, temp_e, &g);
    my_analyzer.time_end(0, my_temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "partition"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "partition_large"));
    MPI_Barrier(MPI_COMM_WORLD);

    // setting
    map<int, int> slave_to_master;
    map<int, vector<int>> master_to_slave;
    my_analyzer.init();
    large_analyzer.init();
    my_analyzer.time_start(my_temp_time);
    large_analyzer.time_start(large_temp_time);
    initialization_MS_communication_master_slave_list_master(&g, world_rank, world_size, &slave_to_master, &master_to_slave);
    my_analyzer.time_end(0, my_temp_time);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "init_setting"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "init_setting_large"));

    // simple
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    allV = get_number_of_global_vertex(&g, world_rank, &master_to_slave, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "get_#_of_V"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "get_#_of_V_large"));

    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    allE = get_number_of_global_edge(&g, world_rank, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "get_#_of_E"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "get_#_of_E_large"));
    
    map<int, int> in_degrees;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_all_vertices_in_degree_after(&g, world_rank, world_size, 
                        &slave_to_master, &master_to_slave, &in_degrees, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "In-degree"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "In-degree_large"));

    float tuple_num = my_count(&slave_to_master, &master_to_slave, &in_degrees);
    float in_degree_mean = my_mean(&slave_to_master, &master_to_slave, &in_degrees, tuple_num);

    map<int, int> vertex_to_outdegree;
    my_analyzer.init();
    large_analyzer.init();
    large_analyzer.time_start(large_temp_time);
    get_all_vertices_out_degree(&g, world_rank, world_size, 
                        &slave_to_master, &master_to_slave, vertex_to_outdegree, 0, my_analyzer);
    MPI_Barrier(MPI_COMM_WORLD);
    large_analyzer.time_end(0, large_temp_time);
    analyzer_vec.push_back(make_pair(my_analyzer, "Out-degree"));
    large_analyzer_vec.push_back(make_pair(large_analyzer, "Out-degree_large"));
    
    float out_degree_mean = my_mean(&slave_to_master, &master_to_slave, &vertex_to_outdegree, tuple_num);

    vector<string> values;

    if(world_rank==0){
        values.push_back("0");
        values.push_back(to_string(allV));
        values.push_back(to_string(allE));

        values.push_back(to_string(in_degree_mean));

        values.push_back(to_string(out_degree_mean));

        if(direct_true_or_undirect_false==true){
            values.push_back("directed");
        }
        else{
            values.push_back("undirected");
        }
    }
    if(world_rank==0){
        fs::path full_path (out_file_name);
        fs::ofstream ofs(full_path);
        ofs<<values[0]<<","<<values[1]<<","<<values[2]<<","<<values[3]<<","<<values[4]<<","<<values[5]<<"\n";
        ofs.close();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    return;
}

// get_graph_stat
void get_ps_rank(string graph_data_path, string algo_pseudocode_path, string algo_feature_extractor, bool direct_true_or_undirect_false, 
                 int world_rank, int world_size, bool print_on, bool dump_on, 
                 string dump_folder, string log_folder, string model_location, string scaler_location, string py_file_location){
    // 1) find the stat log file 
    fs::path graph_file(graph_data_path);
    fs::path algo_file(algo_pseudocode_path);
    string feature_file_name = graph_file.stem().string() + "_data_feature.txt";
    fs::path data_feature = fs::path(log_folder) / fs::path(feature_file_name);
    feature_file_name = graph_file.stem().string() + "_codeanalysis_graph_features.txt";
    fs::path code_feature = fs::path(log_folder) / fs::path(feature_file_name);
    feature_file_name = graph_file.stem().string() + "_" + algo_file.stem().string() + "_algo_feature.txt";
    fs::path algo_feature = fs::path(log_folder) / fs::path(feature_file_name);
    //fs::path py_folder_path(py_folder);
    //fs::path py_file = py_folder_path + / fs::path("hpc_select_PS.py");
    string py_out_file_name = graph_file.stem().string() + "_" + algo_file.stem().string() + "_PS_ranks.txt";
    
    // 2-1) if exist
    if(fs::exists(data_feature) && fs::exists(code_feature)){
        // do nothing
    }
    // 2-2) not exist
    else{
        // make that files
        get_graph_stat(graph_data_path, direct_true_or_undirect_false, world_rank, world_size, print_on, dump_on, log_folder);
    }
    // 3) code analysis
    if(world_rank==0){
        string java_call_string = std::string("java -jar ") 
                                    + algo_feature_extractor + string(" ") 
                                    + code_feature.string()  + string(" ")
                                    + algo_pseudocode_path   + string(" ")
                                    + algo_feature.string()  + string(" 1> /dev/null");
        // /mirror/mpiu/HPC_GPSPE/real_hpc_code/code_feature_extractor/United_Code_extract_HPC.jar
        system(java_call_string.c_str());
	string del_temp = std::string("rm ./") +   algo_file.stem().string() + string("_0_intermediate.txt");
	system(del_temp.c_str());
	string del_temp2 = std::string("rm ./elapsedTime_all_case.txt");
	system(del_temp2.c_str());
        // 4) get ps rank
        string python_call_string = std::string("python3 ") + py_file_location + string(" ") + data_feature.string()
                                    + std::string(" ")+ algo_feature.string()
                                    + std::string(" ")+ scaler_location
                                    + std::string(" ")+ model_location;
        if(print_on==true && dump_on == true){
            fs::path py_output = fs::path(dump_folder) / fs::path(py_out_file_name);
            python_call_string += (string(" | tee ") + py_output.string());
        }
        else if(print_on==false && dump_on == true){
            fs::path py_output = fs::path(dump_folder) / fs::path(py_out_file_name);
            python_call_string += (string(" 1> ")+ py_output.string());
        }
        else if(print_on==true && dump_on == false){
            // do nothing
        }
        system(python_call_string.c_str());
    }
    
}

#endif
