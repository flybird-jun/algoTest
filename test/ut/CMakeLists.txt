cmake_minimum_required(VERSION 3.22)
project(UtTest)

# googletest 个人仓 git@github.com:flybird-jun/googletest.git
add_subdirectory(googletest)
add_library(common common/gtest_main.cc)
target_link_libraries(common gtest)

option(UT_BTREE "btree test" ON)

if (UT_BTREE)
    aux_source_directory(btree UT_SRC_LIST)
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")
add_executable(ut_test ${UT_SRC_LIST})
target_include_directories(ut_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../out/include)

MESSAGE(WARNING "include ${CMAKE_CURRENT_SOURCE_DIR}/../out/include")

target_link_directories(ut_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../out/bin)
target_link_libraries(ut_test PUBLIC common PUBLIC algo)