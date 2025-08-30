#include "../src/mameCOBS.hpp"
#include "utest.h"
#include <ranges>
#include <vector>

using namespace mamecobs;

// Test true incremental encoding - reusing encoder object for individual frames

UTEST(mamecobs, true_incremental_encode)
{
  auto enc = encode(true); // With delimiters
  std::vector<std::byte> result;

  // Encode frame1 individually using enc
  std::vector<std::byte> frame1 = { std::byte{ 0x11 } };
  for (auto b : frame1 | enc)
  {
    result.push_back(b);
  }

  // Reuse the same enc to encode frame2 individually
  std::vector<std::byte> frame2 = { std::byte{ 0x22 } };
  for (auto b : frame2 | enc)
  {
    result.push_back(b);
  }

  // Should have: [0x02, 0x11, 0x00, 0x02, 0x22, 0x00]
  std::vector<std::byte> expected = { std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x00 },
                                      std::byte{ 0x02 }, std::byte{ 0x22 }, std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(mamecobs, batch_encode_multiple_frames)
{
  // Test batch processing - multiple frames at once
  std::vector<std::vector<std::byte>> frames = { { std::byte{ 0x11 } }, { std::byte{ 0x22 } } };

  auto enc = encode(true); // With delimiters
  std::vector<std::byte> result;

  for (auto b : frames | enc)
  {
    result.push_back(b);
  }

  // Should have: [0x02, 0x11, 0x00, 0x02, 0x22, 0x00]
  std::vector<std::byte> expected = { std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x00 },
                                      std::byte{ 0x02 }, std::byte{ 0x22 }, std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(mamecobs, batch_encode_with_zeros)
{
  // Test batch encoding of frames with zeros
  std::vector<std::vector<std::byte>> frames = {
    { std::byte{ 0x00 } },
    { std::byte{ 0x11 }, std::byte{ 0x00 }, std::byte{ 0x22 } }
  };

  auto enc = encode(true);
  std::vector<std::byte> result;

  for (auto b : frames | enc)
  {
    result.push_back(b);
  }

  // Frame 1: [0x00] -> [0x01, 0x01, 0x00]
  // Frame 2: [0x11, 0x00, 0x22] -> [0x02, 0x11, 0x02, 0x22, 0x00]
  std::vector<std::byte> expected = { std::byte{ 0x01 }, std::byte{ 0x01 }, std::byte{ 0x00 },
                                      std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x02 },
                                      std::byte{ 0x22 }, std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(mamecobs, batch_encode_chunks)
{
  // Test batch encoding of larger chunks as frames
  std::vector<std::vector<std::byte>> frames = { { std::byte{ 0x01 }, std::byte{ 0x02 }, std::byte{ 0x03 } },
                                                 { std::byte{ 0x04 }, std::byte{ 0x05 } } };

  auto enc = encode(false); // No trailing delimiter
  std::vector<std::byte> result;

  for (auto b : frames | enc)
  {
    result.push_back(b);
  }

  // Frame 1: [0x01, 0x02, 0x03] -> [0x04, 0x01, 0x02, 0x03, 0x00]
  // Frame 2: [0x04, 0x05] -> [0x03, 0x04, 0x05] (no trailing delimiter)
  std::vector<std::byte> expected = { std::byte{ 0x04 }, std::byte{ 0x01 }, std::byte{ 0x02 },
                                      std::byte{ 0x03 }, std::byte{ 0x00 }, std::byte{ 0x03 },
                                      std::byte{ 0x04 }, std::byte{ 0x05 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(mamecobs, encode_with_delimiter_option)
{
  // Test that delimiter is properly added/removed
  std::vector<std::vector<std::byte>> frames = { { std::byte{ 0xAA } } };

  // With delimiter
  auto enc_with = encode(true);
  std::vector<std::byte> with_delim;
  for (auto b : frames | enc_with)
  {
    with_delim.push_back(b);
  }

  // Without delimiter
  auto enc_without = encode(false);
  std::vector<std::byte> without_delim;
  for (auto b : frames | enc_without)
  {
    without_delim.push_back(b);
  }

  // With delimiter: [0x02, 0xAA, 0x00]
  ASSERT_EQ(with_delim.size(), static_cast<size_t>(3));
  ASSERT_EQ(with_delim[2], std::byte{ 0x00 });

  // Without delimiter: [0x02, 0xAA]
  ASSERT_EQ(without_delim.size(), static_cast<size_t>(2));
}

UTEST(mamecobs, decode_stateless)
{
  // The decoder is stateless - each call processes the entire input
  auto dec = decode();

  // Complete COBS frames
  std::vector<std::byte> encoded = { std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x00 },
                                     std::byte{ 0x02 }, std::byte{ 0x22 }, std::byte{ 0x00 } };

  std::vector<std::vector<std::byte>> frames;
  for (auto frame_result : encoded | dec)
  {
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      frames.push_back(frame_vec);
    }
  }

  // Should decode to two frames
  ASSERT_EQ(frames.size(), static_cast<size_t>(2));
  ASSERT_EQ(frames[0].size(), static_cast<size_t>(1));
  ASSERT_EQ(frames[0][0], std::byte{ 0x11 });
  ASSERT_EQ(frames[1].size(), static_cast<size_t>(1));
  ASSERT_EQ(frames[1][0], std::byte{ 0x22 });
}

UTEST(mamecobs, decode_multiple_chunks)
{
  // Decode multiple frames from a single stream
  std::vector<std::byte> encoded = { std::byte{ 0x04 }, std::byte{ 0x01 }, std::byte{ 0x02 },
                                     std::byte{ 0x03 }, std::byte{ 0x00 }, std::byte{ 0x03 },
                                     std::byte{ 0x04 }, std::byte{ 0x05 }, std::byte{ 0x00 } };

  auto dec = decode();
  std::vector<std::vector<std::byte>> frames;

  for (auto frame_result : encoded | dec)
  {
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      frames.push_back(frame_vec);
    }
  }

  // Should decode to two frames
  ASSERT_EQ(frames.size(), static_cast<size_t>(2));

  // Frame 1: [0x01, 0x02, 0x03]
  ASSERT_EQ(frames[0].size(), static_cast<size_t>(3));
  ASSERT_EQ(frames[0][0], std::byte{ 0x01 });
  ASSERT_EQ(frames[0][1], std::byte{ 0x02 });
  ASSERT_EQ(frames[0][2], std::byte{ 0x03 });

  // Frame 2: [0x04, 0x05]
  ASSERT_EQ(frames[1].size(), static_cast<size_t>(2));
  ASSERT_EQ(frames[1][0], std::byte{ 0x04 });
  ASSERT_EQ(frames[1][1], std::byte{ 0x05 });
}

UTEST(mamecobs, roundtrip_multiple_frames)
{
  // Test roundtrip with multiple frames
  std::vector<std::vector<std::byte>> original = { { std::byte{ 0xFF }, std::byte{ 0xFE } },
                                                   { std::byte{ 0x00 }, std::byte{ 0x01 } },
                                                   { std::byte{ 0xAA } } };

  // Encode
  auto enc = encode();
  std::vector<std::byte> encoded;
  for (auto b : original | enc)
  {
    encoded.push_back(b);
  }

  // Decode
  auto dec = decode();
  std::vector<std::vector<std::byte>> decoded;
  for (auto frame_result : encoded | dec)
  {
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      decoded.push_back(frame_vec);
    }
  }

  // Verify roundtrip
  ASSERT_EQ(decoded.size(), original.size());
  for (size_t i = 0; i < original.size(); ++i)
  {
    ASSERT_EQ(decoded[i].size(), original[i].size());
    for (size_t j = 0; j < original[i].size(); ++j)
    {
      ASSERT_EQ(decoded[i][j], original[i][j]);
    }
  }
}

UTEST(mamecobs, preserve_frame_boundaries)
{
  // Test that frame boundaries are preserved
  std::vector<std::vector<std::byte>> original = { { std::byte{ 0x11 } },
                                                   {}, // Empty frame
                                                   { std::byte{ 0x22 } } };

  // Encode
  auto enc = encode();
  std::vector<std::byte> encoded;
  for (auto b : original | enc)
  {
    encoded.push_back(b);
  }

  // Decode
  auto dec = decode();
  std::vector<std::vector<std::byte>> decoded;
  for (auto frame_result : encoded | dec)
  {
    if (frame_result)
    {
      auto frame = *frame_result;
      std::vector<std::byte> frame_vec(frame.begin(), frame.end());
      decoded.push_back(frame_vec);
    }
  }

  // Should preserve all frames including empty one
  ASSERT_EQ(decoded.size(), static_cast<size_t>(3));
  ASSERT_EQ(decoded[0].size(), static_cast<size_t>(1));
  ASSERT_EQ(decoded[0][0], std::byte{ 0x11 });
  ASSERT_EQ(decoded[1].size(), static_cast<size_t>(0)); // Empty
  ASSERT_EQ(decoded[2].size(), static_cast<size_t>(1));
  ASSERT_EQ(decoded[2][0], std::byte{ 0x22 });
}

UTEST(mamecobs, true_incremental_encode_with_zeros)
{
  // True incremental encoding - reusing encoder for frames with zeros
  auto enc = encode(true); // With delimiters
  std::vector<std::byte> result;

  // Encode first frame with zero
  std::vector<std::byte> frame1 = { std::byte{ 0x00 } };
  for (auto b : frame1 | enc)
  {
    result.push_back(b);
  }

  // Reuse encoder for frame with zero in middle
  std::vector<std::byte> frame2 = { std::byte{ 0x11 }, std::byte{ 0x00 }, std::byte{ 0x22 } };
  for (auto b : frame2 | enc)
  {
    result.push_back(b);
  }

  // Expected: [0x01, 0x01, 0x00, 0x02, 0x11, 0x02, 0x22, 0x00]
  std::vector<std::byte> expected = { std::byte{ 0x01 }, std::byte{ 0x01 }, std::byte{ 0x00 },
                                      std::byte{ 0x02 }, std::byte{ 0x11 }, std::byte{ 0x02 },
                                      std::byte{ 0x22 }, std::byte{ 0x00 } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(mamecobs, true_incremental_encode_different_sizes)
{
  // True incremental encoding - frames of different sizes
  auto enc = encode(false); // No delimiters for last frame
  std::vector<std::byte> result;

  // Encode single byte frame
  std::vector<std::byte> frame1 = { std::byte{ 0xAA } };
  for (auto b : frame1 | enc)
  {
    result.push_back(b);
  }

  // Reuse encoder for larger frame
  std::vector<std::byte> frame2 = { std::byte{ 0xBB }, std::byte{ 0xCC }, std::byte{ 0xDD } };
  for (auto b : frame2 | enc)
  {
    result.push_back(b);
  }

  // Expected: [0x02, 0xAA, 0x04, 0xBB, 0xCC, 0xDD] (each frame encoded individually, no delimiters)
  std::vector<std::byte> expected = { std::byte{ 0x02 }, std::byte{ 0xAA }, std::byte{ 0x04 },
                                      std::byte{ 0xBB }, std::byte{ 0xCC }, std::byte{ 0xDD } };

  ASSERT_EQ(result.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i)
  {
    ASSERT_EQ(result[i], expected[i]);
  }
}

UTEST(mamecobs, true_incremental_encode_many_frames)
{
  // Test reusing encoder for many frames
  auto enc = encode(true);
  std::vector<std::byte> result;

  // Encode multiple single-byte frames
  for (int i = 1; i <= 5; ++i)
  {
    std::vector<std::byte> frame = { std::byte{ static_cast<unsigned char>(0x10 + i) } };
    for (auto b : frame | enc)
    {
      result.push_back(b);
    }
  }

  // Expected 5 frames: each [0x02, 0x1x, 0x00]
  ASSERT_EQ(result.size(), static_cast<size_t>(15)); // 5 frames * 3 bytes each

  for (int i = 0; i < 5; ++i)
  {
    size_t base = i * 3;
    ASSERT_EQ(result[base], std::byte{ 0x02 });                                     // Code
    ASSERT_EQ(result[base + 1], std::byte{ static_cast<unsigned char>(0x11 + i) }); // Data
    ASSERT_EQ(result[base + 2], std::byte{ 0x00 });                                 // Delimiter
  }
}

UTEST(mamecobs, encoder_reusability_test)
{
  // Test that encoder objects can be reused without state interference
  auto enc1 = encode(true);
  auto enc2 = encode(false);

  std::vector<std::byte> frame = { std::byte{ 0x42 } };

  // Use enc1 multiple times
  std::vector<std::byte> result1;
  for (auto b : frame | enc1)
  {
    result1.push_back(b);
  }
  for (auto b : frame | enc1)
  {
    result1.push_back(b);
  }

  // Use enc2 multiple times
  std::vector<std::byte> result2;
  for (auto b : frame | enc2)
  {
    result2.push_back(b);
  }
  for (auto b : frame | enc2)
  {
    result2.push_back(b);
  }

  // enc1 results: [0x02, 0x42, 0x00, 0x02, 0x42, 0x00]
  std::vector<std::byte> expected1 = { std::byte{ 0x02 }, std::byte{ 0x42 }, std::byte{ 0x00 },
                                       std::byte{ 0x02 }, std::byte{ 0x42 }, std::byte{ 0x00 } };

  // enc2 results: [0x02, 0x42, 0x02, 0x42] (no delimiters)
  std::vector<std::byte> expected2 = { std::byte{ 0x02 },
                                       std::byte{ 0x42 },
                                       std::byte{ 0x02 },
                                       std::byte{ 0x42 } };

  ASSERT_EQ(result1.size(), expected1.size());
  ASSERT_EQ(result2.size(), expected2.size());

  for (size_t i = 0; i < expected1.size(); ++i)
  {
    ASSERT_EQ(result1[i], expected1[i]);
  }

  for (size_t i = 0; i < expected2.size(); ++i)
  {
    ASSERT_EQ(result2[i], expected2[i]);
  }
}