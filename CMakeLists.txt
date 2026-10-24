cmake_minimum_required(VERSION 3.20)
project(PriorityScheduling LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Tìm Qt6
find_package(Qt6 QUIET COMPONENTS Widgets)

set(CORE_SOURCES
    src/core/PreemptiveScheduler.cpp
    src/core/NonPreemptiveScheduler.cpp
    src/core/Metrics.cpp
    src/io/CsvHandler.cpp
)

if(Qt6_FOUND)
    add_executable(PriorityScheduling
        src/main.cpp
        src/ui/MainWindow.cpp
        src/ui/GanttChart.cpp
        ${CORE_SOURCES}
    )
    target_link_libraries(PriorityScheduling PRIVATE Qt6::Widgets)
    target_include_directories(PriorityScheduling PRIVATE src)
endif()

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)

# Tự động tìm TẤT CẢ file .cpp trong thư mục tests/unit/
file(GLOB_RECURSE UNIT_TEST_SOURCES "tests/unit/*.cpp")

add_executable(all_tests
    ${UNIT_TEST_SOURCES}
    ${CORE_SOURCES}
)

# 🔥 DÒNG QUAN TRỌNG NHẤT: Khai báo đường dẫn thư mục data cho các file test của Leader
target_compile_definitions(all_tests PRIVATE PS_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")

target_include_directories(all_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)

add_test(NAME AllTests COMMAND all_tests)