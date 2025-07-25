//=============================================================================
// sim_evtol.cpp
//-----------------------------------------------------------------------------
// eVTOL Simulation Core Implementation
//
// This file contains the main implementation of the eVTOL simulation logic,
// including the definitions for AircraftType, Aircraft, Fleet, ChargerQueue,
// StatMetrics, RunStatsRecord, and SimEVTOL classes.
//
// Key Features:
// - Defines aircraft types and their operational parameters.
// - Manages the lifecycle and state transitions of individual aircraft.
// - Handles charger queueing and charging logic for the fleet.
// - Collects and aggregates simulation statistics by aircraft type.
// - Orchestrates the event-driven simulation loop and reporting.
//
// Design Notes:
// - AircraftType is associated with Aircraft by composition, not inheritance.
// - Aircraft are managed by unique_ptr for safe, automatic memory management.
// - The simulation is event-driven, advancing time to the next scheduled event.
// - Statistics are collected for reporting at the end of the simulation.
//
// Author: Joe Gorse
// License: See LICENSE file for details.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <functional>
#include "sim_evtol.h"

// --- AircraftType Implementation ---
AircraftType::AircraftType(const std::string& name,
                           double cruiseSpeed,
                           double batteryCapacity,
                           double chargeTime,
                           double energyUsePerMile,
                           int passengerCount,
                           double faultProbPerHour)
    : name(name)
    , cruiseSpeed(cruiseSpeed)
    , batteryCapacity(batteryCapacity)
    , chargeTime(chargeTime)
    , energyUsePerMile(energyUsePerMile)
    , passengerCount(passengerCount)
    , faultProbPerHour(faultProbPerHour)
{
}

double AircraftType::RangeMiles() const {
    return batteryCapacity / energyUsePerMile;
}

// --- Aircraft Implementation ---
Aircraft::Aircraft(const AircraftType& type, int aircraftId)
    : id(aircraftId)
    , m_type(type)
    , state(AIRBORNE)
    , batteryLevel(type.batteryCapacity)  // Start with full battery
    , lastEventTime(0.0)
    , nextEventTime(0.0)
    , totalFlightTime(0.0)
    , totalChargingTime(0.0)
    , numFaults(0)
    , totalDistance(0.0)
    , totalPassengerMiles(0.0)
{
    double flightTime = m_type.RangeMiles() / m_type.cruiseSpeed;  // First flight and event time
    nextEventTime = flightTime;
}

void Aircraft::Update(double time, double lastTime, StatMetrics& metrics, ChargerQueue& chargerQueue) {
    double deltaTime = time - this->lastEventTime;
    this->lastEventTime = time;

    metrics.TimeElapsedUpdateStats(m_type, state, deltaTime);
    
    switch (state) {  // State machine
        case AIRBORNE: {
            // Complete the flight at event time
            double distance = m_type.RangeMiles(); // Full flight distance
            totalDistance += distance;
            totalPassengerMiles += distance * m_type.passengerCount;
            totalFlightTime += deltaTime;
            
            // Consume battery for full flight
            batteryLevel -= distance * m_type.energyUsePerMile;
            
            // Flight completed successfully, need to charge
            metrics.FlightComplete(*this, time);
            RequestCharge(chargerQueue);
            break;
        }
        
        case WAITING_FOR_CHARGER:
            // Nothing to do while waiting - charger queue will handle transition
            nextEventTime = std::numeric_limits<double>::max();
            break;
            
        case CHARGING: {
            totalChargingTime += deltaTime;
            
            // is charging is complete
            if (totalChargingTime >= m_type.chargeTime && state == CHARGING) {
                batteryLevel = m_type.batteryCapacity; // Full charge
                Unplug(chargerQueue); lastEventTime;
                metrics.ChargingCompleteFlightStart(*this, time);
                state = AIRBORNE;
                
                // Calculate next flight completion time based on full range
                double flightTime = m_type.RangeMiles() / m_type.cruiseSpeed;
                nextEventTime = time + flightTime + 1e-9; // Add small epsilon to avoid exact equality
            }
            break;  // If still charging, keep the original next event time
        }
        
        case FINISHED:
            nextEventTime = std::numeric_limits<double>::max();
            break;
    }
}

