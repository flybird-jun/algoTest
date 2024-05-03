cmake_minimum_required(VERSION 3.22)
project(algo)

option(BTREE "compiler btree" ON)

#add_definitions("-Wall -g")
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -ggdb  -Wall  -mavx2 -maes -msse -mssse3 -mavx")
aux_source_directory(log SRC_LIST)
if(BTREE)
    aux_source_directory(btree BTREE_LIST)
    set(SRC_LIST ${SRC_LIST} ${BTREE_LIST})
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g")

add_library(algo SHARED ${SRC_LIST})
target_include_directories(algo PUBLIC include)
install(DIRECTORY include/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../out/include)