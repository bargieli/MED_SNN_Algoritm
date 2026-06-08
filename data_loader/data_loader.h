#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "../snn_core/snn_core.h"
#include <vector>
#include <string>
#include <chrono>


std::vector<int> generate_reference_point(const vector<Transaction>& data, const string& type);


std::vector<Transaction> load_sample_data();
std::vector<Transaction> load_voting_data(const std::string& filename);
std::vector<Transaction> load_zoo_data(const std::string& filename);
std::vector<Transaction> load_mushroom_data(const std::string& filename);






void save_all_results(
    const std::string& algo_choice,
    const std::string& dataset_name,
    const std::string& file_name_used,
    int n, int k, int kt, bool use_ti,
    const std::string& r_name_full,
    const std::vector<Transaction>& data,
    const KNNResult& knn_res,
    const std::vector<int>& etykiety,
    const std::vector<int>& R_point,
    double t_read, 
    double t_cluster,
    std::chrono::time_point<std::chrono::high_resolution_clock> t_global_start
);


#endif