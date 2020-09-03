cmake_minimum_required(VERSION 3.17)
project(bench LANGUAGES CXX VERSION 0.1.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(BENCH_SRCS src/bench.cpp)




add_library(bench STATIC ${BENCH_SRCS})
target_include_directories(bench PUBLIC include)
set_property(TARGET bench PROPERTY CXX_STANDARD 11)
set_property(TARGET bench PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET bench PROPERTY CXX_STANDARD_REQUIRED ON)

find_package(MPI)
if (MPI_FOUND)
    target_compile_definitions(bench PUBLIC -DBENCH_USE_MPI)
    target_link_libraries(bench PUBLIC MPI::MPI_CXX)
endif()

add_executable(bench-allreduce bin/allreduce.cpp)
target_link_libraries(bench-allreduce bench)

add_executable(bench-pingpong bin/pingpong.cpp)
target_link_libraries(bench-pingpong bench)

add_executable(bench-empty bin/empty.cpp)
target_link_libraries(bench-empty bench)