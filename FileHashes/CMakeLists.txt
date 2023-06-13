cmake_minimum_required(VERSION 3.20)

include(${CMAKE_SOURCE_DIR}/CMake/Filters.cmake)

set(SourcePath "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SourceList "null")
ERS_Add_Filters(${SourcePath} SourceList) # returns the SourceList with sources

add_library(FileHashes STATIC ${SourceList})
target_include_directories(FileHashes PUBLIC ${SourcePath})

find_package(OpenSSL REQUIRED)
target_link_libraries(FileHashes PUBLIC OpenSSL::SSL OpenSSL::Crypto)