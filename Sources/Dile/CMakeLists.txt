add_library(Dile Dile.cpp)
target_include_directories(Dile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(Dile PUBLIC cxx_std_17)
