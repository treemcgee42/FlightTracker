add_library(Dile Dile.cpp)
target_include_directories(Dile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(Dile PUBLIC cxx_std_17)

find_package(spdlog REQUIRED)
target_link_libraries(Dile PRIVATE spdlog::spdlog)

find_package(doctest REQUIRED)
add_executable(dile_test DileTest.cpp)
target_link_libraries(dile_test doctest::doctest Dile)
