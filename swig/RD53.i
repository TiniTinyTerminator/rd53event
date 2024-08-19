// RD53py.i
%module RD53py

%{
    // #include <vector>
    #include "RD53Event.h"

    using word_t = unsigned long long;
%}

%include "std_string.i"
%include "std_array.i"
%include "std_vector.i"
%include "std_pair.i"
%include "stdint.i"

// Typemap for converting std::vector<std::vector<std::pair<bool, uint8_t>>> to Python list
%typemap(out) std::vector<std::vector<std::pair<bool, uint8_t>>> {
    PyObject *py_list = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject *inner_list = PyList_New($1.at(i).size());
        for (size_t j = 0; j < $1.at(i).size(); ++j) {
            PyObject *pair = PyTuple_New(2);
            PyTuple_SetItem(pair, 0, PyBool_FromLong($1.at(i)[j].first));
            PyTuple_SetItem(pair, 1, PyLong_FromUnsignedLong($1.at(i)[j].second));
            PyList_SetItem(inner_list, j, pair);
        }
        PyList_SetItem(py_list, i, inner_list);
    }
    $result = py_list;
}

// Typemap for converting std::vector<std::tuple<int, int, int>> to Python list
%typemap(out) std::vector<std::tuple<int, int, int>> {
    PyObject *py_list = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SetItem(tuple, 0, PyLong_FromLong(std::get<0>($1.at(i))));
        PyTuple_SetItem(tuple, 1, PyLong_FromLong(std::get<1>($1.at(i))));
        PyTuple_SetItem(tuple, 2, PyLong_FromLong(std::get<2>($1.at(i))));
        PyList_SetItem(py_list, i, tuple);
    }
    $result = py_list;
}

// Typemap for converting std::vector<uint64_t> to Python list
%typemap(out) std::vector<uint64_t> {
    PyObject *py_list = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyList_SetItem(py_list, i, PyLong_FromUnsignedLongLong($1.at(i)));
    }
    $result = py_list;
}

// Typemap for converting std::vector<QuarterCore> to Python list
%typemap(out) std::vector<QuarterCore> {
    PyObject *py_list = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject *py_obj = SWIG_NewPointerObj(new QuarterCore($1.at(i)), SWIGTYPE_p_QuarterCore, SWIG_POINTER_OWN |  0);
        PyList_SetItem(py_list, i, py_obj);
    }
    $result = py_list;
}

// Typemap for converting std::vector<std::tuple<int, int, std::string>> to Python list
%typemap(out) std::vector<std::tuple<uint8_t, uint64_t, std::string>> {
    PyObject *py_list = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SetItem(tuple, 0, PyLong_FromUnsignedLong(std::get<0>($1.at(i))));
        PyTuple_SetItem(tuple, 1, PyLong_FromUnsignedLongLong(std::get<1>($1.at(i))));
        PyTuple_SetItem(tuple, 2, PyUnicode_FromString(std::get<2>($1.at(i)).c_str()));
        PyList_SetItem(py_list, i, tuple);
    }
    $result = py_list;
}

// Typemap for converting std::pair<uint16_t, uint64_t> to Python tuple
%typemap(out) std::pair<uint16_t, uint64_t> {
    PyObject *py_tuple = PyTuple_New(2);
    PyTuple_SetItem(py_tuple, 0, PyLong_FromUnsignedLong($1.first));
    PyTuple_SetItem(py_tuple, 1, PyLong_FromUnsignedLongLong($1.second));
    $result = py_tuple;
}

// Typemap for converting std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> to Python list
%typemap(out) std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> {
    PyObject *py_list = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SetItem(tuple, 0, PyLong_FromUnsignedLong(std::get<0>($1.at(i))));
        PyTuple_SetItem(tuple, 1, PyLong_FromUnsignedLong(std::get<1>($1.at(i))));
        PyTuple_SetItem(tuple, 2, PyLong_FromUnsignedLong(std::get<2>($1.at(i))));
        PyList_SetItem(py_list, i, tuple);
    }
    $result = py_list;
}

// Typemap for converting Python list to std::vector<word_t>
%typemap(in) std::vector<word_t> {
    PyObject *obj = $input;
    if (!PyList_Check(obj)) {
        SWIG_exception_fail(SWIG_TypeError, "Expected a Python list");
    }
    Py_ssize_t size = PyList_Size(obj);
    std::vector<word_t>* vec = new std::vector<word_t>();
    vec->reserve(size);
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *item = PyList_GetItem(obj, i);
        word_t temp;
        if (!SWIG_ConvertPtr(item, (void **)&temp, SWIGTYPE_p_word_t, 0)) {
            vec->push_back(temp);
        } else {
            SWIG_exception_fail(SWIG_TypeError, "Failed to convert list item to word_t");
        }
    }
    $1 = vec;
}

// Typemap for converting std::vector<word_t> to Python list
%typemap(out) std::vector<word_t> {
    PyObject* pylist = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject* item = SWIG_NewPointerObj((void *) &($1.data()[i]), SWIGTYPE_p_unsigned_long_long, 0);
        PyList_SetItem(pylist, i, item);
    }
    $result = pylist;
}

// Typemap to clean up the vector after passing it from Python to C++
%typemap(freearg) std::vector<word_t> * {
    delete $1;
}

// Handle overloaded methods
%rename(get_hit_by_coordinates) QuarterCore::get_hit(uint8_t, uint8_t) const;
%rename(get_hit_by_index) QuarterCore::get_hit(uint8_t) const;

%rename(set_hit_by_coordinates) QuarterCore::set_hit(uint8_t, uint8_t, uint8_t);
%rename(set_hit_by_index) QuarterCore::set_hit(uint8_t, uint8_t);

%include "RD53Event.h"
