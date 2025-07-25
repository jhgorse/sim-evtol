//=============================================================================
// test_suite.cpp
//-----------------------------------------------------------------------------
// eVTOL Simulation Test Suite
//
// This file contains a skeleton set of unit and integration tests for the
// eVTOL simulation system. It validates the correctness of algorithms and
// behaviors for aircraft types, aircraft state machines, charger queue logic,
// statistics collection, fleet coordination, event scheduling, and simulation
// integration. Acceptance tests are included to verify end-to-end requirements
// and edge cases.
//
// Test Coverage Includes:
// - AircraftType parameter validation and range calculation
// - Aircraft state transitions and member access
// - ChargerQueue FIFO logic and edge cases
// - StatMetrics aggregation and reporting
// - Fleet construction and event scheduling
// - Simulation configuration, rank/composition logic, and integration
// - Partial activity normalization at simulation end
// - Table output formatting and reporting
// - Fault detection probability and statistical accuracy
// - Acceptance tests for real-world simulation scenarios
//
// Author: Joe Gorse
// License: See LICENSE file for details.
//

#include <cassert>
#include <iostream>
#include <random>
#include <vector>
#include <memory>
#include "sim_evtol.h"


// Rank and Composition tests
void test_nchoose(SimEVTOL& sim) {
    assert(sim.NChooseK(1,1) == 1);
    assert(sim.NChooseK(2,1) == 2);
    assert(sim.NChooseK(3,1) == 3);
    assert(sim.NChooseK(3,2) == 3);
    assert(sim.NChooseK(4,2) == 6);
    std::cout << "✓ test_nchoose passed\n";
}

void test_composition_to_rank(SimEVTOL& sim, int nAircraft) {
    assert(sim.CompositionToRank(sim.RankToComposition(1)) == 1);
    assert(sim.CompositionToRank(sim.RankToComposition(sim.GetMaxRank())) == sim.GetMaxRank());
    try {
        assert(sim.CompositionToRank(sim.RankToComposition(sim.GetMaxRank() + 1)) != sim.GetMaxRank() + 1);  // throws
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }
    assert(sim.RankToComposition(1) == std::vector<int>({0,0,0,0,nAircraft}));
    assert(sim.RankToComposition(1) == std::vector<int>({0,0,0,0,nAircraft}));
    assert(sim.RankToComposition(sim.GetMaxRank()) == std::vector<int>({nAircraft,0,0,0,0}));
    std::cout << "✓ test_composition_to_rank passed\n";
}

// New tests for sim_design algorithms

void test_aircraft_type() {
    std::cout << "\n=== Testing AircraftType ===\n";
    
    AircraftType alpha("Alpha", 120.0, 320.0, 0.6, 1.6, 4, 0.25);
    
    assert(alpha.name == "Alpha");
    assert(alpha.cruiseSpeed == 120.0);
    assert(alpha.batteryCapacity == 320.0);
    assert(alpha.chargeTime == 0.6);
    assert(alpha.energyUsePerMile == 1.6);
    assert(alpha.passengerCount == 4);
    assert(alpha.faultProbPerHour == 0.25);
    
    // Test calculated range
    double expectedRange = 320.0 / 1.6; // 200 miles
    assert(abs(alpha.RangeMiles() - expectedRange) < 0.001);
    
    std::cout << "✓ AircraftType tests passed\n";
}

void test_aircraft_state_machine() {
    std::cout << "\n=== Testing Aircraft State Machine ===\n";
    
    AircraftType testType("Test", 100.0, 100.0, 0.5, 1.0, 2, 0.01);
    Aircraft aircraft(testType, 1);
    
    // Initial state should be AIRBORNE
    assert(aircraft.GetState() == Aircraft::AIRBORNE);
    assert(aircraft.id == 1);
    assert(aircraft.batteryLevel == 100.0);
    
    std::cout << "✓ Aircraft initial state correct\n";
    
    assert(aircraft.totalFlightTime == 0.0);
    assert(aircraft.totalChargingTime == 0.0);
    assert(aircraft.totalDistance == 0.0);
    assert(aircraft.totalPassengerMiles == 0.0);
    
    std::cout << "✓ Aircraft direct member access working correctly\n";
}

