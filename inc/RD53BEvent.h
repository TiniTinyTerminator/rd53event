#ifndef RD53EVENT_H
#define RD53EVENT_H

#include <iostream>
#include <vector>
#include <map>
#include <array>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <sstream>
#include <cstdint>
#include <fstream>

#include "utils.h"

constexpr bool DEBUG = false;

namespace RD53B
{

    constexpr int N_QCORES_VERTICAL = 336 / 2;   // this is the physical number of rows quarter cores on the readout chip
    constexpr int N_QCORES_HORIZONTAL = 432 / 8; // this is the physical number of columns quarter cores on the readout chip

    /**
     * @brief A type alias for a 64-bit unsigned integer
     */
    using word_t = unsigned long long;

    /** @brief A type alias for a pair of a 64-bit unsigned integer and an 8-bit unsigned integer */
    using DataRead = std::pair<word_t, uint8_t>;

    /**
     * @brief A type alias for a tuple of two 16-bit unsigned integers and an 8-bit unsigned integer
     *
     * This type alias represents the coordinates of a hit in a QuarterCore. The first element of the tuple is the column
     * index, the second element is the row index, and the third element is the total value of the hit.
     */
    using HitCoord = std::tuple<uint16_t, uint16_t, uint8_t>;

    /**
     * @brief A namespace containing constants representing the widths of different data fields in the RD53 event data stream
     */
    namespace data_widths
    {
        /** @brief The width of the trigger tag field */
        constexpr uint8_t TRIGGER_TAG_WIDTH = 8;
        /** @brief The width of the chip ID field */
        constexpr uint8_t CHIP_ID_WIDTH = 2;
        /** @brief The width of the column field */
        constexpr uint8_t COL_WIDTH = 6;
        /** @brief The width of the row field */
        constexpr uint8_t ROW_WIDTH = 8;
        /** @brief The width of the is_last field */
        constexpr uint8_t IS_LAST_WIDTH = 1;
        /** @brief The width of the is_neighbour field */
        constexpr uint8_t IS_NEIGHBOUR_WIDTH = 1;
        /** @brief The width of the hitmap field */
        constexpr uint8_t HITMAP_WIDTH = 16;
        /** @brief The width of the tot field */
        constexpr uint8_t TOT_WIDTH = 4;
    };

    /**
     * @brief A struct representing the header of a stream of RD53 event data
     */
    struct StreamHeader
    {
        /** @brief The trigger tag field */
        uint8_t trigger_tag;
        /** @brief The trigger position field */
        uint8_t trigger_pos;
        /** @brief The chip ID field */
        uint8_t chip_id;
        /** @brief The BCID field */
        uint16_t bcid;
        /** @brief The L1ID field */
        uint16_t l1id;

        StreamHeader(uint8_t trigger_tag = 0, uint8_t trigger_pos = 0, uint8_t chip_id = 0, uint16_t bcid = 0, uint16_t l1id = 0)
            : trigger_tag(trigger_tag), trigger_pos(trigger_pos), chip_id(chip_id), bcid(bcid), l1id(l1id)
        {
        }

        std::string as_str() const
        {
            std::stringstream ss;
            ss << "trigger_tag: " << static_cast<int>(trigger_tag) << ", trigger_pos: " << static_cast<int>(trigger_pos)
               << ", chip_id: " << static_cast<int>(chip_id) << ", bcid: " << bcid << ", l1id: " << l1id;
            return ss.str();
        }
    };

    /**
     * @brief A struct representing the configuration parameters of a stream of RD53 event data
     */
    struct StreamConfig
    {
        uint8_t size_qcore_vertical = 4;   // this is the physical dimensons of the quarter cores on the sensor
        uint8_t size_qcore_horizontal = 4; // this is the physical dimensons of the quarter cores on the sensor

        bool chip_id;
        bool drop_tot;
        bool compressed_hitmap;
        bool eos_marker;
        bool bcid;
        bool l1id;

        int events_per_stream; // unused at the moment

