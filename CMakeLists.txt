cmake_minimum_required(VERSION 3.10)
project(TicketSystem)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)


# 包含目录
include_directories(
    src
    src/index 
    src/IO 
    src/item
    src/key_hash 
    src/mySTL 
    src/storage
    src/system    
    main
)

# 查找所有源文件（根据需要调整）
set(TEST_SOURCES 
    main/ioput.cpp 
    src/item/user.cpp
    src/item/train.cpp
    src/IO/commandparser.cpp
    src/IO/tokenscanner.cpp
    src/system/core_system.cpp
    src/system/user_system.cpp
    src/system/train_system.cpp
)

add_executable(code ${TEST_SOURCES})