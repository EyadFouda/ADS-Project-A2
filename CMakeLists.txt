cmake_minimum_required(VERSION 3.16)
project(ADS-Project-A2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Data structures shared by the console app and the GUI.
set(CORE_SOURCES
    src/CircularQueue.cpp
    src/FileParser.cpp
    src/HashTable.cpp
    src/Heap.cpp
    src/Trie.cpp
)

# Console version (Windows only, uses conio.h).
add_executable(search_engine src/main.cpp ${CORE_SOURCES})

# Qt GUI version, built only if Qt 6 is installed.
find_package(Qt6 QUIET COMPONENTS Widgets)

if(Qt6_FOUND)
    add_executable(search_engine_gui WIN32
        gui/main.cpp
        gui/mainwindow.cpp
        gui/mainwindow.h
        gui/mainwindow.ui
        ${CORE_SOURCES}
    )

    set_target_properties(search_engine_gui PROPERTIES AUTOMOC ON AUTOUIC ON)
    target_include_directories(search_engine_gui PRIVATE gui)
    target_link_libraries(search_engine_gui PRIVATE Qt6::Widgets)

    # Put dictionary.txt next to the GUI executable.
    add_custom_command(TARGET search_engine_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_SOURCE_DIR}/data/dictionary.txt
                $<TARGET_FILE_DIR:search_engine_gui>/dictionary.txt
    )
else()
    message(STATUS "Qt 6 not found: building the console version only.")
endif()
