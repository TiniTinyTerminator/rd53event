// rd53event_bindings.cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "RD53Event.h"
#include "utils.h"

namespace py = pybind11;

PYBIND11_MODULE(RD53Eventpy, m) {
    m.doc() = "Pybind11 bindings for the RD53Event library, providing access to RD53 event data structures and functions.";

    // Expose constants
    m.attr("N_QCORES_VERTICAL") = RD53::N_QCORES_VERTICAL;
    m.attr("N_QCORES_HORIZONTAL") = RD53::N_QCORES_HORIZONTAL;

    // Expose data_widths constants in a submodule
    py::module_ data_widths = m.def_submodule("data_widths", "Constants representing the widths of different data fields in the RD53 event data stream.");
    data_widths.attr("TRIGGER_TAG_WIDTH") = RD53::data_widths::TRIGGER_TAG_WIDTH;
    data_widths.attr("CHIP_ID_WIDTH") = RD53::data_widths::CHIP_ID_WIDTH;
    data_widths.attr("COL_WIDTH") = RD53::data_widths::COL_WIDTH;
    data_widths.attr("ROW_WIDTH") = RD53::data_widths::ROW_WIDTH;
    data_widths.attr("IS_LAST_WIDTH") = RD53::data_widths::IS_LAST_WIDTH;
    data_widths.attr("IS_NEIGHBOUR_WIDTH") = RD53::data_widths::IS_NEIGHBOUR_WIDTH;
    data_widths.attr("HITMAP_WIDTH") = RD53::data_widths::HITMAP_WIDTH;
    data_widths.attr("TOT_WIDTH") = RD53::data_widths::TOT_WIDTH;

    // Expose DataTags enum
    py::enum_<DataTags>(m, "DataTags", "Enumeration of data tags used in RD53 event data.")
        .value("TRIGGER_TAG", DataTags::TRIGGER_TAG)
        .value("EXTRA_IDS", DataTags::EXTRA_IDS)
        .value("COLUMN", DataTags::COLUMN)
        .value("IS_NEIGHBOUR", DataTags::IS_NEIGHBOUR)
        .value("IS_LAST", DataTags::IS_LAST)
        .value("ROW", DataTags::ROW)
        .value("HITMAP", DataTags::HITMAP)
        .value("S1", DataTags::S1)
        .value("S2", DataTags::S2)
        .value("S3", DataTags::S3)
        .value("HITPAIR", DataTags::HITPAIR)
        .value("TOT", DataTags::TOT)
        .export_values();

    // Bind StreamHeader class
    py::class_<RD53::StreamHeader>(m, "StreamHeader", "Represents the header of a stream of RD53 event data.")
        .def(py::init<uint8_t, uint8_t, uint8_t, uint16_t, uint16_t>(),
             py::arg("trigger_tag") = 0, py::arg("trigger_pos") = 0, py::arg("chip_id") = 0,
             py::arg("bcid") = 0, py::arg("l1id") = 0,
             "Constructs a StreamHeader object with optional parameters.")
        .def_readwrite("trigger_tag", &RD53::StreamHeader::trigger_tag, "The trigger tag field.")
        .def_readwrite("trigger_pos", &RD53::StreamHeader::trigger_pos, "The trigger position field.")
        .def_readwrite("chip_id", &RD53::StreamHeader::chip_id, "The chip ID field.")
        .def_readwrite("bcid", &RD53::StreamHeader::bcid, "The BCID field.")
        .def_readwrite("l1id", &RD53::StreamHeader::l1id, "The L1ID field.")
        .def("as_str", &RD53::StreamHeader::as_str, "Returns a string representation of the StreamHeader object.");

    // Bind StreamConfig class
    py::class_<RD53::StreamConfig>(m, "StreamConfig", "Represents the configuration parameters of a stream of RD53 event data.")
        .def(py::init<int, int, bool, bool, bool, bool, bool, bool, int>(),
             py::arg("qcore_vertical") = 4, py::arg("qcore_horizontal") = 4,
             py::arg("_chip_id") = false, py::arg("_drop_tot") = false,
             py::arg("_compressed_hitmap") = false, py::arg("_eos_marker") = false,
             py::arg("_bcid") = false, py::arg("_l1id") = false,
             py::arg("_events_per_stream") = 0,
             "Constructs a StreamConfig object with optional parameters.")
        .def_readwrite("size_qcore_vertical", &RD53::StreamConfig::size_qcore_vertical, "Physical dimensions of the quarter cores on the sensor vertically.")
        .def_readwrite("size_qcore_horizontal", &RD53::StreamConfig::size_qcore_horizontal, "Physical dimensions of the quarter cores on the sensor horizontally.")
        .def_readwrite("chip_id", &RD53::StreamConfig::chip_id, "Include chip ID in the stream.")
        .def_readwrite("drop_tot", &RD53::StreamConfig::drop_tot, "Whether to drop ToT (Time over Threshold) information.")
        .def_readwrite("compressed_hitmap", &RD53::StreamConfig::compressed_hitmap, "Whether the hitmap is compressed.")
        .def_readwrite("eos_marker", &RD53::StreamConfig::eos_marker, "Include End of Stream marker.")
        .def_readwrite("bcid", &RD53::StreamConfig::bcid, "Include BCID (Bunch Crossing ID) in the stream.")
        .def_readwrite("l1id", &RD53::StreamConfig::l1id, "Include L1ID (Level 1 Trigger ID) in the stream.")
        .def_readwrite("events_per_stream", &RD53::StreamConfig::events_per_stream, "Number of events per stream (unused at the moment).")
        .def("as_str", &RD53::StreamConfig::as_str, "Returns a string representation of the StreamConfig object.");

    // Bind QuarterCore class
    py::class_<RD53::QuarterCore>(m, "QuarterCore", "Represents a QuarterCore object containing hits in a quarter core of the RD53 detector.")
        .def(py::init<const RD53::StreamConfig &, uint8_t, uint8_t>(),
             py::arg("config"), py::arg("col") = 0, py::arg("row") = 0,
             "Constructs a QuarterCore object with specified configuration and optional column and row indices.")
        .def(py::init<uint8_t, uint8_t>(),
             py::arg("col") = 0, py::arg("row") = 0,
             "Constructs a QuarterCore object with optional column and row indices.")
        .def("get_hit", (std::pair<bool, uint8_t> (RD53::QuarterCore::*)(uint8_t, uint8_t) const) &RD53::QuarterCore::get_hit,
             py::arg("col"), py::arg("row"),
             "Returns the hit value at the specified column and row as a pair (hit_exists, tot_value).")
        .def("get_hit_by_index", (std::pair<bool, uint8_t> (RD53::QuarterCore::*)(uint8_t) const) &RD53::QuarterCore::get_hit,
             py::arg("index"),
             "Returns the hit value at the specified index in the hit map as a pair (hit_exists, tot_value).")
        .def("set_hit", (void (RD53::QuarterCore::*)(uint8_t, uint8_t, uint8_t)) &RD53::QuarterCore::set_hit,
             py::arg("col"), py::arg("row"), py::arg("tot") = 0,
             "Sets the hit value at the specified column and row.")
        .def("set_hit_by_index", (void (RD53::QuarterCore::*)(uint8_t, uint8_t)) &RD53::QuarterCore::set_hit,
             py::arg("index"), py::arg("tot"),
             "Sets the hit value at the specified index in the hit map.")
        .def("set_hit_raw", &RD53::QuarterCore::set_hit_raw,
             py::arg("value"), py::arg("tots"),
             "Sets the hit value using raw integer values.")
        .def("get_hit_raw", &RD53::QuarterCore::get_hit_raw,
             "Returns a pair containing the raw hit value and the raw total value of the hit map.")
        .def("get_hit_vectors", &RD53::QuarterCore::get_hit_vectors,
             "Returns a sparsified vector of hits in the quarter core.")
        .def("get_hit_map", &RD53::QuarterCore::get_hit_map,
             "Returns a 2D vector representing the hit map of the QuarterCore.")
        .def("get_binary_tree", &RD53::QuarterCore::get_binary_tree,
             "Returns the binary tree representation of the hit map.")
        .def("serialize_qcore", &RD53::QuarterCore::serialize_qcore,
             py::arg("prev_last_in_col"),
             "Serializes the quarter core data into a vector of tuples.")
        .def("hit_index", &RD53::QuarterCore::hit_index,
             py::arg("col"), py::arg("row"),
             "Returns the index in the hit map corresponding to the specified row and column.")
        .def("get_col", &RD53::QuarterCore::get_col,
             "Gets the column index of the quarter core.")
        .def("set_col", &RD53::QuarterCore::set_col,
             py::arg("col"),
             "Sets the column index of the quarter core.")
        .def("get_row", &RD53::QuarterCore::get_row,
             "Gets the row index of the quarter core.")
        .def("set_row", &RD53::QuarterCore::set_row,
             py::arg("row"),
             "Sets the row index of the quarter core.")
        .def("get_is_last", &RD53::QuarterCore::get_is_last,
             "Gets a boolean indicating whether the quarter core is the last in the event.")
        .def("set_is_last", &RD53::QuarterCore::set_is_last,
             py::arg("is_last"),
             "Sets a boolean indicating whether the quarter core is the last in the event.")
        .def("get_is_neighbour", &RD53::QuarterCore::get_is_neighbour,
             "Gets a boolean indicating whether the quarter core is a neighbour.")
        .def("set_is_neighbour", &RD53::QuarterCore::set_is_neighbour,
             py::arg("is_neighbour"),
             "Sets a boolean indicating whether the quarter core is a neighbour.")
        .def("get_is_last_in_event", &RD53::QuarterCore::get_is_last_in_event,
             "Gets a boolean indicating whether the quarter core is the last in the event.")
        .def("set_is_last_in_event", &RD53::QuarterCore::set_is_last_in_event,
             py::arg("is_last_in_event"),
             "Sets a boolean indicating whether the quarter core is the last in the event.")
        .def("set_config", &RD53::QuarterCore::set_config,
             py::arg("config"),
             "Sets the StreamConfig object.")
        .def("get_config", &RD53::QuarterCore::get_config,
             "Gets the StreamConfig object.")
        .def("as_str", &RD53::QuarterCore::as_str,
             "Returns a string representation of the QuarterCore object.")
        .def("__eq__", &RD53::QuarterCore::operator==,
             "Checks if two QuarterCore objects are equal.");

    // Bind Event class
    py::class_<RD53::Event>(m, "Event", "Represents an Event object containing hits in the RD53 detector.")
        .def(py::init<>(),
             "Default constructor for Event.")
        .def(py::init<const RD53::Event &>(),
             py::arg("other"),
             "Copy constructor for Event.")
        .def(py::init<const RD53::StreamConfig &, const RD53::StreamHeader &, const std::vector<RD53::HitCoord> &>(),
             py::arg("config"), py::arg("header"), py::arg("hits"),
             "Constructs an Event object with specified configuration, header, and hits.")
        .def(py::init<const RD53::StreamConfig &, const RD53::StreamHeader &, std::vector<RD53::QuarterCore> &>(),
             py::arg("config"), py::arg("header"), py::arg("qcores"),
             "Constructs an Event object with specified configuration, header, and quarter cores.")
        .def("serialize_event", &RD53::Event::serialize_event,
             "Serializes the event data into a vector of 64-bit integers.")
        .def("get_qcores", &RD53::Event::get_qcores,
             "Retrieves the vector of QuarterCore objects representing the quarter cores in the event.")
        .def("get_hits", &RD53::Event::get_hits,
             "Retrieves the vector of hits in the event.")
        .def("as_str", &RD53::Event::as_str,
             "Returns a string representation of the Event object.")
        .def_readonly("config", &RD53::Event::config, "The StreamConfig object that contains the configuration parameters.")
        .def_readonly("header", &RD53::Event::header, "The StreamHeader object that contains the header of the event.");

    // Bind Decoder class
    py::class_<RD53::Decoder>(m, "Decoder", "A class for decoding streams of RD53 event data.")
        .def(py::init<const RD53::StreamConfig &, std::vector<RD53::word_t> &>(),
             py::arg("config"), py::arg("words"),
             "Constructs a new Decoder object with the specified configuration and event data stream.")
        .def("process_stream", &RD53::Decoder::process_stream,
             "Decodes the event data stream.")
        .def("get_events", &RD53::Decoder::get_events,
             "Returns the list of decoded Event objects.");
}