void Aircraft::RequestCharge(ChargerQueue& queue) {
    state = WAITING_FOR_CHARGER;
    nextEventTime = std::numeric_limits<double>::max(); // Wait indefinitely until charger available
    queue.PlugIn(this, lastEventTime);
}

void Aircraft::Unplug(ChargerQueue& queue) {
    queue.Unplug(this, lastEventTime);
}

void Aircraft::StartCharging(double time) {
    state = CHARGING;
    totalChargingTime = 0.0; // Reset charge time for this session
    lastEventTime = time;
    nextEventTime = time + m_type.chargeTime + 1e-9; // Add small epsilon
}

Aircraft::State Aircraft::GetState() const {
    return state;
}

const AircraftType& Aircraft::GetType() const {
    return m_type;
}

double Aircraft::GetNextEventTime() const {
    return nextEventTime;
}

bool Aircraft::CheckFault(double elapsedHours, std::mt19937& rng) const {
    // Simple fault probability check
    double faultProb = m_type.faultProbPerHour * elapsedHours;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < faultProb;
}

bool Aircraft::CheckFaultDuringTimeStep(double deltaTime, std::mt19937& rng) const {
    // Check for fault during the time step
    double faultProb = m_type.faultProbPerHour * deltaTime;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < faultProb;
}

// --- ChargerQueue Implementation ---
ChargerQueue::ChargerQueue(int nChargers)
    : m_nChargers(nChargers)
{
}

void ChargerQueue::PlugIn(Aircraft* aircraft, double time) {
    if (m_chargingNow.size() < static_cast<size_t>(m_nChargers)) {
        // Charger available - start charging immediately
        m_chargingNow.push_back(aircraft);
        aircraft->StartCharging(time);
    } else {
        // No chargers available - add to waiting queue
        m_waitingQueue.push_back(aircraft);
    }
}

void ChargerQueue::Unplug(Aircraft* aircraft, double time) {
    // Remove from charging
    auto it = std::find(m_chargingNow.begin(), m_chargingNow.end(), aircraft);
    if (it != m_chargingNow.end()) {
        m_chargingNow.erase(it);
        
        // If there's a waiting aircraft, start charging it
        if (!m_waitingQueue.empty()) {
            Aircraft* nextAircraft = m_waitingQueue.front();
            m_waitingQueue.erase(m_waitingQueue.begin());
            m_chargingNow.push_back(nextAircraft);
            nextAircraft->StartCharging(time);
        }
    }
}

bool ChargerQueue::IsChargerAvailable() const {
    return m_chargingNow.size() < static_cast<size_t>(m_nChargers);
}

int ChargerQueue::NumCharging() const {
    return static_cast<int>(m_chargingNow.size());
}

// Continue with other class implementations...
// This is getting quite large, so I'll implement them incrementally

// --- StatMetrics Implementation ---
StatMetrics::StatMetrics(const std::vector<AircraftType>& types) {
    for (const auto& type : types) {
        m_statsByType[type.name] = Stats{};
        m_flightTimesByType[type.name] = std::vector<double>();
        m_distancesByType[type.name] = std::vector<double>();
        m_chargeTimesByType[type.name] = std::vector<double>();
        m_faultsByType[type.name] = 0;
        m_passengerMilesByType[type.name] = 0.0;
    }
}

void StatMetrics::TimeElapsedUpdateStats(const AircraftType& type, Aircraft::State state, double dt) {
    // This could be used to track time spent in each state
    // For now, just track the time - detailed state tracking can be added later
}

