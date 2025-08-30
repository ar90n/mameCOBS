#include "../src/mameCOBS.hpp"
#include "utest.h"
#include <random>
#include <ranges>
#include <vector>

using namespace mamecobs;

namespace
{
  template <typename Range>
  std::vector<std::byte> collect_bytes(Range &&range)
  {
    std::vector<std::byte> result;
    for (auto frame_result : range)
    {
      if (frame_result.has_value())
      {
        auto frame = *frame_result;
        result.insert(result.end(), frame.begin(), frame.end());
      }
    }
    return result;
  }
} // namespace

UTEST(roundtrip, single_zero)
{
  std::vector<std::byte> original = { std::byte{ 0x00 } };

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, multiple_zeros)
{
  std::vector<std::byte> original = { std::byte{ 0x00 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x00 } };

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, no_zeros_254_bytes)
{
  // 254 bytes without any zeros - single COBS unit
  std::vector<std::byte> original;
  for (int i = 1; i <= 254; ++i)
  {
    original.push_back(std::byte{ static_cast<unsigned char>(i) });
  }

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, no_zeros_255_bytes)
{
  // 255 bytes without any zeros - requires multiple COBS units
  std::vector<std::byte> original;
  for (int i = 1; i <= 255; ++i)
  {
    original.push_back(std::byte{ static_cast<unsigned char>(i == 255 ? 8 : i) });
  }

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, large_frame_500_bytes)
{
  // Large frame with mixed zeros and data
  std::vector<std::byte> original;
  for (int i = 0; i < 500; ++i)
  {
    if (i % 50 == 0)
    {
      original.push_back(std::byte{ 0x00 }); // Zero every 50 bytes
    }
    else
    {
      original.push_back(std::byte{ static_cast<unsigned char>((i % 254) + 1) });
    }
  }

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, alternating_pattern_1000_bytes)
{
  // Alternating 0x00 and 0xFF for 1000 bytes
  std::vector<std::byte> original;
  for (int i = 0; i < 1000; ++i)
  {
    original.push_back(i % 2 == 0 ? std::byte{ 0x00 } : std::byte{ 0xFF });
  }

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, all_possible_bytes)
{
  // Test all possible byte values 0x00-0xFF
  std::vector<std::byte> original;
  for (int i = 0; i <= 255; ++i)
  {
    original.push_back(std::byte{ static_cast<unsigned char>(i) });
  }

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, boundary_253_254_255_256)
{
  // Test boundary cases around 254 bytes
  for (size_t size : { 253, 254, 255, 256 })
  {
    std::vector<std::byte> original;
    for (size_t i = 0; i < size; ++i)
    {
      // Avoid zeros to test pure data handling
      original.push_back(std::byte{ static_cast<unsigned char>((i % 254) + 1) });
    }

    auto encoded = original | encode(true);
    std::vector<std::byte> encoded_data;
    for (auto b : encoded)
    {
      encoded_data.push_back(b);
    }

    auto decoded = encoded_data | decode();
    std::vector<std::byte> result;
    for (auto frame_result : decoded)
    {
      ASSERT_TRUE(frame_result.has_value());
      if (frame_result)
      {
        auto frame = *frame_result;
        for (auto b : frame)
        {
          result.push_back(b);
        }
      }
    }

    ASSERT_EQ(result.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i)
    {
      ASSERT_EQ(result[i], original[i]);
    }
  }
}

UTEST(roundtrip, zeros_at_boundaries)
{
  // Zeros at exact 254-byte boundaries
  std::vector<std::byte> original;

  // 253 non-zero bytes
  for (int i = 1; i <= 253; ++i)
  {
    original.push_back(std::byte{ static_cast<unsigned char>(i) });
  }
  original.push_back(std::byte{ 0x00 }); // Zero at position 253 (254th byte)

  // Another 253 non-zero bytes
  for (int i = 1; i <= 253; ++i)
  {
    original.push_back(std::byte{ static_cast<unsigned char>(i) });
  }
  original.push_back(std::byte{ 0x00 }); // Zero at position 507

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, random_data_1024_bytes)
{
  // Random data test
  std::vector<std::byte> original;
  std::mt19937 gen(42); // Fixed seed for reproducible tests
  std::uniform_int_distribution<> dis(0, 255);

  for (int i = 0; i < 1024; ++i)
  {
    original.push_back(std::byte{ static_cast<unsigned char>(dis(gen)) });
  }

  auto result = collect_bytes(original | encode(true) | decode());

  ASSERT_EQ(result.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(result[i], original[i]);
  }
}

UTEST(roundtrip, multiple_frames_various_sizes)
{
  // Test multiple frames of various sizes
  std::vector<std::vector<std::byte>> original_frames = {
    {},                                                          // Empty frame
    { std::byte{ 0x42 } },                                       // Single byte
    { std::byte{ 0x00 } },                                       // Single zero
    { std::byte{ 0x11 }, std::byte{ 0x00 }, std::byte{ 0x22 } }, // Data with zero
  };

  // Add a large frame (300 bytes)
  std::vector<std::byte> large_frame;
  for (int i = 0; i < 300; ++i)
  {
    large_frame.push_back(std::byte{ static_cast<unsigned char>((i % 254) + 1) });
    if (i % 50 == 25)
    {
      large_frame.push_back(std::byte{ 0x00 }); // Occasional zeros
      ++i;
    }
  }
  original_frames.push_back(large_frame);

  auto encoded = original_frames | encode(true);
  std::vector<std::byte> encoded_data;
  for (auto b : encoded)
  {
    encoded_data.push_back(b);
  }

  auto decoded = encoded_data | decode();
  std::vector<std::vector<std::byte>> decoded_frames;
  for (auto frame_result : decoded)
  {
    ASSERT_TRUE(frame_result.has_value());
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      decoded_frames.push_back(frame_vec);
    }
  }

  ASSERT_EQ(decoded_frames.size(), original_frames.size());
  for (size_t frame_idx = 0; frame_idx < original_frames.size(); ++frame_idx)
  {
    ASSERT_EQ(decoded_frames[frame_idx].size(), original_frames[frame_idx].size());
    for (size_t byte_idx = 0; byte_idx < original_frames[frame_idx].size(); ++byte_idx)
    {
      ASSERT_EQ(decoded_frames[frame_idx][byte_idx], original_frames[frame_idx][byte_idx]);
    }
  }
}

UTEST(roundtrip, empty_frame)
{
  std::vector<std::byte> original = {};

  auto encoded = original | encode(true);
  std::vector<std::byte> encoded_data;
  for (auto b : encoded)
  {
    encoded_data.push_back(b);
  }

  auto decoded = encoded_data | decode();
  std::vector<std::vector<std::byte>> decoded_frames;
  for (auto frame_result : decoded)
  {
    ASSERT_TRUE(frame_result.has_value());
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      decoded_frames.push_back(frame_vec);
    }
  }

  ASSERT_EQ(decoded_frames.size(), static_cast<size_t>(1));
  ASSERT_EQ(decoded_frames[0].size(), static_cast<size_t>(0));
}