#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "simulator_evtol.h"


int simulator_evtol::nchoose(int n, int k) {
    if (k == 0) return 1;
    return (n * simulator_evtol::nchoose(n - 1, k - 1)) / k;
}

int simulator_evtol::composition_to_rank(const std::vector<int>& xs) {
    int N = this->N_aircraft_count, K = this->K_aircraft_types;

    for (auto i : xs)
        if (i < 0 || i > N)
            throw std::invalid_argument("Bad vector composition value " + std::to_string(i)); 

    int rank = 1, remaining = N;
    for (int i = 0; i < K - 1; ++i) {
        for (int a = 0; a < xs[i]; ++a) {
            rank += simulator_evtol::nchoose(remaining - a + K - i - 2, K - i - 2);
        }
        remaining -= xs[i];
    }
    return rank;
}

std::vector<int> simulator_evtol::rank_to_composition(int input_rank) {
    int N = this->N_aircraft_count, K = this->K_aircraft_types, rank = input_rank - 1, remaining = N;
    if (input_rank < 1 || input_rank > this->max_rank)
        throw std::invalid_argument("Bad rank value " + std::to_string(input_rank)); 

    std::vector<int> xs(K, 0);
    for (int i = 0; i < K - 1; ++i) {
        xs[i] = 0;
        while (true) {
            int count = simulator_evtol::nchoose(remaining - xs[i] + K - i - 2, K - i - 2);
            if (count > rank) break;
            rank -= count;
            xs[i]++;
        }
        remaining -= xs[i];
    }
    xs[K - 1] = remaining;
    return xs;
}