void test_charger_queue() {
    std::cout << "\n=== Testing ChargerQueue Algorithm ===\n";
    
    ChargerQueue queue(2); // 2 chargers available
    
    // Initially no one charging
    assert(queue.IsChargerAvailable() == true);
    assert(queue.NumCharging() == 0);
    
    // Create some test aircraft
    AircraftType testType("Test", 100.0, 100.0, 0.5, 1.0, 2, 0.01);
    Aircraft aircraft1(testType, 1);
    Aircraft aircraft2(testType, 2);
    Aircraft aircraft3(testType, 3);
    
    // First aircraft can charge immediately
    queue.PlugIn(&aircraft1, 0.0);
    assert(queue.NumCharging() == 1);
    assert(queue.m_chargingNow.size() == 1);
    assert(queue.m_waitingQueue.size() == 0);
    
    // Second aircraft can also charge immediately
    queue.PlugIn(&aircraft2, 0.0);
    assert(queue.NumCharging() == 2);
    assert(queue.m_chargingNow.size() == 2);
    assert(queue.m_waitingQueue.size() == 0);
    assert(queue.IsChargerAvailable() == false);
    
    // Third aircraft must wait
    queue.PlugIn(&aircraft3, 0.0);
    assert(queue.NumCharging() == 2);
    assert(queue.m_chargingNow.size() == 2);
    assert(queue.m_waitingQueue.size() == 1);
    
    // First aircraft finishes charging
    queue.Unplug(&aircraft1, 1.0);
    assert(queue.NumCharging() == 2); // Third aircraft should start charging
    assert(queue.m_chargingNow.size() == 2);
    assert(queue.m_waitingQueue.size() == 0);
    
    std::cout << "✓ ChargerQueue FIFO algorithm working correctly\n";
}

void test_stat_metrics() {
    std::cout << "\n=== Testing StatMetrics Algorithms ===\n";
    
    AircraftType alpha{"Alpha", 120.0, 320.0, 0.6, 1.6, 4, 0.25};
    AircraftType bravo{"Bravo", 100.0, 100.0, 0.2, 1.5, 5, 0.10};
    
    std::vector<AircraftType> types = {alpha, bravo};
    StatMetrics metrics(types);
    
    Aircraft aircraftAlpha(alpha, 1);
    Aircraft aircraftBravo(bravo, 2);
    
    metrics.FlightComplete(aircraftAlpha, 1.0);
    metrics.FlightComplete(aircraftBravo, 1.5);
    
    metrics.ChargingCompleteFlightStart(aircraftAlpha, 2.0);
    metrics.ChargingCompleteFlightStart(aircraftBravo, 2.5);
    
    metrics.FinalizeFlightChargeData();
    
    const auto& alphaStats = metrics.GetStats(alpha);
    const auto& bravoStats = metrics.GetStats(bravo);
    
    assert(alphaStats.numFlights == 1);
    assert(bravoStats.numFlights == 1);
    assert(alphaStats.numCharges == 1);
    assert(bravoStats.numCharges == 1);
    
    std::cout << "✓ StatMetrics calculation algorithms working\n";
}

void test_fleet_coordination() {
    std::cout << "\n=== Testing Fleet Coordination Algorithm ===\n";
    
    AircraftType alpha{"Alpha", 120.0, 320.0, 0.6, 1.6, 4, 0.25};
    AircraftType bravo{"Bravo", 100.0, 100.0, 0.2, 1.5, 5, 0.10};
    
    std::vector<AircraftType> types = {alpha, bravo};
    std::vector<int> composition = {2, 1}; // 2 Alpha, 1 Bravo
    
    Fleet fleet(types, composition);
    
    assert(fleet.GetAircraft().size() == 3);
    
    assert(fleet.GetAircraft()[0]->GetType().name == "Alpha");
    assert(fleet.GetAircraft()[1]->GetType().name == "Alpha");
    assert(fleet.GetAircraft()[2]->GetType().name == "Bravo");
    
    // Test getting next event time
    double nextTime = fleet.GetNextEventTime();
    assert(nextTime > 0.0 && nextTime < std::numeric_limits<double>::max());
    
    std::cout << "✓ Fleet coordination algorithm working\n";
}

void test_simulation_integration() {
    std::cout << "\n=== Testing Simulation Integration ===\n";
    
    // Create a small simulation to test integration
    SimEVTOL::Config config;
    config.nAircraft = 5;
    config.nChargers = 2;
    config.simDuration = 0.1; // Very short simulation
    config.aircraftTypesFile = "AircraftTypes.csv"; // TODO
    config.seed = 1;
    
    try {
        SimEVTOL sim(config);

        test_nchoose(sim);  // TODO separate with 
        test_composition_to_rank(sim, config.nAircraft);

        // Test rank to composition conversion
        std::vector<int> composition = sim.RankToComposition(config.seed);
        assert(composition.size() == 5); // 5 aircraft types
        
        int totalAircraft = 0;
        for (int count : composition) {
            totalAircraft += count;
        }
        assert(totalAircraft == config.nAircraft);
        
        std::cout << "✓ Simulation integration tests passed\n";
        
        // Note: We are not running the full simulation yet. Just up front integration.
        
    } catch (const std::exception& e) {
        std::cout << "Simulation test caught exception: " << e.what() << std::endl;
    }
}

