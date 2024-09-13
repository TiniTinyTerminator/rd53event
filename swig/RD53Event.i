%module RD53Event


%{
    #include "RD53Event.h"
%}

#define SWIGWORDSIZE64

%include "std_string.i"
%include "std_array.i"
%include "std_vector.i"
%include "std_pair.i"
%include "shared_ptr.i"
%include "stdint.i"
%include "exception.i"       

%feature("python:annotations", "c");

// Global exception handler for all functions
%exception {
    try {
        $action
    }
    catch (const std::invalid_argument& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
    catch (const std::out_of_range& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    }
    catch (const std::runtime_error& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
    catch (const std::exception& e) {
        // Catch any other standard exceptions
        PyErr_SetString(PyExc_Exception, e.what());
        return NULL;
    }
    catch (...) {
        // Catch all other unknown exceptions
        PyErr_SetString(PyExc_Exception, "Unknown exception occurred");
        return NULL;
    }
}

namespace std {
    %template(EventVector) vector<RD53::Event>;
    %template(QcoreRawData) pair<uint16_t, uint64_t>;
    %template(QcoreVector) vector<RD53::QuarterCore>;
    %template(HitCoordVector) vector<RD53::HitCoord>;
}

// Handle overloaded methods
%rename(get_hit_by_coordinates) RD53::QuarterCore::get_hit(uint8_t, uint8_t) const;
%rename(get_hit_by_index) RD53::QuarterCore::get_hit(uint8_t) const;

%rename(set_hit_by_coordinates) QuarterCore::set_hit(uint8_t, uint8_t, uint8_t);
%rename(set_hit_by_index) RD53::QuarterCore::set_hit(uint8_t, uint8_t);

%include "hitCoord.i"
%include "streamVector.i"
%include "event.i"

%include "RD53Event.h"