#ifndef TOOLS_H
#define TOOLS_H

/************************************
 * Information                      *
 ************************************/
/*
 * This file contains basic mathematical functions 
 * (intersection, count, mean, std, skewness, kurtosis, max). 
 * In addition, it includes a split function for strings 
 * and a function to merge multiple string vectors into one string.
 */

#include <algorithm> //std::sort
#include <iostream> //std::cout
#include <string> //std::string
#include <vector> //std::vector
#include <fstream> 
#include <cmath>
#include "TextTable.h"
#include "mpi.h"

using namespace std;


/*
 * Collects the common elements of vectors v1 and v2 and returns a vector.
 */
template<typename T1>
std::vector<T1> my_intersection(std::vector<T1> &v1,
                             std::vector<T1> &v2){
    std::vector<T1> v3;

    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());

    std::set_intersection(v1.begin(),v1.end(),
                          v2.begin(),v2.end(),
                          back_inserter(v3));
    return v3;
}

template<typename T1, typename T2>
void historam(int worker_ID, map<T1, int> * slave_to_master, 
		     map<T1, vector<int>> * master_to_slave, 
		     map<T1, T2> * some_values, std::string & output_file_path_and_name){
	int local_min = 10000000, local_max = 0, global_min, global_max;
	int slot_size;
	int count_array[10];
	int global_array[10];
	string out_string = "";
	//if(worker_ID==0)
		//cout<<"t1"<<endl;
	for(int i = 0; i<10; i++){
		count_array[i] = 0;
	}
	if(some_values->size()>0){
		local_min = some_values->begin()->second;
		local_max = some_values->begin()->second;
	}
	//if(worker_ID==0)
		//cout<<"t2"<<endl;
	for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
		if(iter->second < local_min){
			local_min = iter->second;
		}
		if(iter->second > local_max){
			local_max = iter->second;
		}
	}
	//if(worker_ID==0)
		//cout<<"t3"<<endl;
	MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
	MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	//if(worker_ID==0)
		//cout<<"t4"<<endl;
	slot_size = (global_max-global_min+9)/10;
	for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
    		count_array[(iter->second - global_min)/slot_size]++;
        }
	}
	MPI_Reduce(count_array, global_array, 10,  MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	//if(worker_ID==0)
		//cout<<"t5"<<endl;
	if(worker_ID == 0){
		ofstream writeFile;
		writeFile.open(output_file_path_and_name);
		int temp = global_min;
        writeFile << "Degree(Greater than or equal to ~ Less than), Frequency\n";
		for(int i = 0; i<10; i++){
			writeFile << temp << "~" << (temp+slot_size) << "," << global_array[i] << "\n";
			temp += slot_size;
		}
		writeFile.close();
		//cout<<"t6"<<endl;
	}
	return;
}

template<typename T1, typename T2>
TextTable historam_table(int worker_ID, map<T1, int> * slave_to_master, 
		     map<T1, vector<int>> * master_to_slave, 
		     map<T1, T2> * some_values){
    TextTable t( '-', '|', '+' );
    t.add("Degree(Greater than or equal to ~ Less than)");
    t.add("Frequency");
    t.endOfRow();
	int local_min = 10000000, local_max = 0, global_min, global_max;
	int slot_size;
    vector<int> count_array(10);
    vector<int> global_array(10);
	//int count_array[10];
	//int global_array[10];
	string out_string = "";

	if(some_values->size()>0){
		local_min = some_values->begin()->second;
		local_max = some_values->begin()->second;
	}

	for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
		if(iter->second < local_min){
			local_min = iter->second;
		}
		if(iter->second > local_max){
			local_max = iter->second;
		}
	}

	MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
	MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

	slot_size = (global_max-global_min+9)/10;
	for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
    		count_array[(iter->second - global_min)/slot_size]++;
        }
	}

	MPI_Reduce(count_array.data(), global_array.data(), 10,  MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if(worker_ID == 0){
        vector<string> degree;
        // vector<string> frequency;
        int temp = global_min;

        //degree.push_back("Degree(Greater than or equal to ~ Less than)");
        //frequency.push_back("Frequency");

        for(int i = 0; i<10; i++){
            degree.push_back(to_string(temp)+"~"+to_string(temp+slot_size));
            temp += slot_size;
        }

        for(int i = 0; i<10; i++){
            t.add(degree[i]);
            t.add(to_string(global_array[i]));
            t.endOfRow();
        }
	}
	return t;
}

