cmake_minimum_required(VERSION 3.20)

include(${CMAKE_SOURCE_DIR}/CMake/Filters.cmake)

set(SourcePath "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SourceList "null")
ERS_Add_Filters(${SourcePath} SourceList) # returns the SourceList with sources

add_executable(Client ${SourceList})
target_include_directories(Client PUBLIC ${SourcePath})
target_link_libraries(Client PUBLIC FileHashes)

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(Client PUBLIC yaml-cpp)