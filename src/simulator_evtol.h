#ifndef SIMULATOR_EVTOL_H
#define SIMULATOR_EVTOL_H

#include <vector>


struct simulator_evtol {

    int N_aircraft_count=20;
    int K_aircraft_types=5;
    int nchoose(int n, int k);
    const int max_rank = nchoose(N_aircraft_count + K_aircraft_types - 1, K_aircraft_types - 1);
    int composition_to_rank(const std::vector<int>& xs);
    std::vector<int> rank_to_composition(int input_rank);
};

#endif // SIMULATOR_EVTOL_H