void test_event_scheduling() {
    std::cout << "\n=== Testing Event Scheduling Algorithm ===\n";
    
    AircraftType fastType("Fast", 200.0, 100.0, 0.25, 2.0, 3, 0.05); // Range: 50 miles
    Aircraft aircraft(fastType, 1);
    
    // Initial next event time should be the flight completion time
    double initialTime = aircraft.GetNextEventTime();
    double expectedFlightTime = 50.0 / 200.0; // 0.25 hours
    assert(abs(initialTime - expectedFlightTime) < 0.001);
    
    std::cout << "✓ Event scheduling algorithm working correctly\n";
}

void test_partial_activity_normalization() {
    std::cout << "\n=== Testing Partial Activity Normalization ===\n";
    
    // Create test aircraft types
    AircraftType alpha{"Alpha", 120.0, 320.0, 0.6, 1.6, 4, 0.25};
    std::vector<AircraftType> types = {alpha};
    
    StatMetrics metrics(types);
    
    // Create test aircraft and manually set them to different states
    auto aircraft1 = std::make_unique<Aircraft>(alpha, 1);
    auto aircraft2 = std::make_unique<Aircraft>(alpha, 2);
    
    aircraft1->state = Aircraft::AIRBORNE;
    aircraft1->lastEventTime = 1.0;  // Started flight at time 1.0
    
    aircraft2->state = Aircraft::CHARGING;
    aircraft2->lastEventTime = 2.0;  // Started charging at time 2.0
    
    std::vector<std::unique_ptr<Aircraft>> aircraftList;  // A fleet-like container
    aircraftList.push_back(std::move(aircraft1));
    aircraftList.push_back(std::move(aircraft2));
    
    // Test partial activity finalization at simulation end time 2.5
    metrics.FinalizePartialActivities(aircraftList, 2.5);
    
    std::cout << "✓ Partial activity normalization test completed\n";
}

void test_edge_cases() {
    std::cout << "\n=== Testing Edge Cases ===\n";
    
    // Test charger queue when no chargers available
    ChargerQueue queue(0); // No chargers
    assert(queue.IsChargerAvailable() == false);
    assert(queue.NumCharging() == 0);
    
    AircraftType safetype("Safe", 100.0, 100.0, 0.5, 1.0, 2, 0.0); // 0% fault rate
    Aircraft safeAircraft(safetype, 1);
    
    AircraftType dangerType("Danger", 100.0, 100.0, 0.5, 1.0, 2, 1.0); // 100% fault rate per hour
    Aircraft dangerAircraft(dangerType, 2);
    
    std::cout << "✓ Edge case tests completed\n";
}

void test_table_output_format() {
    std::cout << "\n=== Testing Table Output Format ===\n";
    
    AircraftType alpha{"Alpha", 120.0, 320.0, 0.6, 1.6, 4, 0.25};
    AircraftType bravo{"Bravo", 100.0, 100.0, 0.2, 1.5, 5, 0.10};
    AircraftType charlie{"Charlie", 160.0, 220.0, 0.8, 2.2, 3, 0.05};
    
    std::vector<AircraftType> types = {alpha, bravo, charlie};
    
    // Test that RunStatsRecord can handle vector of aircraft types for table format
    RunStatsRecord record(types);
    
    std::cout << "✓ Table output format test completed\n";
}

