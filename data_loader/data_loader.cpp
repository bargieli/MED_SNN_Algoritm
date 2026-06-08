#include "data_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>

using namespace std;




vector<int> generate_reference_point(const vector<Transaction>& data, const string& type) {
    vector<int> result;
    if (data.empty()) return result; 

    int n = data.size();
    map<int, int> freq_map;
    
    // liczymy liczbe wystapien 
    for(const auto& t : data) {
        for(int c : t.items) {
            freq_map[c]++;
        }
    }
    
    vector<int> all_p, freq_p, infreq_p;

    for(auto const& [c, count] : freq_map) {
        all_p.push_back(c);
        if (count >= n / 2.0) freq_p.push_back(c);
        if (count <= n / 2.0) infreq_p.push_back(c);
    }
    
    if (type == "FREQ") return freq_p;
    else if (type == "INFREQ") return infreq_p;
    else return all_p;
}



// ------------------------------------------------------------------------- ladowanie danych



std::vector<Transaction> load_sample_data() {
    return {
        {1, {'A', 'B', 'C'}, "G1"}, {2, {'A', 'B', 'C', 'D'}, "G1"},
        {3, {'A', 'B', 'C', 'E'}, "G1"}, {4, {'A', 'B', 'C', 'F'}, "G1"},
        {5, {'X', 'Y', 'Z'}, "G2"}, {6, {'W', 'X', 'Y', 'Z'}, "G2"},
        {7, {'V', 'X', 'Y', 'Z'}, "G2"}, {8, {'U', 'X', 'Y', 'Z'}, "G2"}
    };
}

vector<Transaction> load_voting_data(const string& filename) {
    vector<Transaction> data;
    ifstream file(filename);
    string line;
    int id_counter = 1;

    if (!file.is_open()) return data;

    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        Transaction t;
        t.id = id_counter++;

        getline(ss, token, ',');
        t.expected_group = token;

        int vote_index = 0;
        while (getline(ss, token, ',')) {
            if (!token.empty()) {
                char vote_char = token[0]; 
                int encoded_item;

                int base_offset = vote_index * 3;
                if (vote_char == 'y') encoded_item = 'A' + base_offset;
                else if (vote_char == 'n') encoded_item = 'A' + base_offset + 1;
                else encoded_item = 'A' + base_offset + 2;

                t.items.push_back(encoded_item);
            }
            vote_index++;
        }
        data.push_back(t);
    }
    file.close();
    return data;
}



vector<Transaction> load_zoo_data(const string& filename) {
    vector<Transaction> data;
    ifstream file(filename);
    string line;
    int id_counter = 1;
    
    int offsets[16] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 30, 32, 34};

    if (!file.is_open()) {
        cout << "BLAD: Nie udalo sie otworzyc pliku " << filename << endl;
        return data;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string token;
        Transaction t;
        t.id = id_counter++;

        // nazwa, ignorujemy
        getline(ss, token, ',');

        // 16 cech
        int attr_idx = 0;
        while (attr_idx < 16 && getline(ss, token, ',')) {
            int val = stoi(token);
            char encoded_item;

            if (attr_idx == 12) { // liczba nog
                int sub_offset = 0;
                if (val == 2) sub_offset = 1;
                else if (val == 4) sub_offset = 2;
                else if (val == 5) sub_offset = 3;
                else if (val == 6) sub_offset = 4;
                else if (val == 8) sub_offset = 5; // max to 8 nóg
                encoded_item = 'A' + offsets[attr_idx] + sub_offset;
            } else { 
                encoded_item = 'A' + offsets[attr_idx] + val;
            }
            t.items.push_back(encoded_item);
            attr_idx++;
        }

        // klasa (1-7)
        if (getline(ss, token, ',')) {
            t.expected_group = "Class_" + token;
        }

        data.push_back(t);
    }
    file.close();
    return data;
}



vector<Transaction> load_mushroom_data(const string& filename) {
    vector<Transaction> data;
    ifstream file(filename);
    string line;
    int id_counter = 1;

    if (!file.is_open()) {
        cout << "BLAD: Nie udalo sie otworzyc pliku " << filename << endl;
        return data;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string token;
        Transaction t;
        t.id = id_counter++;

        // e - jadalny, p - trujący
        getline(ss, token, ',');
        t.expected_group = (token == "e") ? "Edible" : "Poisonous";

        // 22 cechy
        int attr_idx = 0;
        while (getline(ss, token, ',')) {
            if (!token.empty() && token[0] != '?') { // ignorujemy brakujące dane '?'
                int encoded_item = (attr_idx * 100) + token[0];
                t.items.push_back(encoded_item);
            }
            attr_idx++;
        }
        data.push_back(t);
    }
    file.close();
    return data;
}



// ------------------------------------------------------------------------- obsluga plikow



void calc_group_stats(const map<string, int>& counts, int& min_s, int& max_s, double& avg_s, double& stddev_s) {
    min_s = 1e9; max_s = 0; double sum = 0;
    for(auto const& [k, v] : counts) {
        if(v < min_s) min_s = v;
        if(v > max_s) max_s = v;
        sum += v;
    }
    if (counts.empty()) { avg_s = 0; stddev_s = 0; min_s = 0; return; }
    avg_s = sum / counts.size();
    double var_sum = 0;
    for(auto const& [k, v] : counts) var_sum += (v - avg_s) * (v - avg_s);
    stddev_s = sqrt(var_sum / counts.size());
}



