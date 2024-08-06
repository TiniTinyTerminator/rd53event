cmake_minimum_required(VERSION 3.10)

# Set the project name
project(RD53)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set include and source directories
set(INC ${CMAKE_SOURCE_DIR}/inc)
set(SRC ${CMAKE_SOURCE_DIR}/src)

# Add the header files
include_directories(${INC})

# Add the source files
add_library(RD53Event STATIC
    ${SRC}/RD53Event.cpp
    ${SRC}/RD53Decoder.cpp
)

# Find SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Set the language for SWIG (e.g., Python)
set(SWIG_LANGUAGE python)

# Create SWIG interface
set(SWIG_INTERFACE ${CMAKE_SOURCE_DIR}/swig/RD53.i)

# Add SWIG module
swig_add_library(RD53Event_swig
    LANGUAGE ${SWIG_LANGUAGE}
    SOURCES ${SWIG_INTERFACE}
            ${SRC}/RD53Event.cpp
            ${SRC}/RD53Decoder.cpp
)
