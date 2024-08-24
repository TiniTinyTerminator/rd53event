
%{
    #include "RD53BEvent.h"

%}

%include "std_vector.i"

%template(QcoreVector) std::vector<RD53B::QuarterCore>;

%extend RD53B::Event {
    Event(const StreamConfig &config, const StreamHeader &header, PyObject *object) {
        PyObject* seq = PySequence_Fast(object, "Expected a sequence of tuples of length 3");

        Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
        std::vector<RD53B::HitCoord> v(len);

        for (Py_ssize_t i = 0; i < len; i++) {
            PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
            if (!PyTuple_Check(item) || PyTuple_Size(item) != 3) {
                Py_DECREF(seq);
                throw std::invalid_argument("Expected a sequence of tuples of length 3");            
            }

            PyObject* item0 = PyTuple_GetItem(item, 0);
            PyObject* item1 = PyTuple_GetItem(item, 1);
            PyObject* item2 = PyTuple_GetItem(item, 2);

            if (!PyLong_Check(item0) || !PyLong_Check(item1) || !PyLong_Check(item2)) {
                Py_DECREF(seq);
                throw std::invalid_argument("Expected a sequence of tuples of integers");            
            }

            uint16_t x = (uint16_t)PyLong_AsUnsignedLong(item0);
            uint16_t y = (uint16_t)PyLong_AsUnsignedLong(item1);
            uint8_t val = (uint8_t)PyLong_AsUnsignedLong(item2);

            v[i] = RD53B::HitCoord(x, y, val);
        }
        Py_DECREF(seq);

        return new RD53B::Event(config, header, std::move(v));
    }
}