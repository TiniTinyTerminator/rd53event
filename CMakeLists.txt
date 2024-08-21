cmake_minimum_required(VERSION 3.20)

# Set the project name
project(RD53BEvent)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include CMake modules
include(CTest)

# Set include and source directories
set(INC ${CMAKE_SOURCE_DIR}/inc)
set(SRC ${CMAKE_SOURCE_DIR}/src)

include_directories(${INC})

# Add the RD53 library
add_library(RD53BEvent STATIC
    ${SRC}/Quartercore.cpp
    ${SRC}/Event.cpp
    ${SRC}/Decoder.cpp
    ${SRC}/utils.cpp
)

set_target_properties(RD53BEvent PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

add_subdirectory(python)

add_executable(test_rd53 ${CMAKE_SOURCE_DIR}/test/test.cpp)

target_link_libraries(test_rd53 RD53BEvent ${CMAKE_THREAD_LIBS_INIT})

add_test(NAME test_rd53 COMMAND $<TARGET_FILE:test_rd53>)
