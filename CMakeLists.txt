cmake_minimum_required(VERSION 3.10)
project(TicketSystem)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(
    src
    src/storage
    src/index
    test
)

# 查找所有源文件（根据需要调整）
set(TEST_SOURCES 
    test/bpm_test.cpp
    src/index/b_plus_tree.hpp
)

add_executable(bpm_test ${TEST_SOURCES})