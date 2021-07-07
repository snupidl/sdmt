import pandas as pd
import numpy as np
import joblib
import xgboost as xgb
import sys, getopt

###########################################
# Information                             #
###########################################

# This file reads the files related to the algorithm feature 
# obtained by executing the code using JavaCC 
# and the graph data feature obtained from the distributed graph engine, 
# and organizes it into a dataframe. 
# Finally, the preprocessed dataframe is used to predict the best partitioning strategy using xgboost written in python code.


# Read the file at file_path. 
# This is the file created by the graph engine. 
# This function read this file and construct the dataframe.
def data_feature_file_read(file_path):
    data_features = ['vertex_all_num','edge_all_num', 
                     'out_degree_mean','out_degree_std', 'out_degree_skewness', 'out_degree_kurtosis',
                     'in_degree_mean','in_degree_std', 'in_degree_skewness', 'in_degree_kurtosis',
                     'direction']
    file_contents = []
    f = open(file_path, 'r')
    while True:
        line = f.readline()
        if not line: break
        each_words = line.split(",")
        for word in each_words:
            index_p = word.find("E")
            index_p2 = word.find("e")
            value_float = 0.0
            if(word.strip()=="directed"):
                value_float = 1.0
            elif(word.strip()=="undirected"):
                value_float = 0.0
            else:
                if(index_p!=-1):
                    value_split = word.split("E")
                    base = value_split[0]
                    tenth = value_split[1]
                    value_float = float(base) * pow(10, float(tenth))
                elif(index_p2!=-1):
                    value_split = word.split("e")
                    base = value_split[0]
                    tenth = value_split[1]
                    value_float = float(base) * pow(10, float(tenth))
                else:
                    value_float = float(word)
            file_contents.append(value_float)
    f.close()
    out_df = pd.DataFrame([file_contents], columns = data_features)
    return out_df

# Read the file at file_path. 
# This is the file created by the JavaCC code. 
# This function read this file and construct the dataframe.
def algo_feature_file_read(file_path):
    algo_data_cols = ["vertex_value_read", "vertex_value_changed", 
                      "edge_value_changed","edge_value_read",
                      "get_in_vertex_to", "get_out_vertex_from", "get_both_vertex_from", 
                      "out_edge_num", "in_edge_num", 
                      "all_vertex_num", "all_edge_num", 
                      "all_vertex_list", "all_edge_list", 
                      "edge_count","SetPop", "SetInsert", 
                      "SomeValueChanged", "WhileCheck", "Comparing", 
                      "Plus", "Minus", "Multiply", "Divide", "Apply"]
    algo_data_df = pd.DataFrame(columns=algo_data_cols)
    algo_data_df.loc[algo_data_df.shape[0]] = np.nan
    f = open(file_path, 'r')
    while True:
        line = f.readline()
        if not line: break
        line_split = line.split(",")
        key = line_split[0]
        value = line_split[1]
        
        index_p = value.find("E")
        index_p2 = value.find("e")
        value_float = 0.0
        if(index_p!=-1):
            value_split = value.split("E")
            base = value_split[0]
            tenth = value_split[1]
            value_float = float(base) * pow(10, float(tenth))
        elif(index_p2!=-1):
            value_split = value.split("e")
            base = value_split[0]
            tenth = value_split[1]
            value_float = float(base) * pow(10, float(tenth))
        else:
            value_float = float(value)
        algo_data_df.loc[algo_data_df.shape[0]-1, key] = value_float
    f.close()
    col_list = algo_data_df.columns
    for col in col_list:
        algo_data_df[col].fillna(0, inplace=True)
    return algo_data_df

# merge two dataframes
def merge_all_features(data_feature_df, algo_feature_df):
    return pd.concat([data_feature_df, algo_feature_df], axis=1)

