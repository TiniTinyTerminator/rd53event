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

%STR_REPR_EXTEND(RD53::QuarterCore)
%STR_REPR_EXTEND(RD53::Event)
%STR_REPR_EXTEND(RD53::StreamHeader)
%STR_REPR_EXTEND(RD53::StreamConfig)
