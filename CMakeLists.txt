cmake_minimum_required(VERSION 3.10)
project(TicketSystem)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(DIRECTORY
    src/include
    src/include/index
    src/include/IO
    src/include/item
    src/include/mySTL
    src/include/storage
    src/include/system
)

set(SOURCES_USER
    src/system/user_system.cpp
    src/system/core_system.cpp
    src/item/user.cpp
)
add_library(user_system ${SOURCES_USER})
target_include_directories(user_system PUBLIC ${DIRECTORY})

set(SOURCES_TRAIN
    src/system/train_system.cpp
    src/system/core_system.cpp
    src/item/user.cpp
    src/item/train.cpp
)
add_library(train_system ${SOURCES_TRAIN})
target_include_directories(train_system PUBLIC ${DIRECTORY})

set(SOURCES_MAIN 
    main/ioput.cpp 
    src/IO/commandparser.cpp
    src/IO/tokenscanner.cpp
)
add_executable(code ${SOURCES_MAIN})
target_link_libraries(code PRIVATE user_system train_system)