        StreamConfig(int qcore_vertical = 4, int qcore_horizontal = 4, bool _chip_id = false, bool _drop_tot = false,
                     bool _compressed_hitmap = false, bool _eos_marker = false, bool _bcid = false, bool _l1id = false,
                     int _events_per_stream = 0)
            : size_qcore_vertical(qcore_vertical), size_qcore_horizontal(qcore_horizontal), chip_id(_chip_id), drop_tot(_drop_tot), compressed_hitmap(_compressed_hitmap), eos_marker(_eos_marker), bcid(_bcid), l1id(_l1id), events_per_stream(_events_per_stream)
        {
        }

        std::string as_str() const
        {
            std::stringstream ss;
            ss << "size_qcore_vertical: " << static_cast<int>(size_qcore_vertical) << ", size_qcore_horizontal: " << static_cast<int>(size_qcore_horizontal)
               << ", chip_id: " << std::boolalpha << chip_id << ", drop_tot: " << std::boolalpha << drop_tot
               << ", compressed_hitmap: " << std::boolalpha << compressed_hitmap << ", eos_marker: " << std::boolalpha
               << eos_marker << ", bcid: " << std::boolalpha << bcid << ", l1id: " << std::boolalpha << l1id
               << ", events_per_stream: " << events_per_stream;
            return ss.str();
        }
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
         * @param config The StreamConfig object that contains the configuration parameters
         * @param col The column index of the QuarterCore (default: 0)
         * @param row The row index of the QuarterCore (default: 0)
         */
        QuarterCore(const StreamConfig &config, uint8_t col = 0, uint8_t row = 0);

        QuarterCore(uint8_t col = 0, uint8_t row = 0);

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
        std::vector<HitCoord> get_hit_vectors() const;

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
        std::vector<std::tuple<uint8_t, unsigned long long, DataTags>> serialize_qcore(bool prev_last_in_col) const;

        /**
         * @brief Returns the index in the hit map corresponding to the specified row and column
         *
         * @param row The row index
         * @param col The column index
         * @return The index in the hit map corresponding to the specified row and column
         */
        uint8_t hit_index(uint8_t col, uint8_t row) const;

        /**
         * @brief Gets the column index of the quarter core
         *
         * @return The column index of the quarter core
         */
        uint8_t get_col() const { return col_; }

        /**
         * @brief Sets the column index of the quarter core
         *
         * @param col The column index to be set
         */
        void set_col(uint8_t col)
        {
            if (col >= N_QCORES_HORIZONTAL)
                throw std::runtime_error("ERROR: col index out of range");
            col_ = col;
        }

        /**
         * @brief Gets the row index of the quarter core
         *
         * @return The row index of the quarter core
         */
        uint8_t get_row() const { return row_; }

        /**
         * @brief Sets the row index of the quarter core
         *
         * @param row The row index to be set
         */
        void set_row(uint8_t row)
        {
            if (row >= N_QCORES_VERTICAL)
                throw std::runtime_error("ERROR: row index out of range");
            row_ = row;
        }

        /**
         * @brief Gets a boolean indicating whether the quarter core is the last in the event
         *
         * @return A boolean indicating whether the quarter core is the last in the event
         */
        bool get_is_last() const { return is_last_; }

        /**
         * @brief Sets a boolean indicating whether the quarter core is the last in the event
         *
         * @param is_last The boolean indicating whether the quarter core is the last in the event
         */
        void set_is_last(bool is_last) { is_last_ = is_last; }

        /**
         * @brief Gets a boolean indicating whether the quarter core is a neighbour
         *
         * @return A boolean indicating whether the quarter core is a neighbour
         */
        bool get_is_neighbour() const { return is_neighbour_; }

        /**
         * @brief Sets a boolean indicating whether the quarter core is a neighbour
         *
         * @param is_neighbour The boolean indicating whether the quarter core is a neighbour
         */
        void set_is_neighbour(bool is_neighbour) { is_neighbour_ = is_neighbour; }

        /**
         * @brief Gets a boolean indicating whether the quarter core is the last in the row
         *
         * @return A boolean indicating whether the quarter core is the last in the row
         */
        bool get_is_last_in_event() const { return is_last_in_event_; }

