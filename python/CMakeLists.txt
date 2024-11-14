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
    ${SRC_DIR}/TEPXEvent.cpp
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

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
  ${LOCAL_RD53PY}/__init__.py
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/__init_global.py.in
  ${CMAKE_SOURCE_DIR}/__init__.py
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml.in
  ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml
)

# Create the custom target to build the Python package
add_custom_target(package-python
    DEPENDS ${MODULE_NAME}
    COMMAND ${Python_EXECUTABLE} -m build --wheel --outdir ${CMAKE_CURRENT_BINARY_DIR}/dist
    COMMENT "Building Python package"
)

# Create the custom target to install the Python package
add_custom_target(install-python
    DEPENDS package-python
    COMMAND ${Python_EXECUTABLE} -m pip install --user ${CMAKE_CURRENT_BINARY_DIR}/dist/*.whl
    COMMENT "Installing Python package"
)

# Generate stubs using pybind11-stubgen after the module is built
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${Python_EXECUTABLE} -m pybind11_stubgen ${MODULE_NAME} --output-dir ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}/*.pyi ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${MODULE_NAME}
    COMMENT "Generating stubs for Python module using pybind11-stubgen"
)

# Copy the built module to the local RD53Event package directory
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCAL_RD53PY}
    COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}*.so ${LOCAL_RD53PY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}/*.pyi ${LOCAL_RD53PY}/
    COMMENT "Copying built Python module to package directory"
)

set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES ${LOCAL_RD53PY})