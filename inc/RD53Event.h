#ifndef RD53EVENT_H
#define RD53EVENT_H

#include <iostream>
#include <vector>
#include <map>
#include <array>
#include <stdexcept>
#include <algorithm>
#include <memory>

constexpr int N_QCORES_VERTICAL = 336 / 2;   // this is the physical number of rows quarter cores on the readout chip
constexpr int N_QCORES_HORIZONTAL = 432 / 8; // this is the physical number of columns quarter cores on the readout chip

struct Rd53StreamConfig
{
    int size_qcore_vertical = 4;   // this is the physical dimensons of the quarter cores on the sensor
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
    QuarterCore(const Rd53StreamConfig &config, uint8_t col = 0, uint8_t row = 0);

    QuarterCore() = default;

    /**
     * @brief Returns the hit value at the specified column and row
     *
     * @param x The column index of the hit
     * @param y The row index of the hit
     * @return A pair containing a boolean indicating whether a hit exists and the total value of the hit
     * @throws std::runtime_error If the column or row index is out of bounds or the qcore size is wrong
     */
    std::pair<bool, uint8_t> get_hit(uint8_t x, uint8_t y) const;

    /**
     * @brief Returns the hit value at the specified index in the hit map
     *
     * @param index The index of the hit value to retrieve
     * @return A pair containing a boolean indicating whether a hit exists and the total value of the hit
     * @throws std::runtime_error If the index is out of bounds or the qcore size is wrong
     */
    std::pair<bool, uint8_t> get_hit(uint8_t index) const;

    /**
     * @brief Sets the hit value at the specified column and row
     *
     * @param col The column index of the hit
     * @param row The row index of the hit
     * @param tot The total value of the hit (default: 0)
     *
     * @throws std::runtime_error If the column or row index is out of bounds or the qcore size is wrong
     */
    void set_hit(uint8_t col, uint8_t row, uint8_t tot);

    /**
     * @brief Sets the hit value at the specified index in the hit map
     *
     * @param index The index of the hit value to set
     * @param tot The total value of the hit
     *
     * @throws std::runtime_error If the index is out of bounds or the qcore size is wrong
     */
    void set_hit(uint8_t index, uint8_t tot);

    /**
     * @brief Sets the hit value using a raw integer
     *
     * @param value The raw integer value to set the hit map to
     */
    void set_hit_raw(uint16_t value, uint64_t tots);

    /**
     * @brief Returns a pair containing the raw hit value and the raw total value of the hit map.
     *
     * The raw hit value is a 16-bit unsigned integer, where each bit corresponds to a hit in the hit map.
     * The raw total value is a 64-bit unsigned integer, where each 4-bit group corresponds to the total value of a hit in the hit map.
     *
     * @return A pair containing the raw hit value and the raw total value of the hit map.
     */
    std::pair<uint16_t, uint64_t> get_hit_raw() const;

    /**
     * @brief Returns a sparsified map of hits in the quarter core.
     *
     * The function returns a vector of triplets containing the row, column and total values of hits in the quarter core.
     * This sparsification is achieved by only storing hits that have a non-zero total value.
     *
     * @return A vector of triplets containing the row, column and total values of hits.
     */
    std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> get_hit_vectors() const;

    /**
     * @brief Returns a 2D vector representing the hit map of the QuarterCore
     *
     * The function returns a 2D vector where each element is a pair of a boolean
     * indicating whether a hit exists and the total value of the hit.
     *
     * @return A 2D vector representing the hit map of the QuarterCore
     */
    std::vector<std::vector<std::pair<bool, uint8_t>>> get_hit_map() const;

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
    uint8_t hit_index(uint8_t row, uint8_t col) const;

    // Member variables
    /** The Rd53StreamConfig object that contains the configuration parameters */
    const Rd53StreamConfig *config;
    /** The column index of the quarter core */
    uint8_t col;
    /** The row index of the quarter core */
    uint8_t row;
    /** A boolean indicating whether the quarter core is the last in the event */
    bool is_last;
    /** A boolean indicating whether the quarter core is a neighbour */
    bool is_neighbour;
    /** A boolean indicating whether the quarter core is the last in the row */
    bool is_last_in_event;

private:
    /** The hit map representing the hits in the quarter core */
    std::array<bool, 16> hits;
    /** The total values of the hits in the quarter core */
    std::array<uint8_t, 16> tots;
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

    // /**
    //  * @brief Set the quarter cores in the event
    //  *
    //  * @param qcores The vector of QuarterCore objects to set
    //  */
    // void set_quarter_cores(const std::vector<QuarterCore> &qcores);

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
    /** The l1 trigger ID */
    int lcid;
    /** The bunch crossing ID */
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

/**
 * @brief Prints "Hello World!" to the console
 */
void say_hello_world(std::string);

#endif // RD53EVENT_H
