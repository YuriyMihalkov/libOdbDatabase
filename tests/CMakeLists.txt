cmake_minimum_required(VERSION 3.16)
project(tests)

set(COMMON_LIBS
    GTest::gtest
    GTest::gtest_main
)

add_test_exe(tests_main
    LABELS unit
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests_main.cpp
    LIBS OdbDatabase
)