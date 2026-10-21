cmake_minimum_required(VERSION 3.20)
project(PriorityScheduling_Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Khai báo thư mục chứa header
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/third_party)

enable_testing()

# 2. TỰ ĐỘNG tìm tất cả các file .cpp trong thư mục tests/unit/
# Nhờ dòng này, bạn tạo file test nào nó cũng tự nhận, không sợ sai đường dẫn
file(GLOB_RECURSE UNIT_TEST_SOURCES "tests/unit/*.cpp")

# 3. Tạo file thực thi
add_executable(test_preemptive
    ${UNIT_TEST_SOURCES}
    src/core/PreemptiveScheduler.cpp
)

# 4. Đăng ký test
add_test(NAME PreemptiveSchedulerTest COMMAND test_preemptive)