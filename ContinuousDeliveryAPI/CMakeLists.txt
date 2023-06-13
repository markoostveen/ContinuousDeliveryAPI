cmake_minimum_required(VERSION 3.20)

include(${CMAKE_SOURCE_DIR}/CMake/Filters.cmake)

set(SourcePath "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SourceList "null")
ERS_Add_Filters(${SourcePath} SourceList) # returns the SourceList with sources

add_executable(Server ${SourceList})
target_include_directories(Server PUBLIC ${SourcePath})
target_link_libraries(Server PUBLIC FileHashes)

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(Server PUBLIC yaml-cpp)