# preprocessing the merged dataframe.
# log scaling, sign and value separation are used.
def do_preprocessing(merged_df, scaler_filename1):
    ps_num = 11
    can_be_minus_list = ['out_degree_skewness', 'out_degree_kurtosis', 'in_degree_skewness', 'in_degree_kurtosis']
    ps_strategies = ["partitioning_strategy_name_1DDst", "partitioning_strategy_name_1DSrc", "partitioning_strategy_name_2D", 
                     "partitioning_strategy_name_CanoRandom", "partitioning_strategy_name_Ginger", "partitioning_strategy_name_HDRF10",
                     "partitioning_strategy_name_HDRF100", "partitioning_strategy_name_HDRF20", "partitioning_strategy_name_HDRF50",
                     "partitioning_strategy_name_Hybrid", "partitioning_strategy_name_Random"]
    log_preprocessing_cols = ['vertex_all_num', 'edge_all_num', 
                              'out_degree_mean', 'out_degree_std', 'out_degree_skewness', 'out_degree_kurtosis', 
                              'in_degree_mean', 'in_degree_std', 'in_degree_skewness', 'in_degree_kurtosis',
                              
                              'vertex_value_read','vertex_value_changed', 
                              'edge_value_changed', 'edge_value_read',
                              'get_in_vertex_to', 'get_out_vertex_from', 'get_both_vertex_from',
                              'out_edge_num', 'in_edge_num', 'all_vertex_num', 'all_edge_num',
                              'all_vertex_list', 'all_edge_list', 
                              'edge_count', 'SetPop', 'SetInsert',
                              'SomeValueChanged', 'WhileCheck', 'Comparing', 
                              'Plus', 'Minus', 'Multiply', 'Divide', 'Apply']
    #scaler_filename1 = "/usr/local/sdmt/gpspe/models/gpspe_xgboost_scaler"
                      #"robust_scaler1_9_dataset_8_hpc"
    
    for item in can_be_minus_list:
        #print(item)
        out_col_name = item+"_sign"
        merged_df[out_col_name] = np.where(merged_df[item]>=0, 0.0, 1.0)
    for item in can_be_minus_list:
        #print(item)
        out_col_name = item+"_sign"
        merged_df[item] = np.where(merged_df[out_col_name]==0.0, merged_df[item], -merged_df[item])
        
    out_df = pd.concat([merged_df]*ps_num, ignore_index=True)
    
    for i in range(ps_num):
        base_list = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        base_list[i] = 1.0
        out_df[ps_strategies[i]] = base_list
    
    out_df[log_preprocessing_cols] = out_df[log_preprocessing_cols] + 1.0
    out_df[log_preprocessing_cols] = np.log10(out_df[log_preprocessing_cols])
    scaler = joblib.load(scaler_filename1) 
    #print(out_df.info())
    last_np = scaler.transform(out_df)
    return last_np

# 1) Load the trained xgboost model.
# 2) predict
# 3) print the result
def do_inference(preprocessed_np, model_path):
    ps_strategies = ["1D Destination Vertex Hashing", "1D Source Vertex Hashing", "2D Hashing", 
                     "Canonical Random Hahsing", "Ginger", "HDRF10",
                     "HDRF100", "HDRF20", "HDRF50",
                     "Hybrid", "Random Hashing"]
    # model_path    =  '/usr/local/sdmt/gpspe/models/gpspe_xgboost_model'
                # = '15_xgboost_model_01_dataset_8_size_9_hpc_saved'
    params = {'tree_method':'auto', 
              'predictor': 'auto'}
    model_xgb = xgb.XGBRegressor(**params)
    model_xgb.load_model(model_path)
    model_out = model_xgb.predict(preprocessed_np)
    x = np.array(model_out)
    temp = x.argsort()
    rank = 0
    for item in temp:
        rank += 1
        print("Rank", rank, ":", ps_strategies[item])


# main code
# 1) read two files (data feature file, algorithm feature file)
# 2) merge two dataframes
# 3) preprocessing
# 4) inference(+print)
if __name__ == "__main__":
    data_file   = sys.argv[1]
    algo_file   = sys.argv[2]
    scaler_path = sys.argv[3]
    model_path  = sys.argv[4]
    
    data_df = data_feature_file_read(data_file)
    algo_df = algo_feature_file_read(algo_file)
    merge_df = merge_all_features(data_df, algo_df)
    preprocessed_input_vector = do_preprocessing(merge_df, scaler_path)
    do_inference(preprocessed_input_vector, model_path)
