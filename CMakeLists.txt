cmake_minimum_required(VERSION 3.25)

# Set the project name
project(RD53)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include CMake modules
include(CTest)
include(UseSWIG)

# Find packages
find_package(SWIG REQUIRED)
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Set include and source directories
set(INC ${CMAKE_SOURCE_DIR}/inc)
set(SRC ${CMAKE_SOURCE_DIR}/src)
include_directories(${INC} ${Python_INCLUDE_DIRS})

# Add the RD53 library
add_library(RD53 STATIC
    ${SRC}/RD53Event.cpp
    ${SRC}/RD53Decoder.cpp
)

# Set the language for SWIG and create the interface
set(SWIG_LANGUAGE python)
set(SWIG_INTERFACE ${CMAKE_SOURCE_DIR}/swig/RD53.i)
set_property(SOURCE ${SWIG_INTERFACE} PROPERTY CPLUSPLUS ON)

# Add SWIG module with documentation enabled
set(CMAKE_SWIG_FLAGS -doxygen)
swig_add_library(RD53py
    LANGUAGE ${SWIG_LANGUAGE}
    SOURCES ${SWIG_INTERFACE}
            ${SRC}/RD53Event.cpp
            ${SRC}/RD53Decoder.cpp
)

# Enable testing
enable_testing()
add_test(NAME PythonBindingsTest COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/test.py)
