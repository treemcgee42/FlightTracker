cmake_minimum_required(VERSION 3.10)
project(FlightTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Trinkets
include_directories(/Users/ogmalladii/dev/Trinkets/Cpp/Include)
link_directories(/Users/ogmalladii/dev/Trinkets/Cpp/Lib)

# Fmt, Raylib
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

# --- Janet -------------------------------------------------------------------------

add_library(janet SHARED Sources/Janet/janet.c)
target_include_directories(janet PUBLIC Sources/Janet)

set_property(TARGET janet PROPERTY C_STANDARD 99)
set_property(TARGET janet PROPERTY C_STANDARD_REQUIRED ON)
target_compile_options(janet PRIVATE -Wall -Werror -O2 -fPIC)
target_link_libraries(janet
    m
    dl
    # linux may need this:
    # rt
    pthread
)

# --- Main project ------------------------------------------------------------------

find_package(nlohmann_json 3.12.0 REQUIRED)

set(SOURCES Sources/FlightData.cpp
            Sources/Main.cpp
            Sources/Layout.cpp)

add_executable(flight_tracker ${SOURCES})
target_link_libraries(flight_tracker
    fmt
    janet
    nlohmann_json::nlohmann_json
    raylib
)
