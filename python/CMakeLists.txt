# CMakeLists.txt

cmake_minimum_required(VERSION 3.4...3.18)
project(RD53Eventpy)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pybind11 and Python packages
find_package(pybind11 REQUIRED)
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Include directories
include_directories(${Python_INCLUDE_DIRS})

# Set source and include directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${INC_DIR})

# Set the module name
set(MODULE_NAME RD53Eventpy)

# Source files for the module
set(SRC_FILES
    ${SRC_DIR}/Quartercore.cpp
    ${SRC_DIR}/Event.cpp
    ${SRC_DIR}/Decoder.cpp
    ${SRC_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings.cc  # The pybind11 binding code
)

# Add the pybind11 module
pybind11_add_module(${MODULE_NAME} MODULE ${SRC_FILES})

# Set the output directory for the Python module
set_target_properties(${MODULE_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Enable testing
enable_testing()
add_test(NAME PythonBindingsTest
    COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/test.py
)

# Set the local RD53Event package directory
set(LOCAL_RD53PY ${CMAKE_SOURCE_DIR}/RD53Event)

# Set the Python install files
set(PYTHON_INSTALL_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}*.so
)

message(STATUS "Python install files: ${PYTHON_INSTALL_FILES}")

# Configure setup.py and __init__.py
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${SETUP_PY_IN} ${SETUP_PY_OUT} @ONLY)

set(INIT_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in)
set(INIT_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)
configure_file(${INIT_PY_IN} ${INIT_PY_OUT} @ONLY)

# Create the custom target to build the Python package
add_custom_target(build-python
    DEPENDS ${MODULE_NAME}
    COMMAND ${Python_EXECUTABLE} -m build --wheel --outdir ${CMAKE_CURRENT_BINARY_DIR}/dist
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Python package"
)

# Create the custom target to install the Python package
add_custom_target(install-python
    DEPENDS build-python
    COMMAND ${Python_EXECUTABLE} -m pip install --user ${CMAKE_CURRENT_BINARY_DIR}/dist/*.whl
    COMMENT "Installing Python package"
)

# Copy the built module to the local RD53Event package directory
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_INSTALL_FILES} ${LOCAL_RD53PY}/
    COMMENT "Copying built Python module to package directory"
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES  
    ${LOCAL_RD53PY}/*.so
`)
