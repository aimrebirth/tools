#
# A.I.M. Tools
#

cmake_minimum_required(VERSION 2.8)

set(output_dir ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${output_dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

project(aim_tools)

find_package(BISON 3.0 REQUIRED)
find_package(FLEX 2.5.35 REQUIRED)
find_package(Git)

if (MSVC)
    set(disable_msvc_warnings "/W1 /wd4996")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP ${disable_msvc_warnings}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ${disable_msvc_warnings}")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14")
endif(MSVC)

add_definitions(-DPOLYGON4_TOOLS=1)
add_definitions(-DPOLYGON4_STATIC=1)

if (NOT DATA_MANAGER_DIR)
    message(FATAL_ERROR "DATA_MANAGER_DIR variable is not set!")
else()
    set(SOLUTION_FOLDER Extern)
    add_subdirectory(${DATA_MANAGER_DIR} DataManager)
    include_directories(${DATA_MANAGER_DIR}/include)
    include_directories(${DATA_MANAGER_ADDITIONAL_INCLUDES})
endif()

if (Git_FOUND)
    add_custom_target(version ALL
        COMMAND git rev-list HEAD --count > ${CMAKE_CURRENT_BINARY_DIR}/version.h_
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/version.h_ ${CMAKE_CURRENT_BINARY_DIR}/version.h.in
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(version PROPERTIES FOLDER Helpers)
else()
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.h.in "0")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
