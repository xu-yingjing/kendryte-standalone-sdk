# DO NOT MODIFY THIS FILE, IT WILL BE OVERRIDE!!!

# set this will supress some warnings
set(BUILDING_SDK "yes" CACHE INTERNAL "")

# basic config
if (NOT DEFINED BOARD)
    message(FATAL_ERROR "BOARD not set, set it then configure. e.g. -DBOARD=CanMV-K210")
else ()
    message("BOARD = ${BOARD}")
endif ()
if (NOT DEFINED PROJ)
    get_filename_component(PROJ ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)
    get_filename_component(PROJ ${PROJ} NAME)
    string(REPLACE " " "_" PROJ ${PROJ})
    message(STATUS "PROJ not set, use ${PROJ} as PROJ. Also, you can set it manually. e.g. -DPROJ=hello_world")
else ()
    message("PROJ = ${PROJ}")
endif ()
if (NOT DEFINED USENNCASE)
    set(USENNCASE "1")
    message(STATUS "USENNCASE not set, use ${USENNCASE} as USENNCASE.")
else ()
    message("USENNCASE = ${USENNCASE}")
endif ()
cmake_minimum_required(VERSION 3.8)
include(./cmake/common.cmake)
project(${PROJ} C CXX ASM)

# config self use headers
include(./cmake/macros.internal.cmake)
header_directories(${SDK_ROOT}/lib)
header_directories(src/${PROJ})
header_directories(src/BSP/${BOARD}/include)

add_subdirectory(third_party/gsl-lite)
add_subdirectory(third_party/mpark-variant)
add_subdirectory(third_party/nlohmann_json)
# build library first
add_subdirectory(lib)

# compile project
add_source_files(src/${PROJ}/*.c src/${PROJ}/*.s src/${PROJ}/*.S src/${PROJ}/*.cpp)
add_source_files(src/BSP/${BOARD}/*.c)
include(./cmake/executable.cmake)