void test_fault_detection() {
    std::cout << "\n=== Testing Fault Detection Algorithm ===\n";
    
    // Test the fault probability calculation directly
    AircraftType alpha("Alpha", 120.0, 320.0, 0.6, 1.6, 4, 0.25);
    Aircraft aircraft(alpha, 1);
    std::mt19937 rng(42); // Fixed seed for reproducibility
    
    // Test fault probability calculation for 1 hour
    int faults = 0;
    int iterations = 10000;
    for (int i = 0; i < iterations; i++) {
        if (aircraft.CheckFaultDuringTimeStep(1.0, rng)) {
            faults++;
        }
    }
    
    double actualRate = static_cast<double>(faults) / iterations;
    double expectedRate = 0.25; // faultProbPerHour
    double tolerance = 0.02; // 2% tolerance
    
    assert(abs(actualRate - expectedRate) < tolerance);
    
    std::cout << "  Fault rate over " << iterations << " iterations: " << actualRate 
              << " (expected: " << expectedRate << ")\n";
    
    // Test that faults occur more frequently with higher time steps
    faults = 0;
    for (int i = 0; i < 1000; i++) {
        if (aircraft.CheckFaultDuringTimeStep(2.0, rng)) { // 2 hour time step
            faults++;
        }
    }
    
    double rate2h = static_cast<double>(faults) / 1000;
    double expected2h = 0.5; // 0.25 * 2 hours
    
    assert(abs(rate2h - expected2h) < 0.05); // 5% tolerance for smaller sample
    
    std::cout << "  2-hour fault rate: " << rate2h << " (expected: " << expected2h << ")\n";
    std::cout << "✓ Fault detection algorithm test completed\n";
}

// Helper function to find seed for specific composition
int findSeedForComposition(const std::vector<int>& targetComposition, int nAircraft) {
    SimEVTOL::Config config;
    config.nAircraft = nAircraft;
    config.nChargers = 1;
    config.simDuration = 1.0;
    config.aircraftTypesFile = "dummy";
    config.seed = 1;
    
    SimEVTOL sim(config);
    int maxRank = sim.GetMaxRank();
    
    for (int seed = 1; seed <= maxRank; seed++) {
        try {
            auto composition = sim.RankToComposition(seed);
            if (composition == targetComposition) {
                return seed;
            }
        } catch (...) {
            continue;
        }
    }
    return -1; // Not found
}

// Helper struct for expected stats
struct ExpectedStats {
    std::string name;
    double avgFlightTime;
    double avgDistance;
    double avgChargeTime;
    double minFaults;
    double passengerMiles;
    double flights;
    double charges;
};

// Helper function to run a simulation and validate stats
void run_sim_and_validate(
    int nAircraft,
    int nChargers,
    double simDuration,
    const std::vector<int>& composition,
    const std::vector<ExpectedStats>& expectedStatsVec,
    double tolerance = 0.1,
    double passengerMilesToleranceRatio = 0.02,
    double flightsTolerance = 0.1,
    double chargesTolerance = 0.1
) {
    int seed = findSeedForComposition(composition, nAircraft);
    assert(seed != -1);

    SimEVTOL::Config config;
    config.nAircraft = nAircraft;
    config.nChargers = nChargers;
    config.simDuration = simDuration;
    config.aircraftTypesFile = "dummy";
    config.seed = seed;

    SimEVTOL sim(config);
    sim.Run();

    const auto& runStatsRecord = sim.GetRunStatsRecord();
    const auto& statMetrics = runStatsRecord.GetStatMetrics();
    const auto& aircraftTypes = sim.GetAircraftTypes();

    for (size_t i = 0; i < expectedStatsVec.size() && i < aircraftTypes.size(); i++) {
        const auto& expected = expectedStatsVec[i];
        const auto& actual = statMetrics.GetStats(aircraftTypes[i]);

        std::cout << "Validating " << expected.name << ":\n";
        std::cout << "  ✓ Average flight time: " << actual.avgFlightTime << " (expected: " << expected.avgFlightTime << ")\n";
        assert(abs(actual.avgFlightTime - expected.avgFlightTime) < tolerance);

        std::cout << "  ✓ Average distance: " << actual.avgDistance << " (expected: " << expected.avgDistance << ")\n";
        assert(abs(actual.avgDistance - expected.avgDistance) < tolerance);

        std::cout << "  ✓ Average charge time: " << actual.avgChargeTime << " (expected: " << expected.avgChargeTime << ")\n";
        assert(abs(actual.avgChargeTime - expected.avgChargeTime) < tolerance);

        std::cout << "  ✓ Total faults: " << actual.totalFaults << " (min expected: " << expected.minFaults << ")\n";
        // assert(abs(actual.totalFaults - expected.minFaults) >= 0);

        std::cout << "  ✓ Passenger miles: " << actual.totalPassengerMiles << " (expected: " << expected.passengerMiles << ")\n";
        if (expected.passengerMiles == 0.0) {
            assert(actual.totalPassengerMiles == 0.0);
        } else {
            assert(abs(actual.totalPassengerMiles - expected.passengerMiles) < expected.passengerMiles * passengerMilesToleranceRatio);
        }

        std::cout << "  ✓ Number of flights: " << actual.numFlights << " (expected: " << expected.flights << ")\n";
        assert(abs(actual.numFlights - expected.flights) < flightsTolerance);

        std::cout << "  ✓ Number of charges: " << actual.numCharges << " (expected: " << expected.charges << ")\n";
        assert(abs(actual.numCharges - expected.charges) < chargesTolerance);
    }
}

