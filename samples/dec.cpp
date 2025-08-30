#include "../src/mameCOBS.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

using namespace mamecobs;

int main()
{
  std::cout << "mameCOBS Decoder Sample\n";
  std::cout << "=======================\n\n";

  // Test basic decoding
  std::vector<std::byte> encoded_data{ std::byte{ 0x03 }, std::byte{ 0x11 }, std::byte{ 0x22 },
                                       std::byte{ 0x02 }, std::byte{ 0x33 }, std::byte{ 0x00 } };

  std::cout << "Encoded data: ";
  for (auto b : encoded_data)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  auto decoded_view = encoded_data | decode();
  std::vector<std::byte> decoded_data;
  for (auto frame_result : decoded_view)
  {
    if (frame_result.has_value())
    {
      auto frame = *frame_result;
      for (auto b : frame)
      {
        decoded_data.push_back(b);
      }
    }
    else
    {
      std::cout << "Decode error: " << static_cast<int>(frame_result.error()) << "\n";
    }
  }

  std::cout << "Decoded data: ";
  for (auto b : decoded_data)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n\n";

  std::cout << "Encoded size: " << std::dec << encoded_data.size() << " bytes\n";
  std::cout << "Decoded size: " << decoded_data.size() << " bytes\n\n";

  // Demonstrate round-trip encoding/decoding
  std::cout << "Round-trip test:\n";
  std::vector<std::byte> original{ std::byte{ 0xAA },
                                   std::byte{ 0x00 },
                                   std::byte{ 0xBB },
                                   std::byte{ 0x00 },
                                   std::byte{ 0xCC } };

  std::cout << "Original:     ";
  for (auto b : original)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  // Encode
  auto encoded_view = original | encode(true);
  std::vector<std::byte> encoded;
  for (auto b : encoded_view)
  {
    encoded.push_back(b);
  }

  std::cout << "Encoded:      ";
  for (auto b : encoded)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  // Decode
  auto roundtrip_view = encoded | decode();
  std::vector<std::byte> roundtrip;
  for (auto frame_result : roundtrip_view)
  {
    if (frame_result.has_value())
    {
      auto frame = *frame_result;
      for (auto b : frame)
      {
        roundtrip.push_back(b);
      }
    }
    else
    {
      std::cout << "Decode error during roundtrip: " << static_cast<int>(frame_result.error()) << "\n";
      return 1;
    }
  }

  std::cout << "Roundtrip:    ";
  for (auto b : roundtrip)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  bool matches = (original.size() == roundtrip.size());
  if (matches)
  {
    for (size_t i = 0; i < original.size(); ++i)
    {
      if (original[i] != roundtrip[i])
      {
        matches = false;
        break;
      }
    }
  }

  std::cout << "Round-trip test: " << (matches ? "PASSED" : "FAILED") << "\n\n";

  // Demonstrate multiple frame decoding
  std::cout << "Multiple frame decoding:\n";
  std::vector<std::byte> multi_encoded{
    std::byte{ 0x03 }, std::byte{ 0x11 }, std::byte{ 0x22 }, std::byte{ 0x00 }, // Frame 1
    std::byte{ 0x01 }, std::byte{ 0x00 },                                       // Frame 2 (single zero)
    std::byte{ 0x04 }, std::byte{ 0x33 }, std::byte{ 0x44 }, std::byte{ 0x55 }, std::byte{ 0x00 } // Frame 3
  };

  std::cout << "Encoded:      ";
  for (auto b : multi_encoded)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  auto multi_decoded = multi_encoded | decode();
  int frame_num = 1;
  for (auto frame_result : multi_decoded)
  {
    std::cout << "Frame " << frame_num++ << ":      ";
    if (frame_result.has_value())
    {
      auto frame = *frame_result;
      for (auto b : frame)
      {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
      }
      if (frame.empty())
      {
        std::cout << "(empty)";
      }
    }
    else
    {
      std::cout << "ERROR: " << static_cast<int>(frame_result.error());
    }
    std::cout << "\n";
  }

  return 0;
}