string vec_to_string(const vector<int>& v) {
    string res = "{";
    for(size_t i = 0; i < v.size(); i++) {
        if (v[i] >= 32 && v[i] <= 126) {
            res += (char)v[i];
        } else {
            res += to_string(v[i]);
        }
        
        if(i != v.size() - 1) res += ", ";
    }
    res += "}";
    return res;
}



void save_all_results(
    const string& algo_choice, const string& dataset_name, const string& file_name_used,
    int n, int k, int kt, bool use_ti, const string& r_name_full,
    const vector<Transaction>& data, const KNNResult& knn_res,
    const vector<int>& etykiety, const vector<int>& R_point,
    double t_read, double t_cluster, chrono::time_point<chrono::high_resolution_clock> t_global_start) 
{
    
    set<int> unique_items;
    int total_items = 0;

    for(const auto& t : data) {
        total_items += t.items.size();
        for(int c : t.items) unique_items.insert(c);
    }
    double avg_items = (double)total_items / n;

    string base_name = (use_ti ? "TI_1-J_SNN_" : "1-J_SNN_") + dataset_name + "_R" + to_string(n) + "_k" + to_string(k) + "_kt" + to_string(kt);
    if(use_ti) base_name += "_" + r_name_full;

    system("mkdir -p ../outputs");
    string out_dir = "../outputs/";

    auto t_save_start = chrono::high_resolution_clock::now();
    


    // OUT
    ofstream f_out(out_dir + "OUT_" + base_name + ".csv");
    vector<string> RId_list, CId_list;
    map<string, int> R_counts, C_counts;

    for (int i = 0; i < n; i++) {
        string rid = data[i].expected_group.empty() ? "-" : data[i].expected_group;
        string cid = "T" + to_string(etykiety[i] + 1);
        
        RId_list.push_back(rid); CId_list.push_back(cid);
        R_counts[rid]++; C_counts[cid]++;
        
        f_out << data[i].id << ",\"" << vec_to_string(data[i].items) << "\"," << rid << "," << cid << "\n";
    }
    f_out.close();


    // rand
    auto t_rand_start = chrono::high_resolution_clock::now();
    long long TP, TN, All_pairs;
    double rand_val = calculate_rand_index(RId_list, CId_list, TP, TN, All_pairs);
    auto t_rand_end = chrono::high_resolution_clock::now();
    double t_rand = chrono::duration<double>(t_rand_end - t_rand_start).count();



    // KNN
    ofstream f_knn(out_dir + "KNN_" + base_name + ".csv");
    for (int i = 0; i < n; i++) {
        f_knn << data[i].id << "," << knn_res.Eps[i] << "," << knn_res.maxEps[i] << "," << knn_res.jaccard_calls[i];
        for(int neighbour_idx : knn_res.knn_list[i]) f_knn << "," << data[neighbour_idx].id;
        f_knn << "\n";
    }
    f_knn.close();
    
    auto t_save_end = chrono::high_resolution_clock::now();
    double t_save = chrono::duration<double>(t_save_end - t_save_start).count();

    auto t_global_end = chrono::high_resolution_clock::now();
    double t_total = chrono::duration<double>(t_global_end - t_global_start).count();



    // STAT
    int r_min, r_max, c_min, c_max;
    double r_avg, r_std, c_avg, c_std;
    calc_group_stats(R_counts, r_min, r_max, r_avg, r_std);
    calc_group_stats(C_counts, c_min, c_max, c_avg, c_std);

    long long total_jaccard_calls = 0;
    for (int calls : knn_res.jaccard_calls) {
        total_jaccard_calls += calls;
    }

    ofstream f_stat(out_dir + "STAT_" + base_name + ".csv");
    f_stat << "Nazwa pliku: " << file_name_used << "\n";
    f_stat << "Liczba unikatowych pozycji: " << unique_items.size() << "\n";
    f_stat << "Srednia liczba pozycji w rekordzie: " << avg_items << "\n";
    f_stat << "Liczba rekordow: " << n << "\n";


    f_stat << "k: " << k << ", kt: " << kt << "\n";
    if (use_ti && !R_point.empty()) {
        f_stat << "Rekord referencyjny: " << vec_to_string(R_point) << "\n";
    }

    f_stat << "Calkowita liczba wyliczen 1-J: " << total_jaccard_calls << "\n";

    f_stat << "\n--- CZASY WYKONANIA [s] ---\n";
    f_stat << "Odczyt: " << t_read << "\n";
    f_stat << "Prep (odleglosci od Ref): " << knn_res.time_prep << "\n";
    f_stat << "Obliczenie kNN: " << knn_res.time_knn << "\n";
    f_stat << "Grupowanie SNN: " << t_cluster << "\n";
    f_stat << "Obliczenie RAND: " << t_rand << "\n";
    f_stat << "Zapis do plikow: " << t_save << "\n";
    f_stat << "CALKOWITY CZAS: " << t_total << "\n\n";
    f_stat << "Rzeczywiste grupy - liczba: " << R_counts.size() << " (Min: " << r_min << ", Max: " << r_max << ", Avg: " << r_avg << ", StdDev: " << r_std << ")\n";
    f_stat << "Wykryte grupy     - liczba: " << C_counts.size() << " (Min: " << c_min << ", Max: " << c_max << ", Avg: " << c_avg << ", StdDev: " << c_std << ")\n";
    f_stat << "|TP|: " << TP << "\n|TN|: " << TN << "\nLiczba par rekordow: " << All_pairs << "\nWartosc indeksu RAND: " << rand_val << "\n";
    f_stat.close();

    cout << "Zakonczono! Pliki zostaly wygenerowane w folderze '../outputs' dla algorytmu " << algo_choice << " (" << base_name << ").\n";
}




