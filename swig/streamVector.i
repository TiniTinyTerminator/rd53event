
%{
    #include "RD53BEvent.h"
%}

%typemap(out) std::vector<RD53B::word_t> {
    PyObject* pyList = PyList_New($1.size());
    for (size_t i = 0; i < $1.size(); ++i) {
        PyObject* item = PyLong_FromUnsignedLongLong($1.at(i));
        PyList_SetItem(pyList, i, item);
    }
    $result = pyList;
}

%typemap(in) std::vector<RD53B::word_t>& {
    PyObject* seq = PySequence_Fast($input, "Expected a sequence");
    if (!seq) SWIG_fail;

    Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
    std::vector<RD53B::word_t>* v = new std::vector<RD53B::word_t>(len);

    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
        if (!PyLong_Check(item)) {
            delete v;
            Py_DECREF(seq);
            SWIG_exception(SWIG_TypeError, "All items in the sequence must be integers");
        }
        (*v)[i] = (RD53B::word_t)PyLong_AsUnsignedLongLong(item);
    }
    Py_DECREF(seq);
    $1 = v;
}

%typemap(freearg) std::vector<RD53B::word_t>& {
    delete $1;
}


