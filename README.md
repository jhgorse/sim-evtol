# eVTOL Simulation Problem

- Any Time Domain

## Reference Documents

1. [eVtol\_Simulation\_Problem\_-\_AnyTimeDomain\_V2\_(1).pdf](https://drive.google.com/file/d/1SPi4VnmAFmv-zx6UiDNhYBel8Qd28zie/view?usp=sharing)  
   1. [eVTOL Aircraft Manual Calculations.xlsx](https://github.com/jhgorse/sim-evtol/blob/dev/doc/eVTOL%20Aircraft%20Manual%20Calculations.xlsx)
   2. [eVtol\_Simulation\_Problem\_-\_AnyTimeDomain\_V2\_(1).doc](https://docs.google.com/document/d/1PSqVqTUEh6liWE_RJonXginCqF4PEbdtAV1Q7q176Dw/edit?tab=t.0)
   3. [eVTOL Simulation Prelim Design Drawings.pptx](https://github.com/jhgorse/sim-evtol/blob/dev/doc/eVTOL%20Simulation%20Prelim%20Design%20Drawings.pptx)
2. [pub\_scientific-and-engineering-c-an-introduction-with-advanced-techniques-and-examples.pdf](https://drive.google.com/file/d/1TIf4128zKpGNFEyiPEXxS-ivI-1mkVUL/view?usp=drive_link)

# Log

## 2025-08-25 Friday

- [x] ~~GH Actions~~  
- [x] Gather docs into repo  
- [x] Final Review  
- [x] Release

`sim-evtol` Output Sample:
https://github.com/jhgorse/sim-evtol/actions/runs/16529697879/job/46751801744#step:8:11

```
=== eVTOL Simulation JHG ===
Run simulation with Seed: 1, Composition: 0 0 0 0 20 , Aircraft: 20, Chargers: 3, Duration: 3 hours
Simulation completed at time 3 hours

=== eVTOL Simulation Results (Table Format) ===
Metric                   Alpha Company   Bravo Company   Charlie Company Delta Company   Echo Company    
---------------------------------------------------------------------------------------------------------
Average flight time (hrs)0.000           0.000           0.000           0.000           0.862           
Average distance (miles) 0.0             0.0             0.0             0.0             25.9            
Average charge time (hrs)0.000           0.000           0.000           0.000           0.300           
Total faults             0               0               0               0               40              
Total passenger miles    0.0             0.0             0.0             0.0             1837.7          
Number of flights        0.00            0.00            0.00            0.00            35.53           
Number of charges        0.00            0.00            0.00            0.00            21.38           

Simulation completed.
Run simulation with Seed: 2, Composition: 0 0 0 1 19 , Aircraft: 20, Chargers: 3, Duration: 3 hours
=== Last 17 lines ===

Simulation completed.
Run simulation with Seed: 10626, Composition: 20 0 0 0 0 , Aircraft: 20, Chargers: 3, Duration: 3 hours
Simulation completed at time 3 hours

=== eVTOL Simulation Results (Table Format) ===
Metric                   Alpha Company   Bravo Company   Charlie Company Delta Company   Echo Company    
---------------------------------------------------------------------------------------------------------
Average flight time (hrs)1.667           0.000           0.000           0.000           0.000           
Average distance (miles) 200.0           0.0             0.0             0.0             0.0             
Average charge time (hrs)0.600           0.000           0.000           0.000           0.000           
Total faults             0               0               0               0               0               
Total passenger miles    17248.0         0.0             0.0             0.0             0.0             
Number of flights        21.56           0.00            0.00            0.00            0.00            
Number of charges        6.67            0.00            0.00            0.00            0.00            

Simulation completed.
```

The entire simulation takes 649 ms in a single-thread on modern hardware for the 10,626 combinations of 5 aircraft types 
adding to a fleet of 20. Captured above are the first and last runs from the Github action.

## 2025-08-24 Thursday

Tweaks and tests.  
0625  
Faults.

- [x] ~~remember the stash.~~

Final acceptance calculations and tests by spreadsheet[eVTOL Aircraft](https://docs.google.com/spreadsheets/d/1-LTgdlu5lFlGsYdDcAFw6e9xE-063WjqAM_zu5wDRs4/edit?gid=0#gid=0).

Make the acceptance tests.  
1759

Back to it.  
2100  
Massage stats verification.

- [x] ~~Code Comments~~  
- [x] ~~Final Documentation~~

2158

## 2025-08-23 Wednesday

1046  
1346  
1401  
We are going to skip class diagram reconciliation while things are in flux and write up tests for key components.

### Tests

#### ChargerQueue Assignment of Aircraft instances

- [x] ~~First 3 are state-\>charging~~  
- [x] ~~Interface: RequestCharge(), PlugIn(a&), UnPlug(this)~~

#### Passenger-Mile Calculation

#### Fault Simulation

#### Aircraft class

- [x] ~~GetNextEventTime()~~

Implementations.

## 2025-07-22 Tuesday

- [x] ~~Review~~  
- [ ] ~~Reconcile class diagram with sequence~~

Lifeline \- Graphical representation of flow of control from top to bottom.  
Stack Frame A calls SF B, need second instance SF.

## 2025-07-21 Monday

- [x] ~~Sequence Flow [https://editor.plantuml.com/](https://editor.plantuml.com/)~~  
- [ ] Reconcile with complete class diagram  
- [x] ~~Ninja into build for speed~~

### Sequence flow

Init  
Simulate

1. Time t=0 set state of all aircraft to full charge and flying.  
2. Simulation Effects.  
   1. Fleet-\>Tick(t). Update state aircraft model machine at t.  
      1. Flying: If battery empty, aircraft requests charge and enters charger queue (push).  
      2. Charging: If battery is fully charged, unplug aircraft from charger and start flight.  
   2. ChargerQueue-\>Step().  
      1. If there are free Chargers, pop Aircraft& from Queue.   
         1. Change Aircraft& state to charging.  
         2. Record in statistics.  
   3. Fault Calculation at time t.  
3. Get next event time. Max time step is min(fleet-\>nextEvent). Driven by battery depletion during flight or charge completion during charging.  
4. Set time t to next event time, t'.  
5. If time t \< sim duration of operation window, go to step 2\.  
   1. Else exit loop.  
6. Finalize flight and charge data. Include fractional flights and charges (float).

Report

1. Seed.  
2. \[5+25\] For each AircraftType, the five StatMetrics.

[https://editor.plantuml.com/](https://editor.plantuml.com/)  
encoded [link](https://editor.plantuml.com/uml/bLDDQm8n4BtdLyofxjAj5Wiz21P5B9HMIzNsAY4xkg7PP4bcGVtrJVpS5QNjBPEyD_DctgJdY5igSyLOjmilMfBaIcuuIQEXssNiqw65koUOodpuDNj_Wyxn68qJU8rPg_4R9e4fOXetMXvxn7XyNrfX-O9cwm8zfnf6ckUOm2CKtFBSnN3R1i6LA1KdJ7scl-RfgoBZpMPpA38pC7d1zJfcsX22bSkC_bMMrQeECcwNQ3zAB4FLQXWvkS4vWTZTkPZLm2VoYHTLLNLWzN2WN2zrKzSpAVMKEBa92cDJ3mjk-hHdzy4d7uwGh1Jk0AiEuIPcp4yiB8JF7rhVByDbdxUpYLnmkAQdWh9SVy_9p8Ktm3WPybqWF2j4EWZQJv3lTNbggIbcEqp5yv59KLLStCPHATC4pdggcRvgtn-Rd61XB5rfSFSOBQJ2iTyDFnhMGvs67_C3) v1, [v2](https://editor.plantuml.com/uml/ZLLDKzim4BthL-m9d6c0b4vxe1E6J0gTJWkb8VJA27kJQ90bLrf3mg_loaw2R9B0ARAq-_Rjsu-SUPAEobmBqU_33wD8IQsU9IbheDyNiYHholmUdH0tAZ__E_xz2mvEONq-MH-JA-Z3SQyBF_dtSqSOImXEJMV4HbSWbKkTd118ax49BGhqO2VhUt6qV-lHRGLdt2xS86QT_IEsZSaCbX1ZncIV-3CnCiSkV852Efdx3dmuXbJgjDIIC6Eaz-8CT36JXuU7v-dC3crUIBDefVZ-i4qfXpFffkZ-b5Y6gF5dujKptX6azPtlY8RnY_EBNnApEe7vfvNBzb1xJJwZqjoG93_2rBgCpK9NC6phFjI9Fo-Hd4hzoYmMOQyZnEQILZ8aFXHpMrSvQHxkoDwbhBNrAlJZ1eSBZKYhs4knLWt5hgMXZgXjel96jMhpRHHqVjohfc99U3aHsje2dcPA8oW3_kND5kZgqvCocNsAPWS-zdeRUOzL-f2GoX6s4BqjCfOg28rSWlN3mUjYZl1VYPxgstLUgvmqLd3taWZT0Z0lQB4JxjOKkfo-XLCtPhQZBQzBhQzmJgyKRwjTGIapvTbkeqLwN3T9yG1lbEqxKWXw_eY6nYnl4gaRDTa6imQdd2kSbSkgaWN3ne31Ulish24hDE95IFijJ3K1WbEWNCwxK5dmwIiWRqNcpffLGHGjcqc4Xz1pWfVFxZczK2RiRhpG8LGjuZT9ahCARJp2mZfgBdNUrxtTeBLNDEcCLet0n3f0cSuQU-_D9J7a9JXuvDocw0Sc6uOHzUqfNASS-xuY9pO4gzRDIJInDVsWm1cZLlzp_m40), [v3](https://editor.plantuml.com/uml/ZLNRKjj037tVhz0JEDC0APtsWKuOCYbqEYsKvj9NPh6LP0TxrzsL8U7hgxLpMJiNUF9wBHr9HphobIDfgSXIS0j7c85JMP5AKaQ3mty5wXWXKN9gPIP4jmi_j28bK_LQsNIxGXPaT94zeXLZXnPEBc6eikk_ez-_uC8VFHv63Z5f2R7-j6LM7gCxwC9vfmq_-Vcv9RGX1AkcCsAZEv3AnbPE2AHEs8GMEJema_MzE3lUdSCId77RCFIP79-nRPXCRmanOaps2LyZBJDimnV89TFWMl3X76APncLX2IEz5wUNvZDvUdfw7SzCtsIvr8j6YUyFMwUoFvDsYlPFWOMF6hv6JhtY0q5StRcMg1bld3T-KPZL1Sm_hLpsXpggvpCez90akG76nYPivkU6OHltla_yUejaLUnMPY49H_jdnahzz43c8MOcZLD-7fTJ5ZhSf8YqGbvIiHeNTYqqjKHb4pGlw4GpcrgxvkUTKXN3ZMfO4QanERpCL8gWTAWeawEjJYzA9-Obq0vyx7HsvZrIyLD4AaFOa-WuJtWUF8t81Ay_d6otQk05xQYwNTUrgYd54kvH4g5T06OvBGx2ZNMU5jEtSAgnImuCtNsHfdSufot6csoNK4fFMRbDj82F-tfIBC-Tj7r7ya6ldr7JYEcD0dQ38TjXLiifumudnRAhP42pCQ2tth_hAiWIZNZDqN43Kqs0u18ea_Csb1Py-Wh8EuzpPyxA88gMmoJy1pzpWbVBOHNUAErtDzwaFbH5uZT9agloOpp0t5YgBtNUnft3e9LNe6D6AoKmCHPGnhFQLdjp1VHvnVMUkROfkfvE-bwYhgd2TScXxrPoOaUmylzm4IYsIjypSCMe_2iKuZy0), [v4](https://editor.plantuml.com/uml/ZLPDRzim3BtxL_2q9bZQPWMsGuiK3R9q6BPbMvBk6gWs4mkr9LUYswR3_lieVoHouZOzMPQfnqVoaVAL9M4eJnEmQqkOWfLfdWYIMe75-nnLY119iJ8Y3OB10BugIL8ayhcq6Gm2aPDMUNgB9hYnQE3u4cOo7V-P__mEvsxfyB1Z4QDk46m-xPaLoyu41d3Mxy4tVdxi1aeJWf6hcDXe0aAQq8WbWL0Hcz0wGmjwkTaFJe_QELJWZDk3cMDoTCgsFfbX1J5dJ3xZltQKIB47do0Jd0RRXVTd48eaB0AB6EcjECCaYyN9oSauZFL8fvbGwvqGt-wscSfHBCmApUySS-VLV-rO-OmBWh3SipiUbz9O0eeDuiRxEn06kVwyhyjpKgsQ3gUvcf4WEyLGcuXnd30wtM1dtnM2NtyW6HdQsioFyjtBeZ12tIr8Bq9EbRRI2QwIaN_WEa6a6hcAjTO37yqLiPGctQ2qyohafNkNKID3JsVzGlgpRMkmx1EjCtYCPO8WbTysEaDJhXwbYlIZro3meTzlvJwNuLs7P8f0xKHlieYBxbA9bLqFokSY4POFRMptCZvdit4YCelH5iKs8-vjItzyQPqNhieMSbzWKpSRB9Mx6wmwSmaMf6y54PerO9hH-XM6rudBj6k81Cb3gqCymEL6PKc-EaIYb7_q2erH9VIQI4bhvWPaAwS6ZMQvVhKHglLKDzH5e_TOHFS5ackztMgmWZTjU3Y0boctIUiBqWIVQFo0YfnMEfu2lMPiCMj4bN80KLufdyU6OcD0P_toq2yX7Hhy1QVZXUjLke3YSGJ_cYxaaZzS0gNYgLUQyEe2aEy23eNRg_0fgUg_m7rmRHhmo6roQCfFAdUXLVKkg_fPaE08NTjFCTE6cZSTNr7zryTXUSfpn6Z5o5Xg0oZ2k37g3uxD4S_zuGC7ja8xLD78ZJMxExKsyVfdzyW5BSwAI_FSct0bVPU1AqRb_uCW-0y0), [v5](https://editor.plantuml.com/uml/ZLPHRzem47xthxYd5ZJQiahRGokg8aQdQHlRWEuLkSa1LXCxjIzjQRN_ldCIm04AVOhZdB_xxkwxSwuTIKjvce1REC8Kd4hpH98o6Xm-vAWZX5Z9cPMf49qEVDEAb4pKIsdJwGYPaz5vUeTMt3gqS7858vNs_uv__O0Bl_Hus728SLE8rQSjisBP6407pjij-CxFJqsX3I5ODPiJ6mr0AXjPEIMGEcOJMcJem4nN--BiU3U72fnnMp3oJ8xFs3OaqwqWnep9PyBNXfOfjk0pP9BJu9hmuHmYcKH5O34ZlHUdcsHpUNfwse_cfcVIJEh5HeZlTrjFPMykxGpjdnnpxpLyRJZrWXE2gDnp6nwdoZe2cbl4bVSZa1Qv_hnlodDAp-eEXxaUaIGtnCZOc76yC1fDiR7l2y6lFv6iYjpIB8pow6rHMAdl9sGc4QVAEEK5LyaeF72J8D8IkOfrgGS-ccjYAGsQehGBAXIaUvDHaQ442vldytQX_D6wCrZrYJ4PFCrLWg1qs3KcGrkkdfIEpLFG7_2ntTv9VQoY-mQf585syxpDOgwvpoHMTYqedvD4EZwqijrA-9ZD-edC7CPh55SFkBMk_CcLyrwuA6l8RNqD_MXmLEwki3OIToU9q2u0qumMUnZU95wAlXyIf01j6U81BhSwI_BP8HAb-kCzD7gLpfT4IbeZFnztSghHg9Vhznv2otQwh7KUQ-YX0FADjraCrk-g2G_pVvlnBcLzHHhWC_KVKPENIYCGOD2AEyngKoJb0EAy4ZuF3St6WDxwxP5VGdeqU0KluudlLBg4ud4C_-ekr9G_N06byhbLcl3g4f1l0Ww5kwlmgQXgF-4_-2uLF73h7EhgKzfVPrMvow9-aIGvGj_rGyoCfVeznnTKU_ymB4y5ZXYjc1XJOm5bDAyD-eD3iyTJl_l8WStGTNNSyqFDRSwiLRpXsIroOeUpuigy20PSITzdu9fH-Uz0YFy0), [v6](https://editor.plantuml.com/uml/ZLPHRzis47uNeFzmJusCsKxM0dj8QgE1cmx3kcoBaxqCWy5APuk8HAaaPTSh-jzt96MPb5Ld2E185E_k--wEt_6TqapgfYn0xPN64XGlcu9fNWbG-Bb1aI9ad6qaA-C8ucWsWry4rvmL_5-tRJQB8zReIZJb9vHnzAHGmcGEIrxU_VNu-qUuXeL4fZ5HYDaOqagi-MOKHwBI299lSdtTBi8ClefRBbF9rffUNb-D0SGYPtA3Kj72cx41-BwHRMXuCwKjhBLutDUefgdQVekZoukK2Oe0pwBQWOaCaeaDGYCqBs27zeE0OpWcikERTNLnISnIpRS4lgCJHnsna2IvVrxfQfLMPLqfRW1QniGqjF5Za5svEk1VQHELMBbyMGn9G6xa0VBTn57WT18YssBwrly-JswBEcVJwVGkpQi5uMLY7nI4Z1B1IXp3Jr0pghqQmG-l8ML5Qhe3ipZAy3j8Gh8V2aHjMQxjqu5SnuloHIMXzNigf9Hi_tTOX7_eavVIaUt2BhHr7qT5LTMWAqYjxsCPuyZ5zx9kyv7pjC1JB2RWR3WtdVdaG2gEa3h6HEwYU6aF4jJkDrqPvDWq69ZLWJDpU4meBx-EOMawoUo_l3ZJI_xh-GAseNrVxJdxiy76zMNgBtJ9wixZIMC_D6AfcLOFc5OoiovakDGv8TlURiEUNdz3BNcghB7otdrBWzlx5AOTXljlYJeCcFJ1MJfUqGANJlcMHt4qgcUxRfTpQYNDI_g1jxv-LZMsEhNZ8gjsWNRvcWa_NbqDetxawNFYV8zEktwejv_gp6J9sOnRNAk2ANhfCvxCVqRjPJ8P3UcBt_lz6ZsI-xk2rGep5zXwJuwIPXfnt9OINUc2azJhkGSpSfHsgudEkJh8Ke6MtYUcDSezO5dh_Jba7mfpnipvBL0xLuTil0pZITH5ipaNtnsDx1m2wvEBpG63GxGqy_Oid1CqYnpJvsLL8bc9JUBNoDzftGIcOMVyuL4QAFA0dbkFtdeOBL2oj2Lx0sikbGQTIqJW0ZvR94mUlRwQp6qt65rlhVvF6WSJzptGGoU3UlqUl-YxBGfj-ZOP3MbqP-b9yP1bmBuaejdXKa9ZID1kGBFzH8zyXNTkZKVu2kQ8himSr3TW_rt0jruOlgOlSz0b-p9sU-Zf1imO8ega8pOkrtYGcEE0eddMmn6U5YxCXQxjKLVdzqopn8gY-IFrUy2wahfttwCxsjKBKy3P1JfZqn9mkwUhccgeJNAadvBjYGpTsHXTFjThMg8hWcC2hUwkAmbKlRnt-tXfgYne0jvkAKCRLBSYMvZBXqfy7wz6VkA2GTCdPz6SHhJJ_DgR5Yu3BUrtv9mkt2Qf_m40)

review sequence

1. remove Charger object and use first three spaces in ChargerQueue storage unit for Charging State of Aircraft&.  
   1. delete 104  
2. step 10, aircraft\_count \= N  
3. ~~100 time t elapsed~~  
   1. GetNextEventTime prior to starting Simulation  
   2. 107 SetNextTime() \-- track last\_t to get elapsed t from time t  
4. 101 UpdateState(time t)  
   1. update StatMetrics  
5. ~~108 FinalizeFlightChargeData() \-\> StatMetrics not RunStatsRecord~~  
6.   
7. 

### cmake ninja

mkdir build && cd build  
cmake .. \-G Ninja  
cmake \--build . \-t all  
cmake \--build . \-t test

## 2025-07-20 Sunday

- [x] ~~Diagrams [eVTOL Simulation Design](https://docs.google.com/presentation/d/1pY8Kx5wDcLs08X5Ellb4jyfhD2xArb9rlFCB2N2qXIw/edit?slide=id.g344dd1bd4ce_0_0#slide=id.g344dd1bd4ce_0_0)~~  
- [x] ~~Start CMake project~~  
- [x] ~~Review with Brian~~

1216  
1321  
tech issue  
1530  
Review note:

* Design incomplete but not wrong.  
* Cut to the problem, less reformulation.

## 2025-07-19 Saturday

- [x] ~~Read Problem Statement~~  
- [x] ~~Project Plan~~  
- [x] ~~Digest into Requirement~~  
- [x] ~~System Design Draft~~  
- [x] ~~What is the hardest problem? Biggest risk?~~

### Plan

Up to 2 work days, 16 hours for:

1. Requirements 1 hr  
2. Design 2 hr  
3. Setup Repository  
4. Setup Build System 1 hr  
5. Setup Initial Tests 2 hr  
6. Implementation 4 hr  
7. Simulation   
8. Document Results 1 hr  
9. Release to Pat, Justin, and Team

### System Design

1340

1. Init  
   1. Input rank range 1 to Total Combinations  
   2. Instantiate x1..5 of Alpha...Echo vehicles  
2. Simulate  
   1. 3 hours in 1 second tick  
   2. Simulator controls the clock with a tick message containing the timestamp. The Vehicle object responds with   
   3. Vehicle Model: Start-\>Fly-\>Queue-\>Charge-\>Fly  
      1. 

      ![][image1]

3. Report  
   1. Per vehicle type statistics \[[ref](#3.1.4-statistics-collection)\]  
   2. Log/Cache prior runs

### Greatest Risk

Architecture. Correct by Construction. Relevance to a real-world scenario with independently operating aircraft.

Charge can either pull from queue or Queue can Request a Push to Charger (or know internals of Charge System). Let's Pull from Queue to Charge System, allowing it to assess its own state and use the generic Queue Interface.

Similarly, the Model Aircraft will Unplug itself from the Charger, then go Fly within one Tick.

### GUI

Time permitting, I would like to use imgui to visualize the statistics of each run. Bonus points to scroll through an actual simulation run forward and backward.

[https://traineq.org/implot\_demo/src/implot\_demo.html](https://traineq.org/implot_demo/src/implot_demo.html) of [imPlot](https://github.com/epezent/implot)

### S\&B Range to Coefficients

I want to encode and decode from a range of numbers into the coefficients that add to 20, the combinations of aircraft types which sum to 20\.

*Stars and Bars* \[[1](https://en.wikipedia.org/wiki/Stars_and_bars_\(combinatorics\)#Enumerating_solutions)\]  
C(n+k-1, k-1) \= C(24,4) \= 10,626  
x1+x2+x3+x4+x5 \= 20  
Combinations of 5 types with repetition (each type can be selected more than once) which sum to 20\.

Map 1 to 10,626 to x1..5 coefficients  
*combinatorial number system* or *ranking/unranking integer partitions* or *stars and bars ranking*.

#### Mapping / Encoding: Coefficient \-\> Rank

int rank \= 1, remaining \= 20;  
for (int i \= 0; i \< 5 \- 1; \++i) {  
    for (int a \= 0; a \< x\[i\]; \++a) {  
        rank \+= binomial\_coeff(remaining \- a \+ 5 \- i \- 2, 5 \- i \- 2);  
    }  
    remaining \-= x\[i\];  
}

Where *x* is an array.

#### Inverse Mapping / Decoding: Rank \-\> Coefficient

int rank \= input \- 1;  
int remaining \= 20;  
for (int i \= 0; i \< 5 \- 1; \++i) {  
    x\[i\] \= 0;  
    while (true) {  
        int count \= binomial\_coeff(remaining \- x\[i\] \+ 5 \- i \- 2, 5 \- i \- 2);  
        if (count \> **rank**) break;  
        **rank** \-= count;  
        x\[i\]++;  
    }  
    remaining \-= x\[i\];  
}  
// x\[4\] \= remaining;

1439  
[https://en.cppreference.com/w/cpp/numeric/random/binomial\_distribution.html](https://en.cppreference.com/w/cpp/numeric/random/binomial_distribution.html)

### Why Average when we already know the answer? {#why-average-when-we-already-know-the-answer?}

Reviewed the [*eVTOL Simulation Problem*](https://drive.google.com/file/d/1SPi4VnmAFmv-zx6UiDNhYBel8Qd28zie/view?usp=sharing) and have one question. What does it mean to ***average time charging per charge session*** when ***Time to Charge*** is constant across vehicle types? The same applies to the other average questions for *flight time* and *distance traveled*. These are given and each are constant in time *per vehicle type*.

Options to make the average different from the constant would be to include (1) queue time waiting for the charger to free up or (2) the incomplete sessions interrupted by the stop of the simulation at 3 hours (without normalization; with normalization would bring the average back to the constant). Option (3) might be to add time effects to faults.   
(1) would negatively affect an aircraft's charge time due to things outside of its control. While this can be good in a sense that if resources are this scarce that the metric helps account for this, it is however a surrogate value for things outside of the aircraft's control.  
(2) Is easy to implement and causes changes in the stats which would seem rewarding, at first, if not for the lack of any physical meaning in reality. If we record the operations of a fleet of aircraft and then lose telemetry from an aircraft, the flight didn't end. We stopped seeing it. Same with charging. Simulation is the observation window and not an end unto itself unless that is part of the problem constraint, as the beginning was with all aircraft starting in the airborne state with full battery charge.  
(3) Aircraft are supposed to be double fault resilient, meaning 3 faults to operational failure or "triple redundancy." We could consider 1 or 3 faults here to be the aerospace triple fault and take an aircraft out of operation for the MTTR. This is realistic, physically relevant, and meaningful but currently out of scope. Consider for the future.

- [x] ~~add rationale JHG~~

Pedantically, this averaging is a data aggregation operation across multiple units after independent operation. Another parallel to series transform and aggregation of information, in addition to the battery charger queue. So, oddly, I can calculate the average and know to expect the closed form solution for the fun of it. Or I am missing something.

Do the simulation. Prove and demonstrate the closed form solution (avg(a\_0 a\_1 … a\_i) . With real telemetric data, the averages would be more useful. In the meanwhile we have a nice degenerate test case for the intended outcome that we can check.

## 2025-07-18

Received. PDF opens. Initial read.

# Software Requirements Specification

eVTOL Fleet Simulation

Document Number: EVTL-SWSR-001  
Prepared for: Joby Aviation  
Prepared by: Joe Gorse  
Guidance: DO-178C, Software Considerations in Airborne Systems and Equipment Certification

Changelog:

1. 2025-07-19 1340 Initial Revision JHG.

## 1\. Introduction

### 1.1 Purpose

This document specifies the requirements for the eVTOL Fleet Simulation software problem assigned by Joby to Joe Gorse for the purposes of demonstrating problem-solving methodology. The software models the behavior of a mixed fleet of electric vertical takeoff and landing (eVTOL) vehicles over a three hour window of operation. As Joby is an aerospace company, this specification will loosely adhere to aerospace best practices and align with DO-178C principles.

### 1.2 Scope

The simulation models the follow for five vehicle types:

1. Operational Performance  
2. Energy Management  
3. Charging Infrastructure Usage  
4. Individual Metrics & Fleet Statistics  
5. Fault Occurrence

## 2\. System Overview

The object-oriented simulation shall:

1. Allocate a fleet of 20 eVTOL vehicles among five types, each with specific operation characteristics.  
2. Simulate each vehicle's energy utilization during flight, waiting for charge, and charging over three hours.  
3. Aggregate operation metrics per vehicle type.

## 3\. System Requirements

### 3.1 Functional Requirements

#### 3.1.1 Vehicle Modeling

The software shall define five eVTOL vehicle types, each characterized by:

1. Cruise speed (mph)  
2. Battery capacity (kWh)  
3. Time to charge (hours)  
4. Energy use at cruise (kWh/mile)  
5. Passenger count  
6. Probability of fault per hour

#### 3.1.2 Fleet Initialization

The system shall create a fleet of 20 vehicles, distributed randomly across the five vehicle types.

#### 3.1.3 Simulation Parameters

1. Each simulation run shall cover a three hour operation window.  
2. Each vehicle shall start airbourne at cruise speed with a fully charged battery.  
3. Each vehicle shall operate at cruise speed until battery depletion.  
4. At battery depletion, the vehicle shall queue for one of three chargers.  
5. Charging occurs at the vehicle-type-specific rate with one vehicle charged at a time per charger.  
6. If there is simulation time remaining, the vehicle shall resume flight after completing the charge.  
7. Interrupted flights or charges shall count the partial work normalized to the proportion of total completion.  
8. Faults shall occur at the specified hourly rates and counted per vehicle type.

#### 3.1.4 Statistics Collection {#3.1.4-statistics-collection}

The system shall record, *per vehicle type*:

1. Average flight time per flight  
2. Average distance traveled per flight  
3. Average charging session time  
4. Total number of faults  
5. Total number of passenger-miles, where passenger-miles \= number of vehicles x passenger count x hours flown x cruise speed

#### 3.1.5 Output

The software shall report all the metrics listed in 3.1.4 after each simulation.

### 3.2 Non-functional Requirements

1. Performance: Shall execute in under 60 seconds.  
2. Reliability: Simulations shall be repeatable given a seed number.  
3. Usability: Source code shall be modular, well-documented, follow object-oriented principles, and be unit testable.  
4. Maintainability: The code base shall be organized for clarity and expediency. All assumptions and design decisions not explicitly defined here shall be documented in code comments.

## 4\. Interface requirements

1. No external interfaces are required beyond standard file IO and standard input/output.  
2. The simulation configuration, i.e. the **seed** number, shall be parameterizable at runtime.

## 5\. Notes & Assumptions

1. Vehicles change states instantaneously from Flight to Queue to Charging and back to Flight.  
2. Vehicles operate in flight from full charge until the battery fully depletes.  
3. Charger queuing is First-In, First-Out (FIFO); charging begins upon availability; in the event of two vehicles arriving at the same time, the vehicle instantiation order will be used where the first vehicle has the highest priority.  
   1. TODO: If time permits, consider optimizing for passenger-miles and other targets of interest, observing changes in statistics.  
   2. Rank (Seed) \-\> Coefficient (Vehicle Types) ordering used at instantiation will determine Queuing priority during a same-time collision. While introducing entropy into the instation ordering may have some real-world effects it would seem more likely that a Queue priority would first depend upon *vehicle type* and then other factors. This is sufficient for the initial simulation and satisfies the requirement if the Seed is randomized.  
4. Faults are independent events. Faults occur during any state equally: flight, waiting in queue, and charging.  
   1. TODO: Time permitting, if more than two faults occur during a period of operation for a vehicle, ground the plane, reset to empty charge, and place the end of the queue. Aviation is double-fault tolerant so we should expect a triple fault to have consequences.  
5. Metrics are aggregated only for the period the simulation is operated. However, partial flights or charges will be normalized to the percent completion achieved in the given time. See [reference above](#why-average-when-we-already-know-the-answer?).  
6. Pseudo-Random Number Generation (PRNG) will be used to allow for deterministic, reproducible simulations that guarantee the outcome with the frequency of the phenomena during the simulation. For example, if the probability of fault is 1 per hour, in 3 hours we will expect 3 from the PRNG each time vs 0-MANY from RNG.  
   1. TODO: Is this physically appropriate? How might it negatively affect the fidelity of simulation?  
7. TODO: Consider variable timestep, event-based approach. The simulator would need to know the next time of interest for each vehicle in the fleet. Consider calculating and returning this upon receiving a Tick of the Current Time from the Simulator.

## 6\. Verification & Validation

1. The software shall provide verifiable output matching the statistical definitions in section 3.1.4 Statistics Collection.  
2. Unit Test code shall verify:  
   1. Correct calculation of flight and charge cycles.  
   2. Proper queuing and allocation of chargers.  
   3. Accurate fault simulation.  
3. Output results from at least one simulation run shall be included for verification.

## 7\. Change Management

1. Modifications shall be tracked via git version control with commit annotations explaining significant changes.

[image1]: <data:image/png;base64,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>