        /**
         * @brief Sets a boolean indicating whether the quarter core is the last in the row
         *
         * @param is_last_in_event The boolean indicating whether the quarter core is the last in the row
         *
         * @param is_last_in_event The boolean indicating whether the quarter core is the last in the row
         */
        void set_is_last_in_event(bool is_last_in_event) { is_last_in_event_ = is_last_in_event; }

        /**
         * @brief Sets the StreamConfig object
         *
         * @param config The StreamConfig object
         */
        void set_config(const StreamConfig *config) { this->config_ = config; }

        /**
         * @brief Gets the StreamConfig object
         *
         * @return The StreamConfig object
         */
        StreamConfig get_config() const { return *config_; }

        /**
         * @brief Returns a string representation of the QuarterCore object
         *
         * @return A string representation of the QuarterCore object
         */
        std::string as_str() const;

        bool operator==(const QuarterCore &other) const;

    private:
        // Member variables
        /** The StreamConfig object that contains the configuration parameters */
        const StreamConfig *config_;
        /** The column index of the quarter core */
        uint8_t col_;
        /** The row index of the quarter core */
        uint8_t row_;
        /** A boolean indicating whether the quarter core is the last in the event */
        bool is_last_;
        /** A boolean indicating whether the quarter core is a neighbour */
        bool is_neighbour_;
        /** A boolean indicating whether the quarter core is the last in the row */
        bool is_last_in_event_;
        /** The hit map representing the hits in the quarter core */
        uint16_t hits_;
        /** The total values of the hits in the quarter core */
        uint64_t tots_;
    };

    /**
     * @brief Represents an Event object
     *
     * The Event class represents an event in a RD53 detector. It contains
     * information about the hits in the event and provides methods to get and set
     * these hits.
     */
    class Event
    {
    public:
        Event() = default;
        Event(const Event &other);
        Event &operator=(const Event &other);

        /**
         * @brief Constructs an Event object
         *
         * @param config The StreamConfig object that contains the configuration parameters
         * @param hits The vector of hits in the event
         * @param event_tag The event tag (default: 0)
         * @param chip_id The chip ID (default: 0)
         * @param lcid The local chamber ID (default: 0)
         * @param bcid The board chamber ID (default: 0)
         */
        Event(const StreamConfig &config, const StreamHeader &header, const std::vector<HitCoord> &hits);

        /**
         * @brief Constructs an Event object
         *
         * @param config The StreamConfig object that contains the configuration parameters
         * @param header The StreamHeader object that contains the header of the event
         * @param qcores The vector of QuarterCore objects that contain the hits in the event
         */
        Event(const StreamConfig &config, const StreamHeader &header, std::vector<QuarterCore> &qcores);

        /**
         * @brief Serializes the event data into a vector of 64-bit integers
         *
         * @return A vector of 64-bit integers containing the serialized event data
         */
        std::vector<word_t> serialize_event();

        /** The StreamConfig object that contains the configuration parameters */
        const StreamConfig config;

        /** The event header */
        const StreamHeader header;

        /**
         * Retrieves the vector of QuarterCore objects representing the quarter cores in the event.
         *
         * @return The vector of QuarterCore objects.
         *
         * @throws None
         */
        std::vector<QuarterCore> get_qcores()
        {
            if (qcores_.empty())
                _get_qcores_from_pixelframe();

            std::vector<QuarterCore> output = qcores_;

            for (size_t i = 0; i < output.size(); i++)
            {
                output[i].set_config(nullptr);
            }

            return qcores_;
        }

        /**
         * Retrieves the vector of hits in the event.
         *
         * @return The vector of hits in the event.
         *
         * @throws None
         */
        std::vector<HitCoord> get_hits()
        {
            if (hits_.empty())
                _get_pixelframe_from_qcores();
            return hits_;
        }

        /**
         * @brief Create a string from the data of this class
         *
         * @return A string containing the data of this class
         */
        std::string as_str() const;

    private:
        /** The vector of hits in the event */
        std::vector<HitCoord> hits_;

