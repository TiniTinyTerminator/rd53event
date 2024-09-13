%{
    #include "RD53Event.h"
%}

// Typemap to convert RD53B::HitCoord object into a Python tuple
%typemap(out) RD53::HitCoord {
    $result = PyTuple_New(3);

    const auto [x,y,tot] = $1;

    PyTuple_SetItem($result, 0, PyLong_FromUnsignedLong(x));
    PyTuple_SetItem($result, 1, PyLong_FromUnsignedLong(y));
    PyTuple_SetItem($result, 2, PyLong_FromUnsignedLong(tot));
}

// Typemap to convert a vector of RD53B::HitCoord objects into a Python list of tuples
%typemap(out) std::vector<RD53::HitCoord> {
    $result = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject* tuple = PyTuple_New(3);

        const auto [x,y,tot] = $1.at(i);

        PyTuple_SetItem(tuple, 0, PyLong_FromUnsignedLong(x));
        PyTuple_SetItem(tuple, 1, PyLong_FromUnsignedLong(y));
        PyTuple_SetItem(tuple, 2, PyLong_FromUnsignedLong(tot));
        PyList_SetItem($result, i, tuple);
    }
}
