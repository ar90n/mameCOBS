#include "../src/mameCOBS.hpp"
#include "utest.h"
#include <ranges>
#include <vector>

using namespace mamecobs;

// Wikipedia COBS examples: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

UTEST(encode, wikipedia_single_zero)
{
  // [0x00] -> [0x01, 0x01]
  std::vector<std::byte> input = { std::byte{ 0x00 } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x01 }, std::byte{ 0x01 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, wikipedia_two_zeros)
{
  // [0x00, 0x00] -> [0x01, 0x01, 0x01]
  std::vector<std::byte> input = { std::byte{ 0x00 }, std::byte{ 0x00 } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x01 }, std::byte{ 0x01 }, std::byte{ 0x01 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, wikipedia_data_with_zero)
{
  // [0x11, 0x22, 0x00, 0x33] -> [0x03, 0x11, 0x22, 0x02, 0x33]
  std::vector<std::byte> input = { std::byte{ 0x11 },
                                   std::byte{ 0x22 },
                                   std::byte{ 0x00 },
                                   std::byte{ 0x33 } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x03 },
                                      std::byte{ 0x11 },
                                      std::byte{ 0x22 },
                                      std::byte{ 0x02 },
                                      std::byte{ 0x33 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, wikipedia_no_zeros)
{
  // [0x11, 0x22, 0x33, 0x44] -> [0x05, 0x11, 0x22, 0x33, 0x44]
  std::vector<std::byte> input = { std::byte{ 0x11 },
                                   std::byte{ 0x22 },
                                   std::byte{ 0x33 },
                                   std::byte{ 0x44 } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x05 },
                                      std::byte{ 0x11 },
                                      std::byte{ 0x22 },
                                      std::byte{ 0x33 },
                                      std::byte{ 0x44 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, wikipedia_data_then_zeros)
{
  // [0x11, 0x00, 0x00, 0x00] -> [0x02, 0x11, 0x01, 0x01, 0x01]
  std::vector<std::byte> input = { std::byte{ 0x11 },
                                   std::byte{ 0x00 },
                                   std::byte{ 0x00 },
                                   std::byte{ 0x00 } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x02 },
                                      std::byte{ 0x11 },
                                      std::byte{ 0x01 },
                                      std::byte{ 0x01 },
                                      std::byte{ 0x01 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, max_unit_254_bytes)
{
  // 254 bytes from 0x01 to 0xFE -> [0xFF, 0x01, 0x02, ..., 0xFE]
  std::vector<std::byte> input;
  for (int i = 1; i <= 254; ++i)
  {
    input.push_back(std::byte{ static_cast<unsigned char>(i) });
  }

  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  ASSERT_EQ(result.size(), static_cast<size_t>(255));
  ASSERT_EQ(result[0], std::byte{ 0xFF }); // Code for 254 data bytes

  for (size_t i = 1; i <= 254; ++i)
  {
    ASSERT_EQ(result[i], std::byte{ static_cast<unsigned char>(i) });
  }
}

UTEST(encode, zero_then_254_bytes)
{
  // [0x00, 0x01, ..., 0xFE] -> [0x01, 0xFF, 0x01, ..., 0xFE]
  std::vector<std::byte> input;
  input.push_back(std::byte{ 0x00 });
  for (int i = 1; i <= 254; ++i)
  {
    input.push_back(std::byte{ static_cast<unsigned char>(i) });
  }

  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  ASSERT_EQ(result.size(), static_cast<size_t>(256));
  ASSERT_EQ(result[0], std::byte{ 0x01 }); // Code for zero
  ASSERT_EQ(result[1], std::byte{ 0xFF }); // Code for 254 bytes

  for (int i = 1; i <= 254; ++i)
  {
    ASSERT_EQ(result[i + 1], std::byte{ static_cast<unsigned char>(i) });
  }
}

UTEST(encode, empty_input)
{
  std::vector<std::byte> input = {};
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  // Empty input should produce [0x01]
  std::vector<std::byte> expected = { std::byte{ 0x01 } };

  ASSERT_EQ(result.size(), expected.size());
  ASSERT_EQ(result[0], expected[0]);
}

UTEST(encode, single_byte)
{
  std::vector<std::byte> input = { std::byte{ 0x42 } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x02 }, std::byte{ 0x42 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, with_delimiter)
{
  std::vector<std::byte> input = { std::byte{ 0x11 }, std::byte{ 0x22 } };
  auto encoded = input | encode(true); // With delimiter

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  // Should end with 0x00 delimiter
  ASSERT_GT(result.size(), static_cast<size_t>(0));
  ASSERT_EQ(result.back(), std::byte{ 0x00 });

  // Without delimiter should be [0x03, 0x11, 0x22], with delimiter adds 0x00
  std::vector<std::byte> expected = { std::byte{ 0x03 },
                                      std::byte{ 0x11 },
                                      std::byte{ 0x22 },
                                      std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, multiple_frames)
{
  // Test encoding multiple frames at once
  std::vector<std::vector<std::byte>> frames = { { std::byte{ 0x11 } },
                                                 { std::byte{ 0x22 }, std::byte{ 0x33 } },
                                                 { std::byte{ 0x44 } } };

  auto encoded = frames | encode(true); // With delimiters

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  // Expected: [0x02, 0x11, 0x00, 0x03, 0x22, 0x33, 0x00, 0x02, 0x44, 0x00]
  std::vector<std::byte> expected = { std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x00 },
                                      std::byte{ 0x03 }, std::byte{ 0x22 }, std::byte{ 0x33 },
                                      std::byte{ 0x00 }, std::byte{ 0x02 }, std::byte{ 0x44 },
                                      std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(encode, alternating_zeros_and_data)
{
  // [0x00, 0xFF, 0x00, 0xAA] -> [0x01, 0x02, 0xFF, 0x02, 0xAA]
  std::vector<std::byte> input = { std::byte{ 0x00 },
                                   std::byte{ 0xFF },
                                   std::byte{ 0x00 },
                                   std::byte{ 0xAA } };
  auto encoded = input | encode(false);

  std::vector<std::byte> result;
  for (auto b : encoded)
  {
    result.push_back(b);
  }

  std::vector<std::byte> expected = { std::byte{ 0x01 },
                                      std::byte{ 0x02 },
                                      std::byte{ 0xFF },
                                      std::byte{ 0x02 },
                                      std::byte{ 0xAA } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}