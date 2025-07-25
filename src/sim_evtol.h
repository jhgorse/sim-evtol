//=============================================================================
// sim_evtol.h
//-----------------------------------------------------------------------------
// eVTOL Simulation Core Header
//
// This header defines the main classes and interfaces for the eVTOL simulation,
// including aircraft types, individual aircraft, fleet management, charger
// queueing, statistics collection, and the simulation orchestrator.
//
// Key Components:
// - AircraftType: Defines static parameters for each aircraft company/type.
// - Aircraft: Represents a single aircraft and its state in the simulation.
// - Fleet: Manages a collection of aircraft and their updates.
// - ChargerQueue: Handles charger slot allocation and waiting queues.
// - StatMetrics: Collects and aggregates simulation statistics by type.
// - RunStatsRecord: Reports overall simulation statistics.
// - SimEVTOL: Main simulation orchestrator, configuration, and reporting.
//
// Design Notes:
// - AircraftType is associated with Aircraft by composition for simplicity.
// - Aircraft are managed externally by unique_ptr for safe memory management.
// - The simulation is event-driven and statistics are collected per type.
//
// Author: Joe Gorse
// License: See LICENSE file for details.
//

#pragma once
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <map>

class AircraftType;
class Aircraft;
class Fleet;
class ChargerQueue;
class StatMetrics;
class RunStatsRecord;

// --- AircraftType ---
// Holds static parameters for each company/type. It is used for stats and airplane identity.
// Rather than inheretence, we will use association by object. This composition is simpler and
// more closely coupled to the data.
// Inheretence could become interesting if we needed more type-specific behavior.
class AircraftType {
public:
    AircraftType(const std::string& name,
                 double cruiseSpeed,
                 double batteryCapacity,
                 double chargeTime,
                 double energyUsePerMile,
                 int passengerCount,
                 double faultProbPerHour);

    double RangeMiles() const; // batteryCapacity / energyUsePerMile

    std::string name;
    double cruiseSpeed;
    double batteryCapacity;
    double chargeTime;
    double energyUsePerMile;
    int passengerCount;
    double faultProbPerHour;
};

// --- Aircraft ---
// Represents a single aircraft in the simulation. A set of aircraft are contained in the Fleet.
// Each Aircraft interacts with the ChargerQueue, time via Sim, and StatMetrics to record moments 
// of interest in time.
class Aircraft {
public:
    enum State { AIRBORNE, WAITING_FOR_CHARGER, CHARGING, FINISHED };

    Aircraft(const AircraftType& type, int aircraftId);

    void Update(double time, double lastTime, StatMetrics& metrics, ChargerQueue& chargerQueue);

    void RequestCharge(ChargerQueue& queue);
    void Unplug(ChargerQueue& queue);
    void StartCharging(double time); // Called by ChargerQueue when charger becomes available

    State GetState() const;
    double GetNextEventTime() const;
    const AircraftType& GetType() const;
    
    // Fault checking for time step, used by Fleet
    bool CheckFaultDuringTimeStep(double deltaTime, std::mt19937& rng) const;

    int id;
    State state;
    double batteryLevel;
    double lastEventTime;
    double nextEventTime;
    
    double totalFlightTime;
    double totalChargingTime;
    int numFaults;
    double totalDistance;
    double totalPassengerMiles;

private:
    const AircraftType& m_type;

    bool CheckFault(double elapsedHours, std::mt19937& rng) const;
};

// --- Fleet ---
// Collection of all aircraft. Interface for the group of aircraft.
class Fleet {
public:
    Fleet(const std::vector<AircraftType>& types, const std::vector<int>& composition);
    const std::vector<std::unique_ptr<Aircraft>>& GetAircraft() const;  // TODO unused except in test

    // Pull-based simulation update
    void Tick(double time, StatMetrics& metrics, ChargerQueue& chargerQueue, std::mt19937& rng);

    // For scheduling
    double GetNextEventTime() const;

