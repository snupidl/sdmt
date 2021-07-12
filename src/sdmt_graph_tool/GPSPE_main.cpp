
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
#include "graph_stat_tools.h"
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
#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace std;


int main(int argc, char *argv[]){
    MPI_Init(&argc, &argv);
    int world_size;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    
    // input arguments parsing
    po::options_description global("Global options");
    global.add_options()
	("help", "print usage")
        ("command", po::value<std::string>(), "command to execute, stat: extract graph statistics, ps: recommend partitioning strategy")
	("subargs", po::value<std::vector<std::string> >(), "Arguments for command")
        ;

    po::positional_options_description pos;
    pos.add("command", 1).
        add("subargs", -1);

    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(argc, argv).
        options(global).
        positional(pos).
        allow_unregistered().
        run();

    po::store(parsed, vm);
    std::string cmd = "";
    if(vm.count("command")){
	cmd = vm["command"].as<std::string>();
	/*
	if(world_rank==0){
		cout << cmd <<endl;
	}
	*/
    }
    if (cmd == "stat")
    {
        // stat command has the following options:
        po::options_description stat_desc("stat options");
        stat_desc.add_options()
            ("print", "print the statistics of graph on the console")
            ("dumpfolder", po::value<std::string>(), "dump the stat infomation and other logs to the dumpfolder")
            ("input", po::value<std::string>(), "path to input graph file")
            ("direction", po::value<std::string>(), "the direction of the input graph file, 'directed' ot 'undirected'")
	    ("help", "print usage")
            ;

        // Collect all the unrecognized options from the first pass. This will include the
        // (positional) command name, so we need to erase that.
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        //opts.erase(opts.begin());

        // Parse again...
        po::store(po::command_line_parser(opts).options(stat_desc).run(), vm);
	
        if(vm.count("help")) { 
	    if(world_rank==0){
                std::cout << global << std::endl; 
                std::cout << stat_desc << std::endl; 
	    }
	    MPI_Barrier(MPI_COMM_WORLD);
    	    MPI_Finalize();
            return 0; 
        }
	
        std::string input_graph_path = "";
	if(vm.count("input")){
	    input_graph_path = vm["input"].as<std::string>();
	}
        bool dump  = vm.count("dumpfolder");
	std::string dump_path = "";
	if(dump){
	    dump_path = vm["dumpfolder"].as<std::string>();
	}
        bool print = vm.count("print");
        bool direction = true;
	std::string directed = "directed";
	if(vm.count("direction")){
	      directed =  vm["direction"].as<std::string>();
	}
        if(directed.compare("undirected")==0){
            direction = false;
        }

        get_graph_stat(input_graph_path, direction, world_rank, world_size, print, dump, dump_path);
    }
    else if (cmd == "ps")
    {
        // ls command has the following options:
        po::options_description ps_desc("partitioning strategies options");
        ps_desc.add_options()
            ("print", "print the rank of partitioning strategies")
            ("input_graph", po::value<std::string>(), "path to the input graph file")
            ("direction", po::value<std::string>(), "the direction of the input graph file, 'directed' ot 'undirected'")
            ("input_algo", po::value<std::string>(), "path to the input algorithm pseudocode file")
            ("dumpfolder", po::value<std::string>(), "dump the stat infomation and other logs to the dumpfolder")
            ("logfolder", po::value<std::string>(), "location that contain the stat infomation dump file / usually same with dumpfolder")
	    ("help", "print usage")
            //("modelfolder,mf", po::value<std::string>(), "location that contain the XGBoost model")
            //("pyfolder,pf", po::value<std::string>(), "location that contain the python code(containing the model)")
            //("algofeatureextractor,afe", po::value<std::string>(), "location of the algorithm feature extractor(JAR file)")
            ;

        // Collect all the unrecognized options from the first pass. This will include the
        // (positional) command name, so we need to erase that.
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);

        // Parse again...
        po::store(po::command_line_parser(opts).options(ps_desc).run(), vm);

        if(vm.count("help")) { 
	    if(world_rank==0){
                std::cout << global << std::endl; 
                std::cout << ps_desc << std::endl; 
	    }
	    MPI_Barrier(MPI_COMM_WORLD);
	    MPI_Finalize();
            return 0; 
        }
        std::string input_graph_path = vm["input_graph"].as<std::string>();
        std::string input_algo_path = vm["input_algo"].as<std::string>();
        std::string dump_path = vm["dumpfolder"].as<std::string>();
        std::string directed = vm["direction"].as<std::string>();
        
        std::string model_file     = "/usr/local/sdmt/sdmt_graph_tool/models/sdmt_graph_tool_xgboost_model";
        std::string scaler_file    = "/usr/local/sdmt/sdmt_graph_tool/models/sdmt_graph_tool_xgboost_scaler";
        std::string py_code        = "/usr/local/sdmt/sdmt_graph_tool/py_codes/sdmt_graph_tool_select_PS.py";
        std::string afe_file   = "/usr/local/sdmt/sdmt_graph_tool/java/sdmt_graph_tool_united_code_extract.jar";

        bool dump  = vm.count("dumpfolder");
        bool print = vm.count("print");
        bool direction = true;
        bool log = vm.count("logfolder");
        if(directed.compare("undirected")==0){
            direction = false;
        }
        std::string log_folder = dump_path;
        if(log==true){
            log_folder = vm["logfolder"].as<std::string>();
        }

        get_ps_rank(input_graph_path, input_algo_path, afe_file, direction, 
                 world_rank, world_size, print, dump, 
                 dump_path, log_folder, model_file, scaler_file, py_code);
    }
    else{
        if(vm.count("help")) {
	    if(world_rank==0){
	        std::cout<< global << std::endl;
	    }
	    MPI_Barrier(MPI_COMM_WORLD);
	    MPI_Finalize();
            return 0; 
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
