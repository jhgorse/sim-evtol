cmake_minimum_required(VERSION 3.10.0)
project(evtol-simulation VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)

add_executable(evtol-simulation src/main.cpp)
target_link_libraries(evtol-simulation evtol-simulation-lib)

include_directories(src)

set(CMAKE_BUILD_TYPE "Debug")

include(CTest)
enable_testing()

add_library(evtol-simulation-lib 
    src/sim_evtol.cpp
)

add_executable(test_suite test/test_suite.cpp)
target_link_libraries(test_suite evtol-simulation-lib)
add_test(NAME Tests COMMAND test_suite)

