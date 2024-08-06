#ifndef RD53EVENT_H
#define RD53EVENT_H

#include <iostream>
#include <vector>
#include <map>
#include <array>
#include <stdexcept>
#include <algorithm>
#include <memory>

constexpr int N_QCORES_VERTICAL = 336 / 2; // this is the physical number of rows quarter cores on the readout chip
constexpr int N_QCORES_HORIZONTAL = 432 / 8; // this is the physical number of columns quarter cores on the readout chip

struct Rd53StreamConfig
{
    int size_qcore_vertical = 4; // this is the physical dimensons of the quarter cores on the sensor
    int size_qcore_horizontal = 4; // this is the physical dimensons of the quarter cores on the sensor

    bool chip_id;
    bool drop_tot;
    bool compressed_hitmap;
    bool eos_marker;
    bool bcid;
    bool l1id;

    int events_per_stream; // unused at the moment
};

/**
 * @brief Represents a QuarterCore object
 *
 * The QuarterCore class represents a quarter core in a RD64 detector. It contains
 * information about the hits in the quarter core and provides methods to get and set
 * these hits.
 */
class QuarterCore
{
public:
    /**
     * @brief Constructs a QuarterCore object
     *
     * @param config The Rd53StreamConfig object that contains the configuration parameters
     * @param col The column index of the QuarterCore (default: 0)
     * @param row The row index of the QuarterCore (default: 0)
     */
    QuarterCore(const Rd53StreamConfig &config, int col = 0, int row = 0);

    QuarterCore() = default;

    /**
     * @brief Returns the hit value at the specified index in the hit map
     *
     * @param index The index of the hit value to retrieve
     * @return The hit value at the specified index
     * @throws std::runtime_error If the index is out of bounds or the qcore size is wrong
     */
    std::pair<bool, uint8_t> get_hit(int index) const;

    /**
     * @brief Sets the hit value at the specified column and row
     *
     * @param col The column index of the hit
     * @param row The row index of the hit
     * @param tot The total value of the hit (default: 0)
     */
    void set_hit(int col, int row, int tot = 0);

    /**
     * @brief Sets the hit value using a raw integer
     *
     * @param value The raw integer value to set the hit map to
     */
    void set_hit_raw(int value);

    /**
     * @brief Returns the raw integer value of the hit map
     *
     * @return The raw integer value of the hit map
     */
    int get_hit_raw() const;

    /**
     * @brief Returns a sparsified map of hits in the quarter core
     *
     * @return A vector of pairs containing the row and column indices of hits
     */
    std::vector<std::pair<int, int>> sparsified_hit_map() const;

    /**
     * @brief Returns the binary tree representation of the hit map
     *
     * @return A pair containing the binary tree representation and its length
     */
    std::pair<int, int> get_binary_tree() const;

    /**
     * @brief Serializes the quarter core data into a vector of tuples
     *
     * @param prev_last_in_col A boolean indicating whether the previous element was the last in the column
     * @return A vector of tuples containing the serialized quarter core data
     */
    std::vector<std::tuple<int, int, std::string>> serialize_qcore(bool prev_last_in_col) const;

    /**
     * @brief Returns the index in the hit map corresponding to the specified row and column
     *
     * @param row The row index
     * @param col The column index
     * @return The index in the hit map corresponding to the specified row and column
     */
    int hit_index(int row, int col) const;

    // Member variables
    /** The Rd53StreamConfig object that contains the configuration parameters */
    const Rd53StreamConfig *config;
    /** The hit map representing the hits in the quarter core */
    std::vector<std::vector<bool>> hit_map;
    /** The total values of the hits in the quarter core */
    std::vector<char> tots;
    /** The column index of the quarter core */
    int col;
    /** The row index of the quarter core */
    int row;
    /** A boolean indicating whether the quarter core is the last in the event */
    bool is_last;
    /** A boolean indicating whether the quarter core is a neighbour */
    bool is_neighbour;
    /** A boolean indicating whether the quarter core is the last in the row */
    bool is_last_in_event;
};

/**
 * @brief Represents an Event object
 *
 * The Event class represents an event in a RD64 detector. It contains
 * information about the hits in the event and provides methods to get and set
 * these hits.
 */
class Event
{
public:
    /**
     * @brief Constructs an Event object
     *
     * @param config The Rd53StreamConfig object that contains the configuration parameters
     * @param hits The vector of hits in the event
     * @param event_tag The event tag (default: 0)
     * @param chip_id The chip ID (default: 0)
     * @param lcid The local chamber ID (default: 0)
     * @param bcid The board chamber ID (default: 0)
     */
    Event(const Rd53StreamConfig &config, const std::vector<std::tuple<int, int, int>> &hits,
          int event_tag = 0, int chip_id = 0, int lcid = 0, int bcid = 0);

    /**
     * @brief Retrieves the quarter cores in the event
     *
     * This function retrieves the quarter cores in the event by calling the
     * get_quarter_cores() method of each QuarterCore object in the qcores vector.
     */
    void get_quarter_cores();


    /**
     * @brief Set the quarter cores in the event
     *
     * @param qcores The vector of QuarterCore objects to set
     */
    void set_quarter_cores(const std::vector<QuarterCore> &qcores);

    /**
     * @brief Serializes the event data into a vector of 64-bit integers
     *
     * @return A vector of 64-bit integers containing the serialized event data
     */
    std::vector<uint64_t> serialize_event();

    /** The Rd53StreamConfig object that contains the configuration parameters */
    const Rd53StreamConfig *config;
    /** The vector of hits in the event */
    std::vector<std::tuple<int, int, int>> hits;
    /** The event tag */
    int event_tag;
    /** The chip ID */
    int chip_id;
    /** The local chamber ID */
    int lcid;
    /** The board chamber ID */
    int bcid;
    /** The vector of QuarterCore objects representing the quarter cores in the event */
    std::vector<QuarterCore> qcores;

    private:
    /**
     * @brief Retrieves the quarter core data in the event
     *
     * @return A vector of tuples containing the serialized quarter core data
     */
    std::vector<std::tuple<int, int, std::string>> _retrieve_qcore_data();
};

#endif // RD53EVENT_H
