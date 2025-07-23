#pragma once
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <map>

// Forward declarations
class AircraftType;
class Aircraft;
class Fleet;
class ChargerQueue;
class StatMetrics;
class RunStatsRecord;

// --- AircraftType ---
// Holds static parameters for each company/type
class AircraftType {
public:
    AircraftType(const std::string& name,
                 double cruiseSpeed,
                 double batteryCapacity,
                 double chargeTime,
                 double energyUsePerMile,
                 int passengerCount,
                 double faultProbPerHour);

    // Getters
    const std::string& GetName() const;
    double GetCruiseSpeed() const;
    double GetBatteryCapacity() const;
    double GetChargeTime() const;
    double GetEnergyUsePerMile() const;
    int GetPassengerCount() const;
    double GetFaultProbPerHour() const;

    // Calculated fields (optionally add as needed)
    double RangeMiles() const; // batteryCapacity / energyUsePerMile

private:
    std::string m_name;
    double m_cruiseSpeed;
    double m_batteryCapacity;
    double m_chargeTime;
    double m_energyUsePerMile;
    int m_passengerCount;
    double m_faultProbPerHour;
};

// --- Aircraft ---
// Represents a single aircraft in the simulation
class Aircraft {
public:
    enum State { AIRBORNE, WAITING_FOR_CHARGER, CHARGING, FINISHED };

    Aircraft(const AircraftType& type, int aircraftId);

    // Simulation step
    void Update(double time, double lastTime, StatMetrics& metrics, ChargerQueue& chargerQueue);

    // For charger queue interaction
    void RequestCharge(ChargerQueue& queue);
    void Unplug(ChargerQueue& queue);

    // Getters
    State GetState() const;
    int GetId() const;
    const AircraftType& GetType() const;

    // Next event time for scheduling
    double GetNextEventTime() const;

    // For stats
    double GetBatteryLevel() const;
    double GetTotalFlightTime() const;
    double GetTotalChargingTime() const;

private:
    int m_id;
    const AircraftType& m_type;
    State m_state;

    // Current simulation data
    double m_batteryLevel;
    double m_lastEventTime;
    double m_nextEventTime;

    // Statistics
    double m_totalFlightTime;
    double m_totalChargingTime;
    int m_numFaults;
    double m_totalDistance;
    double m_totalPassengerMiles;

    // Internal helpers
    bool CheckFault(double elapsedHours, std::mt19937& rng) const;
};

// --- Fleet ---
// Collection of all aircraft; interface for pulling updates
class Fleet {
public:
    Fleet(const std::vector<const AircraftType*>& types, const std::vector<int>& composition);
    std::vector<std::unique_ptr<Aircraft>>& GetAircraft();

    // Pull-based simulation update
    void Tick(double time, StatMetrics& metrics, ChargerQueue& chargerQueue);

    // For scheduling
    double GetNextEventTime() const;

    // Set run stats record pointer
    void SetRunStatsRecord(RunStatsRecord* record);

private:
    std::vector<std::unique_ptr<Aircraft>> m_aircraft;
    RunStatsRecord* m_runStatsRecord;
};

// --- ChargerQueue ---
// Manages charger slots and queueing
class ChargerQueue {
public:
    ChargerQueue(int nChargers);

    // Aircraft request/leave charger
    void PlugIn(Aircraft* vehicle, double time);
    void Unplug(Aircraft* vehicle, double time);

    // Query queue state
    bool IsChargerAvailable() const;
    int NumCharging() const;

private:
    int m_nChargers;
    std::vector<Aircraft*> m_chargingNow;
    std::vector<Aircraft*> m_waitingQueue;
};

// --- StatMetrics ---
// Records stats for simulation (per vehicle type)
class StatMetrics {
public:
    StatMetrics(const std::vector<const AircraftType*>& types);

    // Called by Aircraft to record events
    void TimeElapsedUpdateStats(const AircraftType& type, Aircraft::State state, double dt);
    void FlightComplete(const Aircraft& vehicle, double time);
    void ChargingStart(const Aircraft& vehicle, double time);
    void ChargingCompleteFlightStart(const Aircraft& vehicle, double time);

    // Finalize and calculate averages
    void FinalizeFlightChargeData();
    void CalcAveragesAndCounts(const AircraftType& type);

    // Get stats for reporting
    struct Stats {
        double avgFlightTime;
        double avgDistance;
        double avgChargeTime;
        int totalFaults;
        double totalPassengerMiles;
        int numFlights;
        int numCharges;
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
// Records and reports overall simulation stats
class RunStatsRecord {
public:
    RunStatsRecord(const std::vector<const AircraftType*>& types);

    StatMetrics& GetStatMetrics();
    void ReportStatMetrics();

private:
    StatMetrics m_statMetrics;
};

// --- SimEVTOL ---
// Main simulation orchestrator (pull-based)
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

    // Rank to composition for random type allocation
    std::vector<int> rank_to_composition(unsigned int seed);

    // Load aircraft types from CSV
    std::vector<std::unique_ptr<AircraftType>> CreateTypes(const std::string& csvFile);

private:
    Config m_config;
    std::vector<std::unique_ptr<AircraftType>> m_aircraftTypes;
    std::unique_ptr<Fleet> m_fleet;
    std::unique_ptr<ChargerQueue> m_chargerQueue;
    std::unique_ptr<RunStatsRecord> m_runStatsRecord;
    double m_time;
    double m_lastTime;
    std::mt19937 m_rng;
};
