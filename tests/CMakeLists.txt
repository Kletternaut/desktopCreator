cmake_minimum_required(VERSION 3.16)

find_package(Qt5 REQUIRED COMPONENTS Core Test)

set(CMAKE_AUTOMOC ON)

add_executable(test_desktopentry
    test_desktopentry.cpp
    ../src/desktopentry.cpp
)

target_include_directories(test_desktopentry PRIVATE ../src)

target_link_libraries(test_desktopentry
    PRIVATE
    Qt5::Core
    Qt5::Test
)

add_test(NAME test_desktopentry COMMAND test_desktopentry)