// Count master objects.
template<typename T1, typename T2>
float my_count(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, T2> * some_values){
    int my_count =0;
    int global_count = 0;
    my_count = master_to_slave->size();
    MPI_Allreduce(&my_count, &global_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    return ((float)global_count);
}

// Calculate the mean of the values of some_values map.
template<typename T1, typename T2>
float my_mean(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, T2> * some_values, float count){

    int my_value = 0;

    int global_value = 0;
    for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
            my_value += iter->second;
        }
    }

    MPI_Allreduce(&my_value, &global_value, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    return ((float)global_value)/count;
}

// Calculate the std of the values of some_values map.
template<typename T1, typename T2>
float my_std(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, T2> * some_values, float count, float mean){
    float my_value = 0;
    float global_value = 0;
    for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
            my_value += (iter->second - mean) * (iter->second - mean) / count;
        }
    }
    MPI_Allreduce(&my_value, &global_value, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    global_value = sqrt(global_value);

    return global_value;
}

// Calculate the skewness of the values of some_values map.
template<typename T1, typename T2>
float my_skeness(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, T2> * some_values, float count, float mean, float std){
    float my_value = 0;
    float global_value = 0;
    float temp;
    for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
            temp = (iter->second - mean)/std;
            my_value += (temp * temp * temp) / count;
        }
    }
    MPI_Allreduce(&my_value, &global_value, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    return global_value;
}

// Calculate the kurtosis of the values of some_values map.
template<typename T1, typename T2>
float my_kurtosis(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, T2> * some_values, float count, float mean, float std){
    float my_value = 0;
    float global_value = 0;
    float temp;
    for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
            temp = (iter->second - mean)/std;
            my_value += (temp * temp * temp * temp) / count;
        }
    }
    MPI_Allreduce(&my_value, &global_value, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    global_value -= 3.0;

    return global_value;
}

// Calculate the max of the values of some_values map.
template<typename T1, typename T2>
T2 my_max(map<T1, int> * slave_to_master, map<T1, vector<int>> * master_to_slave, map<T1, T2> * some_values, int my_rank, int cluster_size, T2 default_v){

    T2 my_value = default_v;

    int global_value = 0;
    for(auto iter = some_values->begin(); iter!= some_values->end(); iter++){
        if(master_to_slave->find(iter->first)!=master_to_slave->end()){
            if(my_value<iter->second){
                my_value = iter->second;
            }
        }
    }

    T2 * gather_values = new T2 [cluster_size];

    // MPI_Allreduce(&my_value, &global_value, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    MPI_Gather(&my_value, sizeof(T2), MPI_BYTE,
               gather_values, sizeof(T2), MPI_BYTE, 0, MPI_COMM_WORLD);

    T2 max_value = default_v;
    
    if(my_rank==0){
        max_value = gather_values[0];
        for(int i = 0; i<cluster_size; i++){
            if(max_value<gather_values[i]){
                max_value = gather_values[i];
            }
        }
    }
    delete [] gather_values;
   

    return max_value;
}

// The result of cutting the input with the delimiter is returned as a string vector.
vector<string> tools_split(string input, char delimiter) {
    vector<string> answer;
    stringstream ss(input);
    string temp;
 
    while (getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }
 
    return answer;
}

// Merges all strings contained in a string vector into a single string. (from start to end index)
string tools_string_vec_aggregate(vector<string> input_vec, int start, int end) {
    int input_size = input_vec.size();
    string out_string = "";
    if(start<0 || start>=input_size){
        std::cout<<"error in start"<<endl;
        return "";
    }
    if(end<0 || end>=input_size || end<=start){
        std::cout<<"error in end"<<endl;
        return "";
    }
    for(int i = start; i<end; i++){
        out_string = out_string + input_vec[i];
    }

    return out_string;
}

#endif
