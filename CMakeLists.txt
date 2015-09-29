cmake_minimum_required(VERSION 2.8)

set(output_dir ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${output_dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

project(aim_tools)

find_package(Git)

if (MSVC)
    set(disable_msvc_warnings "/W1 /wd4996")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP ${disable_msvc_warnings}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ${disable_msvc_warnings}")
    
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14")
endif(MSVC)

if (NOT DATABASE_MANAGER_DIR)
    message(STATUS "DATABASE_MANAGER_DIR variable is not set! Some targets won't be built")
else()
    add_subdirectory(${DATABASE_MANAGER_DIR} DatabaseManager)
    include_directories(${DATABASE_MANAGER_DIR}/include)
    set_target_properties(sqlite3 PROPERTIES FOLDER Extern)
    set_target_properties(DatabaseManager PROPERTIES FOLDER Extern)
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