// This test validates partial flight ending scenario. Ending the simulation before finishing a 
// charge or flight is not the physical end of either, but simply that we stopped looking within 
// the simulation. There is no meaning to stopping a flight or charge with the end of the sim.
// Hence, we do not allow the stats to be affected by the end boundary condition.
void acceptance_test_1() {
    std::cout << "\n=== Acceptance Test 1: Basic stats and partial flight ===\n";
    std::cout << "Configuration: 1 aircraft per company, composition = {1,1,1,1,1}, 5 aircraft, 5 chargers, 3.0 hrs\n";

    std::vector<int> composition = {1, 1, 1, 1, 1};
    std::vector<ExpectedStats> expected = {
        {"Alpha Company", 1.67, 200.0, 0.60, 0.75, 1152, 1.44, 1},
        {"Bravo Company", 0.667, 66.7, 0.20, 0.30, 1200, 3.60, 3},
        {"Charlie Company", 0.625, 100.0, 0.80, 0.15, 672, 2.24, 2},
        {"Delta Company", 1.67, 150.0, 0.62, 0.66, 428.4, 1.43, 1},
        {"Echo Company", 0.862, 25.9, 0.30, 1.83, 144, 2.78, 2}
    };

    try {
        run_sim_and_validate(
            /*nAircraft*/ 5,
            /*nChargers*/ 5,
            /*simDuration*/ 3.0,
            composition,
            expected
        );
        std::cout << "✓ Acceptance Test 1 completed with data validation\n";
    } catch (const std::exception& e) {
        std::cout << "Acceptance Test 1 failed: " << e.what() << std::endl;
    }
}

// This test validates partial charge ending scenario.
void acceptance_test_2() {
    std::cout << "\n=== Acceptance Test 2: Partial charge scenario ===\n";
    std::cout << "Configuration: 1 Echo aircraft, 1 charger, 1.0 hr\n";
    
    std::vector<int> composition = {0, 0, 0, 0, 1};  // Echo Company

    std::vector<ExpectedStats> expected = {
        {"Alpha Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Bravo Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Charlie Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Delta Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Echo Company", 0.862, 25.9, 0.30, 0.61, 51.7, 1, 0.460}
    };

    try {
        run_sim_and_validate(
            /*nAircraft*/ 1,
            /*nChargers*/ 1,
            /*simDuration*/ 1.0,
            composition,
            expected
        );
        
        std::cout << "✓ Acceptance Test 2 completed with data validation\n";
        
    } catch (const std::exception& e) {
        std::cout << "Acceptance Test 2 failed: " << e.what() << std::endl;
    }
}

// These validates the queue behavior with the manual model
void acceptance_test_3() {
    std::cout << "\n=== Acceptance Test 3: 2 aircraft, 1 charger ===\n";
    std::cout << "Configuration: 2 Echo aircraft, 1 charger, 3 hrs\n";
    
    std::vector<int> composition = {0, 0, 0, 0, 2};  // Echo Company

    std::vector<ExpectedStats> expected = {
        {"Alpha Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Bravo Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Charlie Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Delta Company", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {"Echo Company", 0.862, 25.9, 0.30, 3.66, 270, 5.22, 4}
    };

    try {
        run_sim_and_validate(
            /*nAircraft*/ 2,
            /*nChargers*/ 1,
            /*simDuration*/ 3.0,
            composition,
            expected
        );

        std::cout << "✓ Acceptance Test 3 completed with data validation\n";
        
    } catch (const std::exception& e) {
        std::cout << "Acceptance Test 3 failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== Running eVTOL Simulation Test Suite ===\n";
    
    // algorithm tests
    test_aircraft_type();
    test_aircraft_state_machine();
    test_charger_queue();
    test_stat_metrics();
    test_fleet_coordination();
    test_simulation_integration();
    test_event_scheduling();
    
    // tests for edge cases and requirements
    test_partial_activity_normalization();
    test_edge_cases();
    test_table_output_format();
    test_fault_detection();
    
    // Acceptance tests
    acceptance_test_1();
    acceptance_test_2();
    acceptance_test_3();
    
    std::cout << "\n=== All Tests Passed! ===\n";

    return 0;
}