void StatMetrics::FlightComplete(const Aircraft& aircraft, double time) {
    const std::string& typeName = aircraft.GetType().name;
    
    // Record the individual flight metrics, not cumulative totals
    double flightTime = aircraft.GetType().RangeMiles() / aircraft.GetType().cruiseSpeed;
    double flightDistance = aircraft.GetType().RangeMiles();
    double passengerMiles = flightDistance * aircraft.GetType().passengerCount;
    
    m_flightTimesByType[typeName].push_back(flightTime);
    m_distancesByType[typeName].push_back(flightDistance);
    m_passengerMilesByType[typeName] += passengerMiles;
    m_statsByType[typeName].numFlights++;
}

void StatMetrics::ChargingStart(const Aircraft& aircraft, double time) {
    // Mark the start of charging - could be used for detailed tracking
}

void StatMetrics::ChargingCompleteFlightStart(const Aircraft& aircraft, double time) {
    const std::string& typeName = aircraft.GetType().name;
    
    // Record the individual charge time, not cumulative total
    double chargeTime = aircraft.GetType().chargeTime;
    
    m_chargeTimesByType[typeName].push_back(chargeTime);
    m_statsByType[typeName].numCharges++;
}

void StatMetrics::FinalizeFlightChargeData() {
    for (auto& pair : m_statsByType) {
        CalcAveragesAndCounts(pair.first);
    }
}

// We use unique_ptr here to ensure no copies of the individual aircraft are made.
void StatMetrics::FinalizePartialActivities(const std::vector<std::unique_ptr<Aircraft>>& aircraft, double simEndTime) {
    // Aggregate fault counts from all aircraft by type
    for (const auto& ac : aircraft) {
        const std::string& typeName = ac->GetType().name;
        m_faultsByType[typeName] += ac->numFaults;
    }
    
    // Process any aircraft that have partial activities at simulation end
    for (const auto& ac : aircraft) {
        const std::string& typeName = ac->GetType().name;
        
        if (ac->GetState() == Aircraft::AIRBORNE) {
            // Aircraft was in mid-flight when simulation ended
            // Calculate completion ratio based on time since flight started
            double flightStartTime = ac->lastEventTime;
            double elapsedFlightTime = simEndTime - flightStartTime;
            double expectedFlightTime = ac->GetType().RangeMiles() / ac->GetType().cruiseSpeed;
            
            if (elapsedFlightTime > 0 && expectedFlightTime > 0) {
                double completionRatio = std::min(1.0, elapsedFlightTime / expectedFlightTime);
                
                // Add the normalized flight data
                m_flightTimesByType[typeName].push_back(expectedFlightTime);  // Use full expected time
                
                // Add partial distance based on completion ratio
                double expectedDistance = ac->GetType().RangeMiles();
                m_distancesByType[typeName].push_back(expectedDistance);  // Use full expected distance
                
                // Add partial passenger miles
                m_passengerMilesByType[typeName] += expectedDistance * ac->GetType().passengerCount * completionRatio;
                
                // Count as a partial flight (weighted)
                m_statsByType[typeName].numFlights += completionRatio;
            }
        }
        else if (ac->GetState() == Aircraft::CHARGING) {
            // Aircraft was in mid-charge when simulation ended
            double chargeStartTime = ac->lastEventTime;
            double elapsedChargeTime = simEndTime - chargeStartTime;
            double expectedChargeTime = ac->GetType().chargeTime;
            
            if (elapsedChargeTime > 0 && expectedChargeTime > 0) {
                double completionRatio = std::min(1.0, elapsedChargeTime / expectedChargeTime);
                
                // Add the normalized charge data
                m_chargeTimesByType[typeName].push_back(expectedChargeTime);  // Use full expected time
                
                // Count as a partial charge (weighted)
                m_statsByType[typeName].numCharges += completionRatio;
            }
        }
    }
    
    // Recalculate averages after adding partial activities
    FinalizeFlightChargeData();
}

