
cmake_minimum_required(VERSION 3.21)

project(PriorityScheduling
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(PriorityScheduling
    src/main.cpp

    src/core/Metrics.cpp
    src/core/NonPreemptiveScheduler.cpp
    src/core/PreemptiveScheduler.cpp

    src/io/CsvHandler.cpp

    src/ui/GanttChart.cpp
    src/ui/MainWindow.cpp
)

target_include_directories(PriorityScheduling PRIVATE
    src/core
    src/io
    src/ui
)

target_link_libraries(PriorityScheduling PRIVATE
    Qt6::Widgets
)
