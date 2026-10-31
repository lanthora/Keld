cmake_minimum_required (VERSION 3.18.4)

set(KELD keld)
project(${KELD})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
aux_source_directory(. DIR_SRCS)
add_executable(${KELD} ${DIR_SRCS})

set(CMAKE_C_FLAGS "-fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++20 -Wall -Werror")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
add_definitions(-DDEBUG)
else()
add_custom_command(TARGET ${KELD} POST_BUILD COMMAND ${CMAKE_STRIP} ${KELD})
endif()

include_directories("/usr/include/libnl3")

add_subdirectory(nlc)
add_subdirectory(heartbeat)
add_subdirectory(file)
add_subdirectory(process)
add_subdirectory(net)
add_subdirectory(ipc)
add_subdirectory(util)
add_subdirectory(dispatcher)

target_link_libraries(${KELD} nlc)
target_link_libraries(${KELD} heartbeat)
target_link_libraries(${KELD} file)
target_link_libraries(${KELD} process)
target_link_libraries(${KELD} net)
target_link_libraries(${KELD} ipc)
target_link_libraries(${KELD} util)
target_link_libraries(${KELD} dispatcher)

target_link_libraries(${KELD} nl-3)
target_link_libraries(${KELD} nl-genl-3)
target_link_libraries(${KELD} pthread)