void StatMetrics::CalcAveragesAndCounts(const AircraftType& type) {
    CalcAveragesAndCounts(type.name);
}

void StatMetrics::CalcAveragesAndCounts(const std::string& typeName) {
    Stats& stats = m_statsByType[typeName];
    
    // Calculate averages
    const auto& flightTimes = m_flightTimesByType[typeName];
    if (!flightTimes.empty()) {
        double sum = 0.0;
        for (double time : flightTimes) {
            sum += time;
        }
        stats.avgFlightTime = sum / flightTimes.size();
    }
    
    const auto& distances = m_distancesByType[typeName];
    if (!distances.empty()) {
        double sum = 0.0;
        for (double dist : distances) {
            sum += dist;
        }
        stats.avgDistance = sum / distances.size();
    }
    
    const auto& chargeTimes = m_chargeTimesByType[typeName];
    if (!chargeTimes.empty()) {
        double sum = 0.0;
        for (double time : chargeTimes) {
            sum += time;
        }
        stats.avgChargeTime = sum / chargeTimes.size();
    }
    
    stats.totalFaults = m_faultsByType[typeName];
    stats.totalPassengerMiles = m_passengerMilesByType[typeName];
}

const StatMetrics::Stats& StatMetrics::GetStats(const AircraftType& type) const {
    auto it = m_statsByType.find(type.name);
    if (it != m_statsByType.end()) {
        return it->second;
    }
    throw std::runtime_error("No stats found for aircraft type: " + type.name);
}

// --- Fleet Implementation ---
Fleet::Fleet(const std::vector<AircraftType>& types, const std::vector<int>& composition)
    : m_runStatsRecord(nullptr)
{
    if (types.size() != composition.size()) {
        throw std::invalid_argument("Types and composition vectors must have same size");
    }
    
    int aircraftId = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        for (int j = 0; j < composition[i]; ++j) {
            m_aircraft.push_back(std::make_unique<Aircraft>(types[i], aircraftId++));
        }
    }
}

const std::vector<std::unique_ptr<Aircraft>>& Fleet::GetAircraft() const {
    return m_aircraft;
}

void Fleet::Tick(double time, StatMetrics& metrics, ChargerQueue& chargerQueue, std::mt19937& rng) {
    static double lastGlobalTime = 0.0;
    double deltaTime = time - lastGlobalTime;
    
    if (deltaTime > 0) {  // Faults
        for (auto& aircraft : m_aircraft) {
            if (aircraft->GetState() != Aircraft::FINISHED) {
                if (aircraft->CheckFaultDuringTimeStep(deltaTime, rng)) {
                    aircraft->numFaults++;
                }
            }
        }
        lastGlobalTime = time;
    }
    
    // Update aircraft that have events at this time
    for (auto& aircraft : m_aircraft) {
        if (aircraft->GetNextEventTime() <= time) {
            aircraft->Update(time, time, metrics, chargerQueue);
        }
    }
}

double Fleet::GetNextEventTime() const {
    double nextTime = std::numeric_limits<double>::max();
    for (const auto& aircraft : m_aircraft) {
        double aircraftTime = aircraft->GetNextEventTime();
        if (aircraftTime < nextTime) {
            nextTime = aircraftTime;
        }
    }
    return nextTime;
}

void Fleet::SetRunStatsRecord(RunStatsRecord* record) {
    m_runStatsRecord = record;
}

// --- RunStatsRecord Implementation ---
RunStatsRecord::RunStatsRecord(const std::vector<AircraftType>& types)
    : m_statMetrics(types)
    , m_aircraftTypes(types) // Store reference to the types for reporting
{
}

StatMetrics& RunStatsRecord::GetStatMetrics() {
    return m_statMetrics;
}

const StatMetrics& RunStatsRecord::GetStatMetrics() const {
    return m_statMetrics;
}

