cmake_minimum_required (VERSION 3.20)
include_guard(GLOBAL)

#make visual studio like filters so that we have a nice file structure in ide
#SourcePath: path to start looking for sources
#SourceList: return value with list of all sources
function(ERS_Add_Filters sourcePath sourceList)
    file(GLOB_RECURSE _SourceList ${sourcePath}/*.cpp* ${sourcePath}/*.h* ${sourcePath}/*.hpp*)
    source_group(TREE ${sourcePath} FILES ${_SourceList})
    foreach(source IN ITEMS ${_SourceList})
        get_filename_component(_source_path "${source}" PATH)
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
    endforeach()
    set(${sourceList} ${_SourceList} PARENT_SCOPE)
endfunction(ERS_Add_Filters)