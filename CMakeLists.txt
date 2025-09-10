cmake_minimum_required(VERSION 3.20)

# Project setup with modern C++ features
project(VeloxDB
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "High-Performance Custom Database Engine"
)

# Only allow in-source builds in specific cases
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# C++ standard and features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Global compile options
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
endif()

# Include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/expected/include
)

# Find required packages
find_package(Threads REQUIRED)

# Option for building tests
option(BUILD_TESTING "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Add third-party dependencies
include(FetchContent)

# Google Test
if(BUILD_TESTING)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
  )
  FetchContent_MakeAvailable(googletest)
endif()

# Google Benchmark
if(BUILD_BENCHMARKS)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(benchmark)
endif()

# Expected library (C++23 std::expected backport)
FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected.git
  GIT_TAG master
)
FetchContent_MakeAvailable(expected)

# fmt library for modern formatting
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.1.1
)
FetchContent_MakeAvailable(fmt)

# spdlog for logging
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Source files
file(GLOB_RECURSE VELOX_SOURCES
  cpp/src/storage/*.cpp
  cpp/src/storage/*.cc
  cpp/src/index/*.cpp
  cpp/src/index/*.cc
  cpp/src/buffer/*.cpp
  cpp/src/buffer/*.cc
  cpp/src/utils/*.cpp
  cpp/src/utils/*.cc
)

# Create main library
add_library(velox_core STATIC ${VELOX_SOURCES})

# Set target properties
set_target_properties(velox_core PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Link libraries
target_link_libraries(velox_core
  PUBLIC
  Threads::Threads
  fmt::fmt
  spdlog::spdlog
  tl::expected
  PRIVATE
  $<$<CONFIG:Debug>:-fsanitize=address>
  $<$<CONFIG:Debug>:-fsanitize=undefined>
)

# Include directories for the library
target_include_directories(velox_core
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/cpp/src
)

# Shared library for FFI
add_library(velox_core_shared SHARED ${VELOX_SOURCES})
set_target_properties(velox_core_shared PROPERTIES
  OUTPUT_NAME velox_core
  SOVERSION 1
  VERSION ${PROJECT_VERSION}
  CXX_STANDARD 20
)

target_link_libraries(velox_core_shared
  PUBLIC
  Threads::Threads
  fmt::fmt
  spdlog::spdlog
  tl::expected
)

target_include_directories(velox_core_shared
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp/include>
  $<INSTALL_INTERFACE:include>
)

# Enable testing
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(cpp/tests)
endif()

# Enable benchmarks
if(BUILD_BENCHMARKS)
  add_subdirectory(cpp/benchmarks)
endif()

# Examples
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Installation
install(TARGETS velox_core velox_core_shared
  EXPORT VeloxDBTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY cpp/include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

install(EXPORT VeloxDBTargets
  FILE VeloxDBTargets.cmake
  NAMESPACE VeloxDB::
  DESTINATION lib/cmake/VeloxDB
)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CPack configuration
set(CPACK_PACKAGE_NAME "VeloxDB")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-Performance Custom Database Engine")
include(CPack)