void RunStatsRecord::ReportStatMetrics(const Fleet& fleet, double simEndTime) {
    // First handle partial activities at simulation end
    m_statMetrics.FinalizePartialActivities(fleet.GetAircraft(), simEndTime);
    
    std::cout << "\n=== eVTOL Simulation Results (Table Format) ===\n";
    
    // Print table header
    std::cout << std::left << std::setw(25) << "Metric";
    for (const auto& type : m_aircraftTypes) {
        std::cout << std::setw(16) << type.name;
    }
    std::cout << "\n";
    
    // Print separator line
    std::cout << std::string(25 + 16 * m_aircraftTypes.size(), '-') << "\n";
    
    // Helper lambda to print a row
    auto printRow = [&](const std::string& metric, std::function<std::string(const StatMetrics::Stats&)> valueFunc) {
        std::cout << std::left << std::setw(25) << metric;
        for (const auto& type : m_aircraftTypes) {
            try {
                const auto& stats = m_statMetrics.GetStats(type);
                std::cout << std::setw(16) << valueFunc(stats);
            } catch (const std::exception& e) {
                std::cout << std::setw(16) << "N/A";
            }
        }
        std::cout << "\n";
    };
    
    // Print all metrics as rows
    printRow("Average flight time (hrs)", [](const auto& s) { 
        std::ostringstream oss; 
        oss << std::fixed << std::setprecision(3) << s.avgFlightTime; 
        return oss.str(); 
    });
    printRow("Average distance (miles)", [](const auto& s) { 
        std::ostringstream oss; 
        oss << std::fixed << std::setprecision(1) << s.avgDistance; 
        return oss.str(); 
    });
    printRow("Average charge time (hrs)", [](const auto& s) { 
        std::ostringstream oss; 
        oss << std::fixed << std::setprecision(3) << s.avgChargeTime; 
        return oss.str(); 
    });
    printRow("Total faults", [](const auto& s) { return std::to_string(s.totalFaults); });
    printRow("Total passenger miles", [](const auto& s) { 
        std::ostringstream oss; 
        oss << std::fixed << std::setprecision(1) << s.totalPassengerMiles; 
        return oss.str(); 
    });
    printRow("Number of flights", [](const auto& s) { 
        std::ostringstream oss; 
        oss << std::fixed << std::setprecision(2) << s.numFlights; 
        return oss.str(); 
    });
    printRow("Number of charges", [](const auto& s) { 
        std::ostringstream oss; 
        oss << std::fixed << std::setprecision(2) << s.numCharges; 
        return oss.str(); 
    });
    
    std::cout << "\nSimulation completed.\n";
}

// --- SimEVTOL Implementation ---
SimEVTOL::SimEVTOL(const Config& config)
    : m_config(config)
    , m_time(0.0)
    , m_lastTime(0.0)
    , m_rng(config.seed)
{
    CreateTypesFromFile(config.aircraftTypesFile);
    
    // Create fleet composition
    std::vector<int> composition = RankToComposition(config.seed);
    
    // Create simulation components
    m_fleet = std::make_unique<Fleet>(m_aircraftTypes, composition);
    m_chargerQueue = std::make_unique<ChargerQueue>(config.nChargers);
    m_runStatsRecord = std::make_unique<RunStatsRecord>(m_aircraftTypes);
    
    // Connect fleet to stats
    m_fleet->SetRunStatsRecord(m_runStatsRecord.get());
}

