#ifndef SNN_CORE_H
#define SNN_CORE_H

#include <vector>
#include <string>


using namespace std;

struct Transaction {
    int id;
    vector<int> items; 
    string expected_group; 
};



// wyniki znajdowania kNN
struct KNNResult {
    vector<vector<int>> knn_list;
    vector<double> Eps;
    vector<double> maxEps;
    vector<int> jaccard_calls;
    double time_prep = 0.0;
    double time_knn = 0.0;
};



double jaccard_distance(const vector<int>& a, const vector<int>& b);
long long combinations_of_2(long long n);
double calculate_rand_index(const vector<string>& RId, const vector<string>& CId, long long& TP, long long& TN, long long& All_pairs);


KNNResult find_knn(const vector<Transaction>& data, int k, const vector<int>& R_point, bool use_ti);
vector<int> compute_snn_clusters(int n, const vector<vector<int>>& knn, int kt);

#endif