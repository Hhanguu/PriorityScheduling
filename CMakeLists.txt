cmake_minimum_required(VERSION 3.20)
project(PriorityScheduling_Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Cấu hình Unit Test
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)

# Tự động tìm TẤT CẢ file test trong thư mục tests/unit/
file(GLOB_RECURSE UNIT_TEST_SOURCES "tests/unit/*.cpp")

# Build file test tổng hợp
add_executable(all_tests
    ${UNIT_TEST_SOURCES}
    src/core/PreemptiveScheduler.cpp
    src/core/Metrics.cpp
)

target_include_directories(all_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)

add_test(NAME AllTests COMMAND all_tests)