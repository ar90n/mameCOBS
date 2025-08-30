#include "../src/mameCOBS.hpp"
#include "utest.h"
#include <ranges>
#include <vector>

using namespace mamecobs;

// Wikipedia COBS examples: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

UTEST(decode, wikipedia_single_zero)
{
  // [0x01, 0x01, 0x00] -> [0x00]
  std::vector<std::byte> input = { std::byte{ 0x01 }, std::byte{ 0x01 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(decode, wikipedia_two_zeros)
{
  // [0x01, 0x01, 0x01, 0x00] -> [0x00, 0x00]
  std::vector<std::byte> input = { std::byte{ 0x01 },
                                   std::byte{ 0x01 },
                                   std::byte{ 0x01 },
                                   std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x00 }, std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(decode, wikipedia_data_with_zero)
{
  // [0x03, 0x11, 0x22, 0x02, 0x33, 0x00] -> [0x11, 0x22, 0x00, 0x33]
  std::vector<std::byte> input = { std::byte{ 0x03 }, std::byte{ 0x11 }, std::byte{ 0x22 },
                                   std::byte{ 0x02 }, std::byte{ 0x33 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x11 },
                                      std::byte{ 0x22 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x33 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(decode, wikipedia_no_zeros)
{
  // [0x05, 0x11, 0x22, 0x33, 0x44, 0x00] -> [0x11, 0x22, 0x33, 0x44]
  std::vector<std::byte> input = { std::byte{ 0x05 }, std::byte{ 0x11 }, std::byte{ 0x22 },
                                   std::byte{ 0x33 }, std::byte{ 0x44 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x11 },
                                      std::byte{ 0x22 },
                                      std::byte{ 0x33 },
                                      std::byte{ 0x44 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(decode, wikipedia_data_then_zeros)
{
  // [0x02, 0x11, 0x01, 0x01, 0x01, 0x00] -> [0x11, 0x00, 0x00, 0x00]
  std::vector<std::byte> input = { std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x01 },
                                   std::byte{ 0x01 }, std::byte{ 0x01 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x11 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(decode, max_unit_254_bytes)
{
  // [0xFF, 0x01, 0x02, ..., 0xFE, 0x00] -> [0x01, 0x02, ..., 0xFE]
  std::vector<std::byte> input;
  input.push_back(std::byte{ 0xFF }); // Code for 254 bytes
  for (int i = 1; i <= 254; ++i)
  {
    input.push_back(std::byte{ static_cast<unsigned char>(i) });
  }
  input.push_back(std::byte{ 0x00 }); // Delimiter

  auto decoded = input | decode();

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

  ASSERT_EQ(result.size(), static_cast<size_t>(254));
  for (size_t i = 0; i < 254; ++i)
  {
    ASSERT_EQ(result[i], std::byte{ static_cast<unsigned char>(i + 1) });
  }
}

UTEST(decode, zero_then_254_bytes)
{
  // [0x01, 0xFF, 0x01, ..., 0xFE, 0x00] -> [0x00, 0x01, ..., 0xFE]
  std::vector<std::byte> input;
  input.push_back(std::byte{ 0x01 }); // Code for zero
  input.push_back(std::byte{ 0xFF }); // Code for 254 bytes
  for (int i = 1; i <= 254; ++i)
  {
    input.push_back(std::byte{ static_cast<unsigned char>(i) });
  }
  input.push_back(std::byte{ 0x00 }); // Delimiter

  auto decoded = input | decode();

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

  ASSERT_EQ(result.size(), static_cast<size_t>(255));
  ASSERT_EQ(result[0], std::byte{ 0x00 });
  for (size_t i = 1; i <= 254; ++i)
  {
    ASSERT_EQ(result[i], std::byte{ static_cast<unsigned char>(i) });
  }
}

UTEST(decode, empty_frame)
{
  // [0x01, 0x00] -> []
  std::vector<std::byte> input = { std::byte{ 0x01 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

  std::vector<std::vector<std::byte>> frames;
  for (auto frame_result : decoded)
  {
    ASSERT_TRUE(frame_result.has_value());
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      frames.push_back(frame_vec);
    }
  }

  ASSERT_EQ(frames.size(), static_cast<size_t>(1));
  ASSERT_EQ(frames[0].size(), static_cast<size_t>(0)); // Empty frame
}

UTEST(decode, single_byte)
{
  // [0x02, 0x42, 0x00] -> [0x42]
  std::vector<std::byte> input = { std::byte{ 0x02 }, std::byte{ 0x42 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x42 } };

  ASSERT_EQ(result.size(), expected.size());
  ASSERT_EQ(result[0], expected[0]);
}

UTEST(decode, multiple_frames)
{
  // [0x02, 0x11, 0x00, 0x03, 0x22, 0x33, 0x00, 0x02, 0x44, 0x00]
  // -> Frame 1: [0x11], Frame 2: [0x22, 0x33], Frame 3: [0x44]
  std::vector<std::byte> input = { std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x00 }, std::byte{ 0x03 },
                                   std::byte{ 0x22 }, std::byte{ 0x33 }, std::byte{ 0x00 }, std::byte{ 0x02 },
                                   std::byte{ 0x44 }, std::byte{ 0x00 } };

  auto decoded = input | decode();

  std::vector<std::vector<std::byte>> frames;
  for (auto frame_result : decoded)
  {
    ASSERT_TRUE(frame_result.has_value());
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      frames.push_back(frame_vec);
    }
  }

  ASSERT_EQ(frames.size(), static_cast<size_t>(3));

  // Frame 1: [0x11]
  ASSERT_EQ(frames[0].size(), static_cast<size_t>(1));
  ASSERT_EQ(frames[0][0], std::byte{ 0x11 });

  // Frame 2: [0x22, 0x33]
  ASSERT_EQ(frames[1].size(), static_cast<size_t>(2));
  ASSERT_EQ(frames[1][0], std::byte{ 0x22 });
  ASSERT_EQ(frames[1][1], std::byte{ 0x33 });

  // Frame 3: [0x44]
  ASSERT_EQ(frames[2].size(), static_cast<size_t>(1));
  ASSERT_EQ(frames[2][0], std::byte{ 0x44 });
}

UTEST(decode, alternating_zeros_and_data)
{
  // [0x01, 0x02, 0xFF, 0x01, 0x02, 0xAA, 0x00] -> [0x00, 0xFF, 0x00, 0xAA]
  std::vector<std::byte> input = { std::byte{ 0x01 }, std::byte{ 0x02 }, std::byte{ 0xFF }, std::byte{ 0x01 },
                                   std::byte{ 0x02 }, std::byte{ 0xAA }, std::byte{ 0x00 } };
  auto decoded = input | decode();

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

  std::vector<std::byte> expected = { std::byte{ 0x00 },
                                      std::byte{ 0xFF },
                                      std::byte{ 0x00 },
                                      std::byte{ 0x00 },
                                      std::byte{ 0xAA } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(decode, error_invalid_code)
{
  // Invalid COBS code (0x00 as code byte)
  std::vector<std::byte> input = { std::byte{ 0x00 }, std::byte{ 0x11 }, std::byte{ 0x00 } };
  auto decoded = input | decode();

  bool found_error = false;
  for (auto frame_result : decoded)
  {
    if (!frame_result.has_value())
    {
      found_error = true;
      ASSERT_EQ(frame_result.error(), decode_error::invalid_cobs);
    }
  }

  ASSERT_TRUE(found_error);
}

UTEST(decode, error_oversized_frame)
{
  // Try to decode a frame larger than max size (using small max size for test)
  std::vector<std::byte> input;
  input.push_back(std::byte{ 0xFF }); // Code for 254 bytes
  for (int i = 0; i < 254; ++i)
  {
    input.push_back(std::byte{ 0x11 });
  }
  input.push_back(std::byte{ 0x00 });

  auto decoded = input | decode<100>(); // Max frame size = 100

  bool found_error = false;
  for (auto frame_result : decoded)
  {
    if (!frame_result.has_value())
    {
      found_error = true;
      ASSERT_EQ(frame_result.error(), decode_error::oversized);
    }
  }

  ASSERT_TRUE(found_error);
}

UTEST(decode, with_size_limit)
{
  // Create a frame that would exceed 10 bytes
  std::vector<std::byte> encoded;
  encoded.push_back(std::byte{ 20 }); // Code for 19 data bytes
  for (int i = 0; i < 19; ++i)
  {
    encoded.push_back(std::byte{ 0x42 });
  }
  encoded.push_back(std::byte{ 0x00 }); // Delimiter

  // Decode with 10 byte limit
  auto decoded_frames = encoded | decode<10>();

  for (auto frame_result : decoded_frames)
  {
    ASSERT_FALSE(frame_result.has_value());
    ASSERT_EQ(frame_result.error(), decode_error::oversized);
  }
}

UTEST(decode, error_recovery)
{
  // Invalid frame followed by valid frame
  std::vector<std::byte> encoded = {
    std::byte{ 0x00 }, // Delimiter (empty frame)
    std::byte{ 0x03 },
    std::byte{ 0xAA },
    std::byte{ 0xBB },
    std::byte{ 0x00 } // Valid frame
  };

  auto decoded_frames = encoded | decode();
  std::vector<bool> results;
  std::vector<size_t> sizes;

  for (auto frame_result : decoded_frames)
  {
    results.push_back(frame_result.has_value());
    if (frame_result)
    {
      sizes.push_back((*frame_result).size());
    }
  }

  ASSERT_EQ(results.size(), static_cast<size_t>(2));
  ASSERT_TRUE(results[0]);                     // First frame is empty but valid
  ASSERT_TRUE(results[1]);                     // Second frame is valid
  ASSERT_EQ(sizes[0], static_cast<size_t>(0)); // Empty frame
  ASSERT_EQ(sizes[1], static_cast<size_t>(2)); // Two bytes
}