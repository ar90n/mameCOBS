# mameCOBS

[![CI](https://github.com/ar90n/mameCOBS/actions/workflows/ci.yml/badge.svg)](https://github.com/ar90n/mameCOBS/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Header Only](https://img.shields.io/badge/header--only-✓-brightgreen.svg)](src/mameCOBS.hpp)
![Built with vibe coding](https://img.shields.io/badge/built%20with-vibe%20coding-ff69b4)

A header-only C++23 library for COBS (Consistent Overhead Byte Stuffing) encoding and decoding.

## Features

- Single header only
- C++23 ranges/views based
- Type-safe API
- Zero heap allocation
- Composable with pipe operator
- Embedded systems friendly

## Requirements

- C++23 compatible compiler

## Installation

Copy `src/mameCOBS.hpp` to your project and include it.

```cpp
#include "mameCOBS.hpp"
```

## Usage

### Encoding

```cpp
#include "mameCOBS.hpp"
#include <array>
#include <span>

using namespace mamecobs;

// Direct pipeline encoding
std::array<std::byte, 4> data = {
    std::byte{0x11}, std::byte{0x22}, std::byte{0x00}, std::byte{0x33}
};

// Process encoded bytes directly
for (auto b : std::span(data) | encode(true)) {
    // Use encoded byte immediately
    send_byte(b);
}

// Single byte encoding
for (auto b : std::byte{0x42} | encode(false)) {
    process(b);
}
```

### Decoding

```cpp
// Direct frame processing
std::array<std::byte, 6> cobs_stream = {
    std::byte{0x03}, std::byte{0x11}, std::byte{0x22},
    std::byte{0x02}, std::byte{0x33}, std::byte{0x00}
};

for (auto frame : std::span(cobs_stream) | decode()) {
    if (frame) {
        for (auto b : frame.value()) {
            process_decoded_byte(b);
        }
    } else {
        handle_error(frame.error());
    }
}
```

### Composing Ranges

```cpp
// Multiple frames without intermediate storage
auto frames = std::views::single(std::span(frame1))
            | std::views::concat(std::views::single(std::span(frame2)));

for (auto b : frames | encode(true)) {
    transmit(b);
}

// Round-trip without materialization
for (auto frame : data | encode(true) | decode()) {
    // Process round-tripped frame
}
```

## Building Examples and Tests

```bash
# Build everything
make

# Build samples only
make samples

# Run tests
make test

# Format code
make format
```

## API

### encode(bool append_delimiter = true)

Creates an encoder adapter for COBS encoding.
- Input: Range of bytes or Range of Range of bytes
- Output: Range of encoded bytes
- `append_delimiter`: Append 0x00 delimiter after each frame

### decode<MaxFrameSize = 4096>()

Creates a decoder adapter for COBS decoding.
- Input: Range of bytes
- Output: Range of `std::expected<frame, error>`
- `MaxFrameSize`: Maximum frame size in bytes

### Error Types

```cpp
enum class decode_error {
    oversized,    // Frame exceeds MaxFrameSize
    invalid_cobs, // Invalid COBS structure
    incomplete    // Incomplete frame
};
```

## License

Apache License 2.0 - See LICENSE file for details.
