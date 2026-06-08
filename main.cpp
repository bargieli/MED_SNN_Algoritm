#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "snn_core/snn_core.h"
#include "data_loader/data_loader.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 5) {
        cout << "Użycie: " << argv[0] << " <zbior_danych> <k> <kt> <algorytm> [punkt_referencyjny]\n";
        cout << "Parametry:\n";
        cout << "  <zbior_danych> : 'Sample', 'Voting', 'Zoo', 'Mushroom'\n";
        cout << "  <k>            : liczba sasiadow (calkowita, np. 15)\n";
        cout << "  <kt>           : prog SNN (calkowita, np. 7, przy czym kt < k)\n";
        cout << "  <algorytm>     : 'naive' lub 'ti'\n";
        cout << "  [punkt_ref]    : 'freq', 'infreq', 'all' (opcjonalne, tylko dla 'ti', jeden parametr!)\n";
        cout << "Przyklad wywolania:\n";
        cout << "  " << argv[0] << " Voting 15 7 ti freq\n";
        return 1;
    }

    string dataset_choice = argv[1];
    int k = stoi(argv[2]);
    int kt = stoi(argv[3]);
    string algo_choice = argv[4];

    bool use_ti = (algo_choice == "ti");
    string ref_type = ""; 
    
    if (use_ti) {
        if (argc >= 6) {
            ref_type = argv[5];
            for (auto & c: ref_type) c = toupper(c); 
        } else {
            cout << "Ostrzezenie: Nie podano punktu referencyjnego dla TI. Uzywam domyslnie 'FREQ'.\n";
            ref_type = "FREQ";
        }
    }



// -----------------------------------------------------------------------------------------------



    auto t_global_start = chrono::high_resolution_clock::now();
    double t_read = 0, t_cluster = 0;

    vector<Transaction> data;
    vector<int> R_point; 
    string dataset_name, r_name_full = "";
    string file_name_used;

    auto t_read_start = chrono::high_resolution_clock::now();

    dataset_name = dataset_choice; 


    if (dataset_choice == "Sample") {
        file_name_used = "SampleData";
        data = load_sample_data();
    } 
    else if (dataset_choice == "Voting") {
        file_name_used = "../data/house-votes-84.data";
        data = load_voting_data(file_name_used);
    } 
    else if (dataset_choice == "Zoo") {
        file_name_used = "../data/zoo.data";
        data = load_zoo_data(file_name_used);
    } 
    else if (dataset_choice == "Mushroom") {
        file_name_used = "../data/agaricus-lepiota.data";
        data = load_mushroom_data(file_name_used);
    } 
    else {
        cout << "Nieznany zbior danych: " << dataset_choice << "\n";
        return 1;
    }

  
    if (use_ti) {
        R_point = generate_reference_point(data, ref_type); 
    }

    if (data.empty()) {
        cout << "Blad! Zbior danych jest pusty.\n";
        return 1;
    }
  
    

    auto t_read_end = chrono::high_resolution_clock::now();
    t_read = chrono::duration<double>(t_read_end - t_read_start).count();


    int n = data.size();

    if (use_ti) {
        string r_lower = ref_type;
        for(auto& c : r_lower) c = tolower(c);
        r_name_full = "{" + r_lower + "}"; 
    }



// -----------------------------------------------------------------------------------------------



    KNNResult knn_res = find_knn(data, k, R_point, use_ti); 
    
    auto t_cluster_start = chrono::high_resolution_clock::now();
    vector<int> etykiety = compute_snn_clusters(n, knn_res.knn_list, kt);
    auto t_cluster_end = chrono::high_resolution_clock::now();

    t_cluster = chrono::duration<double>(t_cluster_end - t_cluster_start).count();

    save_all_results(algo_choice, dataset_name, file_name_used, n, k, kt, use_ti, 
                     r_name_full, data, knn_res, etykiety, R_point, 
                     t_read, t_cluster, t_global_start);

    return 0;
}