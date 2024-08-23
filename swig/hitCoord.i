%{
    #include "RD53BEvent.h"
    #include <stdexcept>
%}

// Custom constructor for Python using a tuple
%extend RD53B::HitCoord {
    HitCoord(PyObject* tuple) {
        if (!PyTuple_Check(tuple) || PyTuple_Size(tuple) != 3) {
            PyErr_SetString(PyExc_TypeError, "Expected a tuple of 3 items");
            throw std::invalid_argument("Invalid argument");
        }

        PyObject* item0 = PyTuple_GetItem(tuple, 0);
        PyObject* item1 = PyTuple_GetItem(tuple, 1);
        PyObject* item2 = PyTuple_GetItem(tuple, 2);

        if (!PyLong_Check(item0) || !PyLong_Check(item1) || !PyLong_Check(item2)) {
            PyErr_SetString(PyExc_TypeError, "Tuple elements must be integers");
            throw std::invalid_argument("Invalid argument");
        }

        uint32_t x = (uint32_t)PyLong_AsUnsignedLong(item0);
        uint32_t y = (uint32_t)PyLong_AsUnsignedLong(item1);
        uint32_t z = (uint32_t)PyLong_AsUnsignedLong(item2);

        // Use the initializer list to construct the object
        return new RD53B::HitCoord(x, y, z);
    }
}

// Typemap to convert RD53B::HitCoord object into a Python tuple
%typemap(out) RD53B::HitCoord {
    $result = PyTuple_New(3);
    PyTuple_SetItem($result, 0, PyLong_FromUnsignedLong($1.x));
    PyTuple_SetItem($result, 1, PyLong_FromUnsignedLong($1.y));
    PyTuple_SetItem($result, 2, PyLong_FromUnsignedLong($1.val));
}

// Typemap to convert a vector of RD53B::HitCoord objects into a Python list of tuples
%typemap(out) std::vector<RD53B::HitCoord> {
    $result = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject* tuple = PyTuple_New(3);
        PyTuple_SetItem(tuple, 0, PyLong_FromUnsignedLong($1.at(i).x));
        PyTuple_SetItem(tuple, 1, PyLong_FromUnsignedLong($1.at(i).y));
        PyTuple_SetItem(tuple, 2, PyLong_FromUnsignedLong($1.at(i).val));
        PyList_SetItem($result, i, tuple);
    }
}