void SimEVTOL::Run() {
    std::cout << "Run simulation with ";
    std::cout << "Seed: " << m_config.seed << ", Composition: ";
    for (auto i : RankToComposition(m_config.seed)) std::cout << i << " ";
    std::cout << ", Aircraft: " << m_config.nAircraft << ", Chargers: " << m_config.nChargers;
    std::cout << ", Duration: " << m_config.simDuration << " hours\n";
    
    // Pure event-driven simulation: only advance time to next aircraft events
    while (m_time < m_config.simDuration) {
        m_lastTime = m_time;
        
        double nextEventTime = m_fleet->GetNextEventTime();  // Find the nearest next event from the Aircraft objects
        
        if (nextEventTime == std::numeric_limits<double>::max())  // If no more events end simulation.
            break;
        if (nextEventTime <= m_config.simDuration)
            m_time = nextEventTime;  // if next event is beyond simulation duration, coerce to sim duration.
        else
            m_time = m_config.simDuration;
        
        m_fleet->Tick(m_time, m_runStatsRecord->GetStatMetrics(), *m_chargerQueue, m_rng);
    }
    
    std::cout << "Simulation completed at time " << m_time << " hours\n";
    
    // Generate final report with partial activity handling
    m_runStatsRecord->ReportStatMetrics(*m_fleet, m_time);
}

const RunStatsRecord& SimEVTOL::GetRunStatsRecord() const {
    return *m_runStatsRecord;
}

const std::vector<AircraftType>& SimEVTOL::GetAircraftTypes() const {
    return m_aircraftTypes;
}

int SimEVTOL::NChooseK(int n, int k) {
    if (k == 0) return 1;
    return (n * SimEVTOL::NChooseK(n - 1, k - 1)) / k;
}

int SimEVTOL::GetMaxRank() {
    int N = this->m_config.nAircraft, K = this->m_aircraftTypes.size();
    return NChooseK(N + K - 1, K - 1);
}

std::vector<int> SimEVTOL::RankToComposition(unsigned int seed) {
    int N = this->m_config.nAircraft, K = this->m_aircraftTypes.size(), 
    rank = seed - 1, remaining = N;
    if (seed < 1 || seed > GetMaxRank())
        throw std::invalid_argument("Bad rank value " + std::to_string(seed)); 

    std::vector<int> composition(K, 0);
    for (int i = 0; i < K - 1; ++i) {
        composition[i] = 0;
        while (true) {
            int count = SimEVTOL::NChooseK(remaining - composition[i] + K - i - 2, K - i - 2);
            if (count > rank) break;
            rank -= count;
            composition[i]++;
        }
        remaining -= composition[i];
    }
    composition[K - 1] = remaining;
    return composition;
}

int SimEVTOL::CompositionToRank(const std::vector<int>& composition) {
    int N = this->m_config.nAircraft, K = this->m_aircraftTypes.size();

    for (auto i : composition)
        if (i < 0 || i > N)
            throw std::invalid_argument("Bad vector composition value " + std::to_string(i)); 

    int rank = 1, remaining = N;
    for (int i = 0; i < K - 1; ++i) {
        for (int a = 0; a < composition[i]; ++a) {
            rank += SimEVTOL::NChooseK(remaining - a + K - i - 2, K - i - 2);
        }
        remaining -= composition[i];
    }
    return rank;
}

void SimEVTOL::CreateTypesFromFile(const std::string& csvFile) {
    // Aircraft types based on the provided source table:
    // Company Name | Cruise Speed (mph) | Battery Capacity (kWh) | Energy use at Cruise (kWh/mile) | Passenger Count | Probability of fault per hour | Time to Charge (hours)
    
    AircraftType alpha {"Alpha Company", 120.0, 320.0, 0.6, 1.6, 4, 0.25};
    AircraftType bravo {"Bravo Company", 100.0, 100.0, 0.2, 1.5, 5, 0.10};
    AircraftType charlie {"Charlie Company", 160.0, 220.0, 0.8, 2.2, 3, 0.05};
    AircraftType delta {"Delta Company", 90.0, 120.0, 0.62, 0.8, 2, 0.22};
    AircraftType echo {"Echo Company", 30.0, 150.0, 0.3, 5.8, 2, 0.61};
    
    m_aircraftTypes = {alpha, bravo, charlie, delta, echo};
}
