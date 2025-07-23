#include <cassert>
#include <iostream>
#include <random>
#include <vector>
#include "simulator_evtol.h"

simulator_evtol sim;
void test_nchoose() {
    assert(sim.nchoose(1,1) == 1);
    assert(sim.nchoose(2,1) == 2);
    assert(sim.nchoose(3,1) == 3);
    assert(sim.nchoose(3,2) == 3);
    assert(sim.nchoose(4,2) == 6);
}

void test_composition_to_rank() {
    assert(sim.composition_to_rank(sim.rank_to_composition(1)) == 1);
    assert(sim.composition_to_rank(sim.rank_to_composition(sim.max_rank)) == sim.max_rank);
    try {
        assert(sim.composition_to_rank(sim.rank_to_composition(sim.max_rank + 1)) != sim.max_rank + 1);  // throws
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    assert(sim.rank_to_composition(1) == std::vector<int>({0,0,0,0,sim.N_aircraft_count}));
    assert(sim.rank_to_composition(1) == std::vector<int>({0,0,0,0,sim.N_aircraft_count}));
    assert(sim.rank_to_composition(sim.max_rank) == std::vector<int>({sim.N_aircraft_count,0,0,0,0}));
}

int main() {
    test_nchoose();
    test_composition_to_rank();
    
    using namespace std;
    // for (auto i : sim.rank_to_composition(1))
    //     cout << i << " ";
    // cout << endl;

    std::random_device rd;
    auto seed = rd();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,sim.max_rank);

    std::cout << "seed "<< seed << " dist(1," << sim.max_rank << ") = " << dist(rng) << std::endl;

    return 0;
}
