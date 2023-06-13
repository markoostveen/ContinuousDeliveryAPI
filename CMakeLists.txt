cmake_minimum_required(VERSION 3.20)


macro(ERSRunVCPKG)
    # Configure vcpkg toolchain
    if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        if(DEFINED ENV{VCPKG_ROOT})
            set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
        endif()
    endif()

    option(ERS_USE_DEFAULT_VCPKG_INSTALL_DIR "Prevent reinstalling of packages of vcpkg after deleting build directory" OFF)

    if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        message(ERROR " unable to find vcpkg")
    endif()

    if(ERS_USE_DEFAULT_VCPKG_INSTALL_DIR)
        if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED VCPKG_INSTALLED_DIR)
            set(VCPKG_INSTALLED_DIR "$ENV{VCPKG_ROOT}/installed")
        endif()
    endif()

    # if("${ERS_TRIPLET}" STREQUAL "")
    # #    message(ERROR " Please change ERS_TRIPLET to correct vcpkg triplet to use")
    # endif()
    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
    elseif(UNIX AND NOT APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
    else()
        message(ERROR " Please change ERS_TRIPLET to correct vcpkg triplet to use")
    endif()

    include(${CMAKE_TOOLCHAIN_FILE})
endmacro()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

ERSRunVCPKG()

project(cdn VERSION 0.0.1)

add_subdirectory(FileHashes)
add_subdirectory(ContinuousDeliveryAPI)
add_subdirectory(ContinuousDeliveryClient)