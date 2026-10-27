cmake_minimum_required(VERSION 3.21)

project(PriorityScheduling
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Cờ cảnh báo theo compiler (MSVC không hiểu -Wall/-Wextra)
if(MSVC)
    set(PS_WARNINGS /W4 /permissive-)
else()
    set(PS_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Logic dùng chung: không phụ thuộc Qt, test được độc lập.
# Include duy nhất: src/  ->  dùng "core/...", "io/...", "ui/...", "benchmark/..."
# ---------------------------------------------------------------------------
add_library(ps_core STATIC
    src/core/Metrics.cpp
    src/core/NonPreemptiveScheduler.cpp
    src/core/PreemptiveScheduler.cpp
    src/io/CsvHandler.cpp
)

target_include_directories(ps_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(ps_core PRIVATE ${PS_WARNINGS})

# Core không được phụ thuộc Qt -> tắt moc để lỗi phụ thuộc Qt lộ ra ngay
set_target_properties(ps_core PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

# ---------------------------------------------------------------------------
# Ứng dụng GUI
# ---------------------------------------------------------------------------
add_executable(PriorityScheduling
    src/main.cpp
    src/ui/GanttChart.cpp
    src/ui/MainWindow.cpp
)

target_link_libraries(PriorityScheduling PRIVATE ps_core Qt6::Widgets)
target_compile_options(PriorityScheduling PRIVATE ${PS_WARNINGS})

# ---------------------------------------------------------------------------
# Sinh dữ liệu benchmark: ./build/generate_perf [--out samples] [--sizes ...]
# ---------------------------------------------------------------------------
add_executable(generate_perf tools/generate_perf.cpp)
target_compile_options(generate_perf PRIVATE ${PS_WARNINGS})
set_target_properties(generate_perf PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

# ---------------------------------------------------------------------------
# Kiểm thử (tự nhận file mới, không cần sửa CMake khi thêm test)
# ---------------------------------------------------------------------------
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()

    file(GLOB_RECURSE UNIT_TEST_SOURCES CONFIGURE_DEPENDS tests/unit/*.cpp)
    add_executable(unit_tests ${UNIT_TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE ps_core)
    target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)
    target_compile_definitions(unit_tests PRIVATE
        PS_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
    target_compile_options(unit_tests PRIVATE ${PS_WARNINGS})
    set_target_properties(unit_tests PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    add_test(NAME unit_tests COMMAND unit_tests)

    file(GLOB INTEGRATION_TEST_SOURCES CONFIGURE_DEPENDS tests/integration/*.cpp)
    if(INTEGRATION_TEST_SOURCES)
        add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
        target_link_libraries(integration_tests PRIVATE ps_core)
        target_include_directories(integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)
        target_compile_definitions(integration_tests PRIVATE
            PS_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
        target_compile_options(integration_tests PRIVATE ${PS_WARNINGS})
        set_target_properties(integration_tests PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
        add_test(NAME integration_tests COMMAND integration_tests)
    endif()

    add_executable(gui_tests
        tests/gui/doctest_main.cpp
        tests/gui/gui_test.cpp
        src/ui/GanttChart.cpp
        src/ui/MainWindow.cpp
    )
    target_link_libraries(gui_tests PRIVATE ps_core Qt6::Widgets)
    target_include_directories(gui_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)
    target_compile_definitions(gui_tests PRIVATE
        PS_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
    target_compile_options(gui_tests PRIVATE ${PS_WARNINGS})
    add_test(NAME gui_tests COMMAND gui_tests)
    set_tests_properties(gui_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
