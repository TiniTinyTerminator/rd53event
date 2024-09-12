%module RD53BEvent


%{
    #include "RD53BEvent.h"
%}

#define SWIGWORDSIZE64

%include "std_string.i"
%include "std_array.i"
%include "std_vector.i"
%include "std_pair.i"
%include "shared_ptr.i"
%include "stdint.i"

%feature("python:annotations", "c");

namespace std {
    %template(EventVector) vector<RD53B::Event>;
    %template(QcoreRawData) pair<uint16_t, uint64_t>;
}

// Handle overloaded methods
%rename(get_hit_by_coordinates) RD53B::QuarterCore::get_hit(uint8_t, uint8_t) const;
%rename(get_hit_by_index) RD53B::QuarterCore::get_hit(uint8_t) const;

%rename(set_hit_by_coordinates) QuarterCore::set_hit(uint8_t, uint8_t, uint8_t);
%rename(set_hit_by_index) RD53B::QuarterCore::set_hit(uint8_t, uint8_t);

%include "hitCoord.i"
%include "streamVector.i"
%include "event.i"

// %include "prints.i"

%include "RD53BEvent.h"
