%module RD53BEvent


%{
    #include "RD53BEvent.h"

    using namespace RD53B;
%}

%include "std_string.i"
%include "std_array.i"
%include "std_vector.i"
%include "std_pair.i"
%include "inttypes.i"

%template(HitCoordVector) std::vector<HitCoord>;
%template(QCoreVector) std::vector<QuarterCore>;
%template(EventVector) std::vector<Event>;
%template(StreamVector) std::vector<word_t>;

%feature("python:annotations", "c");

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

// Typemap for converting std::pair<uint16_t, uint64_t> to Python tuple
%typemap(out) std::pair<uint16_t, uint64_t> {
    PyObject *py_tuple = PyTuple_New(2);
    PyTuple_SetItem(py_tuple, 0, PyLong_FromUnsignedLong($1.first));
    PyTuple_SetItem(py_tuple, 1, PyLong_FromUnsignedLongLong($1.second));
    $result = py_tuple;
}

// Define the typemap for converting Python list to std::vector<long long>
%typemap(in) std::vector<word_t> & (std::vector<word_t> * vec ){
    PyObject *seq = PySequence_Fast($input, "Expected a sequence");
    if (!seq) SWIG_fail;
    
    Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);

    vec = new std::vector<word_t>(len);

    for (Py_ssize_t i = 0; i < len; ++i) {
        PyObject *item = PySequence_Fast_GET_ITEM(seq, i);
        word_t value = (word_t)PyLong_AsUnsignedLongLong(item);
        if (PyErr_Occurred()) {
            Py_DECREF(seq);
            SWIG_fail;
        }
        vec->data()[i] = value;
    }

    $1 = vec;

    Py_DECREF(seq);
}


// Typemap for converting std::vector<word_t> to Python list
%typemap(out) std::vector<word_t> {
    PyObject* pylist = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject* item = PyLong_FromUnsignedLongLong($1.at(i));
        PyList_SetItem(pylist, i, item);
    }
    $result = pylist;
}

// Typemap to clean up the temporary std::vector after it has been used
%typemap(freearg) std::vector<word_t>* {
    delete $1;
}

%typemap(out) std::vector<Event> {
    $result = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyList_SetItem($result, i, SWIG_NewPointerObj(new Event($1.at(i)), SWIGTYPE_p_RD53Event, SWIG_POINTER_OWN));
    }
}

%define %STR_REPR_EXTEND(CLASS)
%extend CLASS {
    std::string __str__() const {
         return $self->as_str();
    }

    std::string __repr__() const {
         return $self->as_str();
    }
}
%enddef

%STR_REPR_EXTEND(RD53B::QuarterCore)
%STR_REPR_EXTEND(RD53B::Event)
%STR_REPR_EXTEND(RD53B::Header)
%STR_REPR_EXTEND(RD53B::StreamConfig)
%STR_REPR_EXTEND(RD53B::HitCoord)

// Handle overloaded methods
%rename(get_hit_by_coordinates) QuarterCore::get_hit(uint8_t, uint8_t) const;
%rename(get_hit_by_index) QuarterCore::get_hit(uint8_t) const;

%rename(set_hit_by_coordinates) QuarterCore::set_hit(uint8_t, uint8_t, uint8_t);
%rename(set_hit_by_index) QuarterCore::set_hit(uint8_t, uint8_t);

%include "RD53BEvent.h"
