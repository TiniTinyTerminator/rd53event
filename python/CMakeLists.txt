find_package(SWIG REQUIRED)
find_package(Python REQUIRED COMPONENTS Interpreter Development)

include(${SWIG_USE_FILE})

include_directories(${Python_INCLUDE_DIRS})

# Set the language for SWIG and create the interface
set(SWIG_LANGUAGE python)
set(SWIG_INTERFACE ${CMAKE_SOURCE_DIR}/swig/RD53.i)
set_property(SOURCE ${SWIG_INTERFACE} PROPERTY CPLUSPLUS ON)

# Add SWIG module with documentation enabled
set(CMAKE_SWIG_FLAGS -doxygen)

swig_add_library(RD53py
    TYPE SHARED
    LANGUAGE    ${SWIG_LANGUAGE}
    SOURCES     ${SWIG_INTERFACE}
                ${SRC}/Quartercore.cpp
                ${SRC}/Event.cpp
                ${SRC}/Decoder.cpp
    OUTFILE_DIR ${CMAKE_BINARY_DIR}/wrappers
)

# Enable testing
enable_testing()
add_test(NAME PythonBindingsTest COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/test.py)

set(LOCAL_RD53PY ${CMAKE_SOURCE_DIR}/RD53py)

set(PYTHON_INSTALL_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/RD53py.py
        ${CMAKE_CURRENT_BINARY_DIR}/_RD53py.so)

# Configure setup.py and copy to output directory
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${SETUP_PY_IN} ${SETUP_PY_OUT})

# Configure setup.py and copy to output directory
set(INIT_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in)
set(INIT_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)
configure_file(${INIT_PY_IN} ${INIT_PY_OUT})

set(INIT_PY_LOCAL ${CMAKE_SOURCE_DIR}/__init__.py)
configure_file(${INIT_PY_IN} ${INIT_PY_LOCAL})
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${INIT_PY_LOCAL})

set(INIT_PY_SUBLOCAL ${LOCAL_RD53PY}/__init__.py)
configure_file(${INIT_PY_IN} ${INIT_PY_SUBLOCAL})
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${INIT_PY_SUBLOCAL})

# Create the custom target to build the Python package
add_custom_target(build-python
        DEPENDS RD53py
        COMMAND ${Python_EXECUTABLE} -m build
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Python package"
)

# Create the custom target to install the Python package
add_custom_target(install-python
        DEPENDS build-python
        COMMAND ${Python_EXECUTABLE} -m installer ${CMAKE_CURRENT_BINARY_DIR}/dist/RD53-0.1.0-py3-none-any.whl
        COMMENT "Installing Python package"
)


add_custom_command(TARGET RD53py POST_BUILD
        COMMAND cp ${PYTHON_INSTALL_FILES} ${LOCAL_RD53PY}
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES  ${LOCAL_RD53PY}/_RD53py.so ${LOCAL_RD53PY}/RD53py.py)
