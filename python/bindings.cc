// RD53Event_py.cpp
//
// This file contains pybind11 bindings for the RD53Event.h header file.
// It exposes the RD53 namespace and its classes to Python.
//
// To compile:
//   c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) RD53Event_py.cpp -o RD53Event_py$(python3-config --extension-suffix)
//
// Note: Replace 'RD53Event_py.cpp' with the path to this file if needed.
#include <cstdint>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "RD53Event.h"  // Include your RD53Event.h header file

namespace py = pybind11;

// Begin module definition
PYBIND11_MODULE(RD53Eventpy, m) {
    m.doc() = "Python bindings for RD53Event using pybind11";

    // Create a submodule for the RD53 namespace
    py::module rd53 = m.def_submodule("RD53", "RD53 namespace");

    // Expose constants
    rd53.attr("N_QCORES_VERTICAL") = RD53::N_QCORES_VERTICAL;
    rd53.attr("N_QCORES_HORIZONTAL") = RD53::N_QCORES_HORIZONTAL;

    // Create a submodule for the data_widths namespace
    py::module data_widths = rd53.def_submodule("data_widths", "data_widths namespace");
    data_widths.attr("TRIGGER_TAG_WIDTH") = RD53::data_widths::TRIGGER_TAG_WIDTH;
    data_widths.attr("CHIP_ID_WIDTH") = RD53::data_widths::CHIP_ID_WIDTH;
    data_widths.attr("COL_WIDTH") = RD53::data_widths::COL_WIDTH;
    data_widths.attr("ROW_WIDTH") = RD53::data_widths::ROW_WIDTH;
    data_widths.attr("IS_LAST_WIDTH") = RD53::data_widths::IS_LAST_WIDTH;
    data_widths.attr("IS_NEIGHBOUR_WIDTH") = RD53::data_widths::IS_NEIGHBOUR_WIDTH;
    data_widths.attr("HITMAP_WIDTH") = RD53::data_widths::HITMAP_WIDTH;
    data_widths.attr("TOT_WIDTH") = RD53::data_widths::TOT_WIDTH;

    // Expose StreamHeader class
    py::class_<RD53::StreamHeader>(rd53, "StreamHeader", "Represents the header of a stream of RD53 event data")
        .def(py::init<uint8_t, uint8_t, uint8_t, uint16_t, uint16_t>(),
             py::arg("trigger_tag") = 0, py::arg("trigger_pos") = 0, py::arg("chip_id") = 0,
             py::arg("bcid") = 0, py::arg("l1id") = 0,
             "Constructor for StreamHeader.\n\n"
             "Args:\n"
             "    trigger_tag (int): The trigger tag field.\n"
             "    trigger_pos (int): The trigger position field.\n"
             "    chip_id (int): The chip ID field.\n"
             "    bcid (int): The BCID field.\n"
             "    l1id (int): The L1ID field.")
        .def_readwrite("trigger_tag", &RD53::StreamHeader::trigger_tag, "The trigger tag field")
        .def_readwrite("trigger_pos", &RD53::StreamHeader::trigger_pos, "The trigger position field")
        .def_readwrite("chip_id", &RD53::StreamHeader::chip_id, "The chip ID field")
        .def_readwrite("bcid", &RD53::StreamHeader::bcid, "The BCID field")
        .def_readwrite("l1id", &RD53::StreamHeader::l1id, "The L1ID field")
        .def("__repr__", &RD53::StreamHeader::as_str, "Returns a string representation of the StreamHeader");

    // Expose StreamConfig class
    py::class_<RD53::StreamConfig>(rd53, "StreamConfig", "Represents the configuration parameters of a stream of RD53 event data")
        .def(py::init<int, int, bool, bool, bool, bool, bool, bool, int>(),
             py::arg("qcore_vertical") = 4, py::arg("qcore_horizontal") = 4,
             py::arg("_chip_id") = false, py::arg("_drop_tot") = false,
             py::arg("_compressed_hitmap") = false, py::arg("_eos_marker") = false,
             py::arg("_bcid") = false, py::arg("_l1id") = false,
             py::arg("_events_per_stream") = 0,
             "Constructor for StreamConfig.\n\n"
             "Args:\n"
             "    qcore_vertical (int): Vertical size of quarter cores.\n"
             "    qcore_horizontal (int): Horizontal size of quarter cores.\n"
             "    _chip_id (bool): Include chip ID.\n"
             "    _drop_tot (bool): Drop TOT values.\n"
             "    _compressed_hitmap (bool): Use compressed hitmap.\n"
             "    _eos_marker (bool): End of stream marker.\n"
             "    _bcid (bool): Include BCID.\n"
             "    _l1id (bool): Include L1ID.\n"
             "    _events_per_stream (int): Number of events per stream.")
        .def_readwrite("size_qcore_vertical", &RD53::StreamConfig::size_qcore_vertical, "Vertical size of quarter cores")
        .def_readwrite("size_qcore_horizontal", &RD53::StreamConfig::size_qcore_horizontal, "Horizontal size of quarter cores")
        .def_readwrite("chip_id", &RD53::StreamConfig::chip_id, "Include chip ID")
        .def_readwrite("drop_tot", &RD53::StreamConfig::drop_tot, "Drop TOT values")
        .def_readwrite("compressed_hitmap", &RD53::StreamConfig::compressed_hitmap, "Use compressed hitmap")
        .def_readwrite("eos_marker", &RD53::StreamConfig::eos_marker, "End of stream marker")
        .def_readwrite("bcid", &RD53::StreamConfig::bcid, "Include BCID")
        .def_readwrite("l1id", &RD53::StreamConfig::l1id, "Include L1ID")
        .def_readwrite("events_per_stream", &RD53::StreamConfig::events_per_stream, "Number of events per stream")
        .def("__repr__", &RD53::StreamConfig::as_str, "Returns a string representation of the StreamConfig");

    // Expose QuarterCore class
    py::class_<RD53::QuarterCore>(rd53, "QuarterCore", "Represents a QuarterCore object")
        .def(py::init<const RD53::StreamConfig &, uint8_t, uint8_t>(),
             py::arg("config"), py::arg("col") = 0, py::arg("row") = 0,
             "Constructor with config, column, and row.\n\n"
             "Args:\n"
             "    config (StreamConfig): Configuration object.\n"
             "    col (int): Column index.\n"
             "    row (int): Row index.",
             py::keep_alive<1, 2>())  // Keep config alive
        .def(py::init<uint8_t, uint8_t>(),
             py::arg("col") = 0, py::arg("row") = 0,
             "Constructor with column and row.\n\n"
             "Args:\n"
             "    col (int): Column index.\n"
             "    row (int): Row index.")
        .def("get_hit",
             (std::pair<bool, uint8_t>(RD53::QuarterCore::*)(uint8_t, uint8_t) const) & RD53::QuarterCore::get_hit,
             py::arg("x"), py::arg("y"),
             "Returns the hit value at the specified column and row.\n\n"
             "Args:\n"
             "    x (int): Column index.\n"
             "    y (int): Row index.\n\n"
             "Returns:\n"
             "    tuple: (bool, int) indicating if hit exists and its TOT value.")
        .def("get_hit",
             (std::pair<bool, uint8_t>(RD53::QuarterCore::*)(uint8_t) const) & RD53::QuarterCore::get_hit,
             py::arg("index"),
             "Returns the hit value at the specified index.\n\n"
             "Args:\n"
             "    index (int): Index in the hit map.\n\n"
             "Returns:\n"
             "    tuple: (bool, int) indicating if hit exists and its TOT value.")
        .def("set_hit",
             (void(RD53::QuarterCore::*)(uint8_t, uint8_t, uint8_t)) & RD53::QuarterCore::set_hit,
             py::arg("col"), py::arg("row"), py::arg("tot") = 0,
             "Sets the hit value at the specified column and row.\n\n"
             "Args:\n"
             "    col (int): Column index.\n"
             "    row (int): Row index.\n"
             "    tot (int): TOT value.")
        .def("set_hit",
             (void(RD53::QuarterCore::*)(uint8_t, uint8_t)) & RD53::QuarterCore::set_hit,
             py::arg("index"), py::arg("tot"),
             "Sets the hit value at the specified index.\n\n"
             "Args:\n"
             "    index (int): Index in the hit map.\n"
             "    tot (int): TOT value.")
        .def("set_hit_raw", &RD53::QuarterCore::set_hit_raw, py::arg("value"), py::arg("tots"),
             "Sets the hit map using raw values.\n\n"
             "Args:\n"
             "    value (int): Raw hit map value.\n"
             "    tots (int): Raw TOT values.")
        .def("get_hit_raw", &RD53::QuarterCore::get_hit_raw,
             "Returns the raw hit map and TOT values.\n\n"
             "Returns:\n"
             "    tuple: (int, int) raw hit map and TOT values.")
        .def("get_hit_vectors", &RD53::QuarterCore::get_hit_vectors,
             "Returns a sparsified map of hits.\n\n"
             "Returns:\n"
             "    list: List of HitCoord tuples.")
        .def("get_hit_map", &RD53::QuarterCore::get_hit_map,
             "Returns the 2D hit map.\n\n"
             "Returns:\n"
             "    list: 2D list of (bool, int) tuples representing hits.")
        .def("get_binary_tree", &RD53::QuarterCore::get_binary_tree,
             "Returns the binary tree representation of the hit map.\n\n"
             "Returns:\n"
             "    tuple: (int, int) representing the binary tree and its length.")
        .def("serialize_qcore", &RD53::QuarterCore::serialize_qcore, py::arg("prev_last_in_col"),
             "Serializes the quarter core data.\n\n"
             "Args:\n"
             "    prev_last_in_col (bool): Whether previous was last in column.\n\n"
             "Returns:\n"
             "    list: List of serialized data tuples.")
        .def("hit_index", &RD53::QuarterCore::hit_index, py::arg("col"), py::arg("row"),
             "Returns the index in the hit map for given col and row.\n\n"
             "Args:\n"
             "    col (int): Column index.\n"
             "    row (int): Row index.\n\n"
             "Returns:\n"
             "    int: Index in the hit map.")
        .def("get_col", &RD53::QuarterCore::get_col, "Gets the column index.")
        .def("set_col", &RD53::QuarterCore::set_col, py::arg("col"), "Sets the column index.")
        .def("get_row", &RD53::QuarterCore::get_row, "Gets the row index.")
        .def("set_row", &RD53::QuarterCore::set_row, py::arg("row"), "Sets the row index.")
        .def("get_is_last", &RD53::QuarterCore::get_is_last, "Checks if it's the last in event.")
        .def("set_is_last", &RD53::QuarterCore::set_is_last, py::arg("is_last"), "Sets if it's the last in event.")
        .def("get_is_neighbour", &RD53::QuarterCore::get_is_neighbour, "Checks if it's a neighbour.")
        .def("set_is_neighbour", &RD53::QuarterCore::set_is_neighbour, py::arg("is_neighbour"), "Sets if it's a neighbour.")
        .def("get_is_last_in_event", &RD53::QuarterCore::get_is_last_in_event, "Checks if it's the last in the row.")
        .def("set_is_last_in_event", &RD53::QuarterCore::set_is_last_in_event, py::arg("is_last_in_event"), "Sets if it's the last in the row.")
        .def("set_config", &RD53::QuarterCore::set_config, py::arg("config"),
             "Sets the StreamConfig object.", py::keep_alive<1, 2>())  // Keep config alive
        .def("get_config", &RD53::QuarterCore::get_config, "Gets the StreamConfig object.")
        .def("__repr__", &RD53::QuarterCore::as_str, "Returns a string representation of the QuarterCore.")
        .def("__eq__", &RD53::QuarterCore::operator==, py::arg("other"),
             "Checks if two QuarterCore objects are equal.");

    // Expose Event class
    py::class_<RD53::Event>(rd53, "Event", "Represents an Event object")
        .def(py::init<>(), "Default constructor.")
        .def(py::init<const RD53::Event &>(), py::arg("other"), "Copy constructor.")
        .def(py::init<const RD53::StreamConfig &, const RD53::StreamHeader &, const std::vector<RD53::HitCoord> &>(),
             py::arg("config"), py::arg("header"), py::arg("hits"),
             "Constructor with config, header, and hits.\n\n"
             "Args:\n"
             "    config (StreamConfig): Configuration object.\n"
             "    header (StreamHeader): Header object.\n"
             "    hits (list): List of HitCoord tuples.")
        .def(py::init<const RD53::StreamConfig &, const RD53::StreamHeader &, std::vector<RD53::QuarterCore> &>(),
             py::arg("config"), py::arg("header"), py::arg("qcores"),
             "Constructor with config, header, and quarter cores.\n\n"
             "Args:\n"
             "    config (StreamConfig): Configuration object.\n"
             "    header (StreamHeader): Header object.\n"
             "    qcores (list): List of QuarterCore objects.")
        .def("serialize_event", &RD53::Event::serialize_event,
             "Serializes the event data into a list of 64-bit integers.\n\n"
             "Returns:\n"
             "    list: List of serialized event data.")
        .def("get_qcores", &RD53::Event::get_qcores,
             "Gets the list of QuarterCore objects.\n\n"
             "Returns:\n"
             "    list: List of QuarterCore objects.")
        .def("get_hits", &RD53::Event::get_hits,
             "Gets the list of hits in the event.\n\n"
             "Returns:\n"
             "    list: List of HitCoord tuples.")
        .def("__repr__", &RD53::Event::as_str, "Returns a string representation of the Event.")
        .def_readonly("config", &RD53::Event::config, "Configuration object.")
        .def_readonly("header", &RD53::Event::header, "Header object.");

    // Expose Decoder class
    py::class_<RD53::Decoder>(rd53, "Decoder", "A class for decoding streams of RD53 event data")
        .def(py::init<const RD53::StreamConfig &, std::vector<RD53::word_t> &>(),
             py::arg("config"), py::arg("words"),
             "Constructor for Decoder.\n\n"
             "Args:\n"
             "    config (StreamConfig): Configuration object.\n"
             "    words (list): List of 64-bit words containing event data.")
        .def("process_stream", &RD53::Decoder::process_stream,
             "Processes the event data stream.")
        .def("get_events", &RD53::Decoder::get_events,
             "Gets the list of decoded events.\n\n"
             "Returns:\n"
             "    list: List of Event objects.");
}
