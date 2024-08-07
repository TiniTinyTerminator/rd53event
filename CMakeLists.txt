cmake_minimum_required(VERSION 3.25)

# Set the project name
project(RD53)

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
add_library(RD53 STATIC
    ${SRC}/RD53Event.cpp
    ${SRC}/RD53Decoder.cpp
)

set_target_properties(RD53 PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

add_subdirectory(python)
