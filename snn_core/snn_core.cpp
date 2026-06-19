#include "snn_core.h"
#include <map>
#include <cmath>
#include <chrono>
#include <algorithm>

double jaccard_distance(const vector<int>& a, const vector<int>& b) {
    int i = 0, j = 0, wspolne = 0;
    
    int len_a = a.size();
    int len_b = b.size();
    
    while (i < len_a && j < len_b) {
        if (a[i] == b[j]) { 
            wspolne++; 
            i++; 
            j++; 
        }
        else if (a[i] < b[j]) {
            i++; 
        }
        else {
            j++; 
        }
    }
    
    int wszystkie_unikalne = len_a + len_b - wspolne;
    
    if (wszystkie_unikalne <= 0) return 0.0;
    
    return 1.0 - ((double)wspolne / wszystkie_unikalne);
}



long long combinations_of_2(long long n) {
    if (n < 2) return 0;
    return n * (n - 1) / 2;
}



double calculate_rand_index(const vector<string>& RId, const vector<string>& CId, long long& out_TP, long long& out_TN, long long& out_Pairs) {
    if (RId.size() != CId.size() || RId.empty()) return 0.0;
    
    int N = RId.size();
    
    map<string, map<string, int>> confusion_matrix;
    map<string, int> row_sums, col_sums;

    for (int i = 0; i < N; i++) {
        confusion_matrix[CId[i]][RId[i]]++;
        row_sums[CId[i]]++;
        col_sums[RId[i]]++;
    }

    long long TP = 0, sum_TP_FP = 0, sum_TP_FN = 0;
    for (const auto& row : confusion_matrix) for (const auto& cell : row.second) TP += combinations_of_2(cell.second);
    for (const auto& row : row_sums) sum_TP_FP += combinations_of_2(row.second);
    for (const auto& col : col_sums) sum_TP_FN += combinations_of_2(col.second);

    long long All_pairs = combinations_of_2(N);
    long long TP_plus_TN = All_pairs - sum_TP_FP - sum_TP_FN + 2 * TP;

    out_Pairs = All_pairs;
    out_TP = TP;
    out_TN = TP_plus_TN - TP;
    return (double)TP_plus_TN / All_pairs;
}



KNNResult find_knn(const vector<Transaction>& data, int k, const vector<int>& R_point, bool use_ti) {
    int n = data.size();
    KNNResult result;
    result.knn_list.resize(n);
    result.Eps.assign(n, 0.0);
    result.maxEps.assign(n, 0.0);
    result.jaccard_calls.assign(n, 0);

    auto t_prep_start = chrono::high_resolution_clock::now();
    
    vector<double> dist_to_R(n, 0.0);
    vector<int> sorted_indices(n);
    for (int i = 0; i < n; i++) sorted_indices[i] = i;
    

    if (use_ti) {
        for (int i = 0; i < n; i++) {
            dist_to_R[i] = jaccard_distance(data[i].items, R_point);
        }

        // dodane
        sort(sorted_indices.begin(), sorted_indices.end(), [&](int a, int b) {
            return dist_to_R[a] < dist_to_R[b];
        });
    }

    auto t_prep_end = chrono::high_resolution_clock::now();
    result.time_prep = chrono::duration<double>(t_prep_end - t_prep_start).count();
    auto t_knn_start = chrono::high_resolution_clock::now();
    
    
    for (int i = 0; i < n; i++) {
        vector<int> aktualni_sasiedzi;
        vector<double> dystanse_sasiadow;

        bool maxEps_set = false;
        double local_max_naive = 0.0;
        
        for (int idx = 0; idx < n; idx++) {
            int j = sorted_indices[idx];

            if (i == j) continue;
            
            double eps = 1.0; 
            
            if (aktualni_sasiedzi.size() == k) {
                eps = 0.0;
                for (double d : dystanse_sasiadow) if (d > eps) eps = d;
                
                if (use_ti && !maxEps_set) {
                    result.maxEps[i] = eps;
                    maxEps_set = true;
                }
                
                if (use_ti) {
                    double diff = dist_to_R[j] - dist_to_R[i];

                    if (diff > eps) {
                        break; 
                    }
                    if (-diff > eps) { 
                        continue; 
                    }
                }
            }
            
            // sprawdzamy odl jaccarda

            result.jaccard_calls[i]++;
            double dist = jaccard_distance(data[i].items, data[j].items);
            if (!use_ti && dist > local_max_naive) local_max_naive = dist;
            
            if (aktualni_sasiedzi.size() < k) {
                aktualni_sasiedzi.push_back(j);
                dystanse_sasiadow.push_back(dist);
            } else {

                // znalezlismy blizszego sasiada niz wczesniejsi

                int id_najgorszego = 0;
                double max_d = dystanse_sasiadow[0];
                for (int m = 1; m < k; m++) {
                    if (dystanse_sasiadow[m] > max_d) { max_d = dystanse_sasiadow[m]; id_najgorszego = m; }
                }
                if (dist < max_d) {
                    aktualni_sasiedzi[id_najgorszego] = j;
                    dystanse_sasiadow[id_najgorszego] = dist;
                }
            }
        }
        
        double final_eps = 0.0;
        for (double d : dystanse_sasiadow) if (d > final_eps) final_eps = d;
        result.Eps[i] = final_eps;
        
        if (!use_ti) result.maxEps[i] = local_max_naive;
        result.knn_list[i] = aktualni_sasiedzi; 
    }
    auto t_knn_end = chrono::high_resolution_clock::now();
    result.time_knn = chrono::duration<double>(t_knn_end - t_knn_start).count();
    
    return result;
}



vector<int> compute_snn_clusters(int n, const vector<vector<int>>& knn, int kt) {
    vector<int> etykiety(n);
    for (int i = 0; i < n; i++) etykiety[i] = i;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            
            bool i_ma_j = false, j_ma_i = false;

            for (int sasiad : knn[i]) if (sasiad == j) i_ma_j = true;
            for (int sasiad : knn[j]) if (sasiad == i) j_ma_i = true;

            if (i_ma_j && j_ma_i) {

                int shared_count = 0;
                for (int si : knn[i]) for (int sj : knn[j]) if (si == sj && si != i && sj != j) shared_count++;
    
                if (shared_count >= kt) {
                    int mniejsza = min(etykiety[i], etykiety[j]);
                    int wieksza = max(etykiety[i], etykiety[j]);
                    if (mniejsza != wieksza) {
                        for (int x = 0; x < n; x++) if (etykiety[x] == wieksza) etykiety[x] = mniejsza;
                    }
                }
            }
        }
    }
    return etykiety;
}