    // Set run stats record pointer
    void SetRunStatsRecord(RunStatsRecord* record);

private:
    std::vector<std::unique_ptr<Aircraft>> m_aircraft;
    RunStatsRecord* m_runStatsRecord;
};

// --- ChargerQueue ---
// Manages charger slots and queueing. Determines when to connect an aircraft to the charger.
class ChargerQueue {
public:
    ChargerQueue(int nChargers);

    void PlugIn(Aircraft* vehicle, double time);
    void Unplug(Aircraft* vehicle, double time);

    bool IsChargerAvailable() const;
    int NumCharging() const;
    
    std::vector<Aircraft*> m_chargingNow;
    std::vector<Aircraft*> m_waitingQueue;

private:
    int m_nChargers;
};

// --- StatMetrics ---
// Records stats for simulation (per vehicle type). 
// Used by each aircraft during state changes, and the start and end of the simulation.
class StatMetrics {
public:
    StatMetrics(const std::vector<AircraftType>& types);

    // Called by Aircraft to record events
    void TimeElapsedUpdateStats(const AircraftType& type, Aircraft::State state, double dt);
    void FlightComplete(const Aircraft& vehicle, double time);
    void ChargingStart(const Aircraft& vehicle, double time);
    void ChargingCompleteFlightStart(const Aircraft& vehicle, double time);

    void FinalizeFlightChargeData();
    void FinalizePartialActivities(const std::vector<std::unique_ptr<Aircraft>>& aircraft, double simEndTime);
    void CalcAveragesAndCounts(const AircraftType& type);
    void CalcAveragesAndCounts(const std::string& typeName);

    // Get stats for reporting
    struct Stats {
        double avgFlightTime;
        double avgDistance;
        double avgChargeTime;
        int totalFaults;
        double totalPassengerMiles;
        double numFlights;
        double numCharges;
    };
    const Stats& GetStats(const AircraftType& type) const;

private:
    std::map<std::string, Stats> m_statsByType;
    std::map<std::string, std::vector<double>> m_flightTimesByType;
    std::map<std::string, std::vector<double>> m_distancesByType;
    std::map<std::string, std::vector<double>> m_chargeTimesByType;
    std::map<std::string, int> m_faultsByType;
    std::map<std::string, double> m_passengerMilesByType;
};

// --- RunStatsRecord ---
// Reports overall simulation stats. It uses StatMetrics records.
class RunStatsRecord {
public:
    RunStatsRecord(const std::vector<AircraftType>& types);

    StatMetrics& GetStatMetrics();
    const StatMetrics& GetStatMetrics() const;
    void ReportStatMetrics(const Fleet& fleet, double simEndTime);

private:
    StatMetrics m_statMetrics;
    std::vector<AircraftType> m_aircraftTypes;
};

// --- SimEVTOL ---
// Main simulation orchestrator
// Configure, Run, and Report. This simulator pulls information from the subordinate objects
// to generate the report statistics.
class SimEVTOL {
public:
    struct Config {
        int nAircraft;
        int nChargers;
        double simDuration;
        std::string aircraftTypesFile;
        unsigned int seed;
    };

    SimEVTOL(const Config& config);

    // Run simulation
    void Run();
    
    const RunStatsRecord& GetRunStatsRecord() const;
    
    // TODO: below this belongs in SimEVTOL::Config
    const std::vector<AircraftType>& GetAircraftTypes() const;
    int NChooseK(int n, int k);
    std::vector<int> RankToComposition(unsigned int seed);
    int CompositionToRank(const std::vector<int>& composition);
    int GetMaxRank();
    void CreateTypesFromFile(const std::string& csvFile);  // TODO: Load aircraft types from CSV

private:
    Config m_config;
    std::vector<AircraftType> m_aircraftTypes;
    std::unique_ptr<Fleet> m_fleet;
    std::unique_ptr<ChargerQueue> m_chargerQueue;
    std::unique_ptr<RunStatsRecord> m_runStatsRecord;
    double m_time;
    double m_lastTime;
    std::mt19937 m_rng;
};