        /** The vector of QuarterCore objects representing the quarter cores in the event */
        std::vector<QuarterCore> qcores_;

        /**
         * @brief Retrieves the quarter core data in the event
         *
         * @return A vector of tuples containing the serialized quarter core data
         */
        std::vector<std::tuple<uint8_t, word_t, DataTags>> _retrieve_qcore_data();

        /**
         * @brief Retrieves the quarter cores in the event
         *
         * This function retrieves the quarter cores in the event by calling the
         * _get_qcores_from_pixelframe() method of each QuarterCore object in the qcores vector.
         */
        void _get_qcores_from_pixelframe();

        /**
         * @brief Retrieves the pixel frame in the event
         *
         * This function retrieves the pixel frame in the event by calling the
         * _get_pixelframe_from_qcores() method of each QuarterCore object in the qcores vector.
         */
        void _get_pixelframe_from_qcores();
    };

    /**
     * @brief A class for decoding streams of RD53 event data
     */
    class Decoder
    {
    public:
        /**
         * @brief Constructs a new Decoder object
         *
         * @param config The StreamConfig object containing the configuration parameters
         * @param words The vector of 64-bit unsigned integers containing the event data stream
         */
        Decoder(const StreamConfig &config, std::vector<word_t> &words);

        /**
         * @brief Decodes the event data stream
         */
        void process_stream();

        std::vector<Event> get_events() const;

    private:
        /**
         * @brief Validates the chip ID field
         */
        void _validate_chip_id();

        /**
         * @brief Gets the trigger IDs from the event data stream
         */
        void _get_trigger_ids();

        /**
         * @brief Gets the trigger tag from the event data stream
         */
        void _get_trigger_tag();

        /**
         * @brief Gets the column index from the event data stream
         */
        void _get_col();

        /**
         * @brief Gets the is_neighbour and is_last fields from the event data stream
         */
        void _get_neighbour_and_last();

        /**
         * @brief Gets the row index from the event data stream
         */
        void _get_row();

        /**
         * @brief Gets the hitmap and tot fields from the event data stream
         */
        void _get_hitmap();

        /**
         * @brief Calculates the tot value from the number of hits
         *
         * @param n_hits The number of hits
         * @return The tot value
         */
        uint64_t _get_tots(uint16_t n_hits);

        /**
         * @brief Shifts the bit index of the event data stream
         *
         * @param bit_index The new bit index
         * @param remove_start Whether to remove the start bit
         * @return The shifted value
         */
        inline word_t _shift_stream(size_t bit_index);

        /**
         * @brief Gets the specified number of bits from the event data stream
         *
         * @param n_bits The number of bits to get
         * @param increment Whether to increment the bit index
         * @return The bits as a 64-bit unsigned integer
         */
        inline word_t _get_nbits(uint8_t n_bits, bool increment = true);

        /**
         * @brief Indicates the start of a new event
         */
        inline void _new_event();

        /**
         * @brief The bit index of the event data stream
         */
        size_t bit_index_;

        /**
         * @brief The jump size of the event data stream
         */
        size_t jump_size_;

        /**
         * @brief The size of a word in bits
         */
        uint8_t word_size_;

        /**
         * @brief The size of the meta data in bits
         */
        uint8_t word_meta_size_;

        /** @brief The event data stream */
        std::vector<word_t> stream_;

        /** @brief The StreamConfig object containing the configuration parameters */
        const StreamConfig config_;

        /** @brief The vector of pairs representing the events in the event data stream */
        using EventVec = std::vector<std::pair<StreamHeader, std::vector<QuarterCore>>>;

        /** @brief The vector of pairs representing the events in the event data stream */
        EventVec events_;

        /** @brief An iterator pointing to the current event in the events vector */
        EventVec::iterator current_event_;

        /** @brief A pointer to the header of the current event */
        StreamHeader *current_header_;

        /** @brief A pointer to the vector of QuarterCore objects of the current event */
        std::vector<QuarterCore> *current_qcores_;

        QuarterCore qc_;
    };

};

#endif // RD53EVENT_H
