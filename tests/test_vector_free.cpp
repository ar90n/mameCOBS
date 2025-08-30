#include "../src/mameCOBS.hpp"
#include "utest.h"
#include <array>
#include <ranges>

using namespace mamecobs;

// Test single byte encoding without vectors
UTEST(mamecobs, vector_free_single_byte_encode)
{
  auto encoded = std::byte{ 0x42 } | encode(false); // No delimiter

  std::array<std::byte, 10> result;
  std::size_t pos = 0;

  for (auto b : encoded)
  {
    result[pos++] = b;
  }

  ASSERT_EQ(pos, static_cast<size_t>(2));
  ASSERT_EQ(result[0], std::byte{ 0x02 }); // Code
  ASSERT_EQ(result[1], std::byte{ 0x42 }); // Data
}

// Test Range<byte> encoding without vectors
UTEST(mamecobs, vector_free_range_encode)
{
  std::array<std::byte, 3> input = { std::byte{ 0x11 }, std::byte{ 0x22 }, std::byte{ 0x33 } };
  auto encoded = std::span(input) | encode(false); // No delimiter

  std::array<std::byte, 10> result;
  std::size_t pos = 0;

  for (auto b : encoded)
  {
    result[pos++] = b;
  }

  ASSERT_EQ(pos, static_cast<size_t>(4));
  ASSERT_EQ(result[0], std::byte{ 0x04 }); // Code for 3 data bytes + 1
  ASSERT_EQ(result[1], std::byte{ 0x11 }); // Data
  ASSERT_EQ(result[2], std::byte{ 0x22 }); // Data
  ASSERT_EQ(result[3], std::byte{ 0x33 }); // Data
}

// Test single byte decode without vectors (expects incomplete frame error)
UTEST(mamecobs, vector_free_single_byte_decode)
{
  // A single byte 0x42 is not a complete COBS frame
  // It would be a code byte expecting 65 data bytes to follow
  auto decoded = std::byte{ 0x42 } | decode<256>();

  for (auto frame_result : decoded)
  {
    // Should get an error (incomplete frame)
    ASSERT_FALSE(frame_result.has_value());

    if (!frame_result)
    {
      ASSERT_EQ(frame_result.error(), decode_error::incomplete);
    }
  }
}

// Test user-side heap-free usage with std::views::single
UTEST(mamecobs, user_heap_free_usage)
{
  // User creates frames without heap allocation
  std::array<std::byte, 2> frame1_data = { std::byte{ 0x11 }, std::byte{ 0x22 } };
  std::array<std::byte, 1> frame2_data = { std::byte{ 0x33 } };

  // Create views using std::views::single
  auto frame1 = std::span(frame1_data);
  auto frame2 = std::span(frame2_data);

  // Create frames collection without heap
  std::array<std::span<const std::byte>, 2> frames = { frame1, frame2 };

  // Encode without heap
  auto encoded = std::span(frames) | encode(true);

  // Verify encoding
  std::array<std::byte, 20> result;
  std::size_t pos = 0;

  for (auto b : encoded)
  {
    result[pos++] = b;
  }

  // Should be: [03 11 22 00] [02 33 00]
  ASSERT_GT(pos, static_cast<size_t>(4));
  ASSERT_EQ(result[0], std::byte{ 0x03 }); // Code for frame1
  ASSERT_EQ(result[1], std::byte{ 0x11 }); // frame1 data
  ASSERT_EQ(result[2], std::byte{ 0x22 }); // frame1 data
  ASSERT_EQ(result[3], std::byte{ 0x00 }); // Delimiter
}

// Test stack-based helper function
auto make_single_byte_frame(std::byte b)
{
  // Stack-based construction
  auto single_byte = std::views::single(b);
  return std::views::single(single_byte);
}

UTEST(mamecobs, stack_based_construction)
{
  auto frames = make_single_byte_frame(std::byte{ 0x55 });
  auto encoded = frames | encode(false);

  std::array<std::byte, 5> result;
  std::size_t pos = 0;

  for (auto b : encoded)
  {
    result[pos++] = b;
  }

  ASSERT_EQ(pos, static_cast<size_t>(2));
  ASSERT_EQ(result[0], std::byte{ 0x02 }); // Code
  ASSERT_EQ(result[1], std::byte{ 0x55 }); // Data
}