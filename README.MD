# RD53 Event Decoder Library

A C++ and Python library for decoding and handling event data from the RD53 readout chip, commonly used in high-energy physics experiments for pixel detectors.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Building the C++ Library](#building-the-c-library)
  - [Building the Python Bindings](#building-the-python-bindings)
- [Usage](#usage)
  - [C++ Usage](#c-usage)
  - [Python Usage](#python-usage)
- [Classes and Structures](#classes-and-structures)
  - [StreamHeader](#streamheader)
  - [StreamConfig](#streamconfig)
  - [QuarterCore](#quartercore)
  - [Event](#event)
  - [Decoder](#decoder)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Introduction

The RD53 Event Decoder Library is designed to parse and interpret event data streams from the RD53 readout chip. The RD53 chip is integral in pixel detectors for high-energy physics experiments, such as those conducted at the Large Hadron Collider (LHC). This library provides classes and methods to decode raw data into meaningful structures, facilitating data analysis and processing.

Now, with Python bindings using [pybind11](https://github.com/pybind/pybind11), you can access the RD53 event structures and functionalities directly from Python.

## Features

- **Event Decoding**: Convert raw data streams into structured event data.
- **Quarter Core Representation**: Model individual quarter cores of the RD53 chip.
- **Hit Mapping**: Access and manipulate hit data within events.
- **Stream Configuration**: Customize decoding based on specific stream settings.
- **Serialization**: Serialize and deserialize event data for storage or transmission.
- **Python Bindings**: Use the library directly from Python for easier integration and scripting.

## Installation

### Prerequisites

- **C++ Compiler**: Ensure you have a C++11 (or later) compatible compiler.
- **CMake**: For building the project.
- **Python 3.x**: Required for Python bindings.
- **pybind11**: Library for creating Python bindings of C++ code (optional, needed for Python bindings).

### Building the C++ Library

1. **Clone the Repository**

   ```bash
   git clone https://github.com/yourusername/rd53-event-decoder.git
   cd rd53-event-decoder
   ```

2. **Create Build Directory and Build**

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

   This will build the C++ library.

3. **Install the Library (Optional)**

   You can install the library system-wide (e.g., in `/usr/local/`):

   ```bash
   sudo make install
   ```

   Alternatively, you can use the built files directly by linking them in your project.

### Building the Python Bindings

To build the Python bindings, follow these additional steps:

1. **Install Dependencies**

   Make sure `pybind11` is available. You can install it via `pip`:

   ```bash
   pip install pybind11 pybind11-stubgen
   ```

2. **Configure and Build the Python Bindings**

   Run the following commands in the `build` directory:

   ```bash
   cmake .. -DPYTHON_EXECUTABLE=$(which python)
   make RD53Eventpy
   # optional packaging and install
   make package-python
   make install-python
   ```
   
3. **Install the Python Module**

   You can install the generated Python module system-wide or in your environment:

   ```bash
   pip install .
   ```

## Usage

### C++ Usage

#### Basic Example

```cpp
#include "RD53Event.h"

int main() {
    // Configure the stream settings
    RD53::StreamConfig config(4, 4, true, false, false, false, true, true);

    // Create a decoder instance with raw data
    std::vector<RD53::word_t> raw_data = {/* Raw event data */};
    RD53::Decoder decoder(config, raw_data);

    // Process the data stream
    decoder.process_stream();

    // Retrieve decoded events
    std::vector<RD53::Event> events = decoder.get_events();

    // Iterate and analyze events
    for (const auto& event : events) {
        std::cout << event.as_str() << std::endl;
    }

    return 0;
}
```

#### Handling Quarter Cores

```cpp
// Create a QuarterCore instance
RD53::QuarterCore qcore(config, 5, 10);

// Set a hit at position (2, 3) with a time-over-threshold (ToT) of 15
qcore.set_hit(2, 3, 15);

// Retrieve hit information
auto hit = qcore.get_hit(2, 3);
if (hit.first) {
    std::cout << "Hit at (2,3) with ToT: " << static_cast<int>(hit.second) << std::endl;
}
```

### Python Usage

With the Python bindings, you can access the RD53Event functionalities directly from Python.

#### Importing the Module

```python
import RD53Eventpy as RD53
```

#### Basic Example

```python
# Configure the stream settings
config = RD53.StreamConfig(
    qcore_vertical=4,
    qcore_horizontal=4,
    _chip_id=True,
    _drop_tot=False,
    _compressed_hitmap=False,
    _eos_marker=False,
    _bcid=True,
    _l1id=True
)

# Create a decoder instance with raw data
raw_data = [...]  # Raw event data as a list of integers
decoder = RD53.Decoder(config, raw_data)

# Process the data stream
decoder.process_stream()

# Retrieve decoded events
events = decoder.get_events()

# Iterate and analyze events
for event in events:
    print(event.as_str())
```

#### Handling Quarter Cores

```python
# Create a QuarterCore instance
qcore = RD53.QuarterCore(config, col=5, row=10)

# Set a hit at position (2, 3) with a time-over-threshold (ToT) of 15
qcore.set_hit(2, 3, 15)

# Retrieve hit information
hit_exists, tot_value = qcore.get_hit(2, 3)
if hit_exists:
    print(f"Hit at (2,3) with ToT: {tot_value}")
```

## Classes and Structures

### StreamHeader

Represents the header information of an event data stream.

- **Members**:
  - `trigger_tag`: Identifier for the trigger.
  - `trigger_pos`: Position of the trigger.
  - `chip_id`: Identifier for the chip.
  - `bcid`: Bunch Crossing ID.
  - `l1id`: Level-1 Trigger ID.

### StreamConfig

Configuration parameters for decoding the event data stream.

- **Members**:
  - `size_qcore_vertical`: Vertical size of quarter cores.
  - `size_qcore_horizontal`: Horizontal size of quarter cores.
  - `chip_id`: Include chip ID in decoding.
  - `drop_tot`: Exclude Time-over-Threshold information.
  - `compressed_hitmap`: Use compressed hit mapping.
  - `eos_marker`: End-of-stream marker.
  - `bcid`: Include Bunch Crossing ID.
  - `l1id`: Include Level-1 Trigger ID.
  - `events_per_stream`: Number of events per data stream.

### QuarterCore

Represents a quarter core section of the RD53 chip.

- **Methods**:
  - `get_hit(col, row)`: Retrieve hit information at a specific position.
  - `set_hit(col, row, tot)`: Set a hit with ToT at a specific position.
  - `serialize_qcore(prev_last_in_col)`: Serialize quarter core data.
  - `as_str()`: String representation of the quarter core.

### Event

Represents a single event containing multiple quarter cores.

- **Methods**:
  - `serialize_event()`: Serialize the entire event.
  - `get_qcores()`: Retrieve quarter cores in the event.
  - `get_hits()`: Retrieve all hits in the event.
  - `as_str()`: String representation of the event.

### Decoder

Decodes raw data streams into structured events.

- **Methods**:
  - `process_stream()`: Decode the entire data stream.
  - `get_events()`: Retrieve decoded events.

## Contributing

Contributions are welcome! Please follow these steps:

1. **Fork the Repository**

   Click on the 'Fork' button at the top right of the repository page.

2. **Create a Feature Branch**

   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Commit Your Changes**

   ```bash
   git commit -am 'Add new feature'
   ```

4. **Push to Your Fork**

   ```bash
   git push origin feature/your-feature-name
   ```

5. **Submit a Pull Request**

   Open a pull request to the main repository's `develop` branch.

