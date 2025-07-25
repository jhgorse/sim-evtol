//=============================================================================
// main.cpp
//-----------------------------------------------------------------------------
// eVTOL Simulation Entry Point
//
// This file contains the main function for running the eVTOL simulation.
// It sets up the simulation configuration, initializes the SimEVTOL object,
// and executes the simulation run, handling any exceptions that may occur.
//
// Usage:
//   ./evtol-simulation [seed]
//
// - If a seed is provided as a command-line argument, it is used for simulation.
// - Otherwise, a default seed is used.
//
// Author: [Your Name or Organization]
// License: See LICENSE file for

#include <iostream>
#include "sim_evtol.h"

int main(int argc, char** argv) {
    std::cout << "=== eVTOL Simulation JHG ===\n";
    
    try {
        SimEVTOL::Config config;
        config.nAircraft = 20;
        config.nChargers = 3;
        config.simDuration = 3.0; // hours
        config.aircraftTypesFile = "aircraftTypes.csv";  // TODO
        if (argc == 2) {
            config.seed = atoi(argv[1]);  // TODO add method to calculate max in Config
            SimEVTOL simulation(config);
            simulation.Run();
        }
        else {
            for (int i=0; i<10626; i++) {
                config.seed++;
                SimEVTOL simulation(config);
                simulation.Run();
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
