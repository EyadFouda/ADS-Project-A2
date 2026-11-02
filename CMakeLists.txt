cmake_minimum_required(VERSION 3.16)
project(ADS-Project-A2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/CircularQueue.cpp
    src/FileParser.cpp
    src/HashTable.cpp
    src/Heap.cpp
    src/Trie.cpp
)

add_executable(search_engine ${SOURCES})
