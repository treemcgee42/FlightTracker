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

set(FTL_SOURCES Sources/FlightData.cpp
                Sources/Layout.cpp)
add_library(ftl ${FTL_SOURCES})
target_link_libraries(ftl fmt
                          janet
                          nlohmann_json::nlohmann_json
                          raylib)

find_package(doctest REQUIRED)
set(FTL_TESTS Sources/LayoutTest.cpp)
add_executable(ftl_test ${FTL_TESTS}
                        Sources/Test.cpp)
target_link_libraries(ftl_test doctest::doctest
                               ftl)

add_executable(flight_tracker Sources/Main.cpp)
target_link_libraries(flight_tracker ftl)
