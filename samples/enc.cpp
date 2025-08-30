#include "../src/mameCOBS.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

using namespace mamecobs;

int main()
{
  std::cout << "mameCOBS Encoder Sample\n";
  std::cout << "=======================\n\n";

  std::vector<std::byte> test_data{ std::byte{ 0x11 },
                                    std::byte{ 0x22 },
                                    std::byte{ 0x00 },
                                    std::byte{ 0x33 },
                                    std::byte{ 0x44 } };

  std::cout << "Original data: ";
  for (auto b : test_data)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  auto encoded_view = test_data | encode(true);
  std::vector<std::byte> encoded_data;
  for (auto b : encoded_view)
  {
    encoded_data.push_back(b);
  }

  std::cout << "Encoded data:  ";
  for (auto b : encoded_data)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n\n";

  std::cout << "Original size: " << std::dec << test_data.size() << " bytes\n";
  std::cout << "Encoded size:  " << encoded_data.size() << " bytes\n";
  std::cout << "Overhead:      " << (encoded_data.size() - test_data.size()) << " bytes\n\n";

  // Demonstrate multiple frame encoding
  std::cout << "Multiple frame encoding:\n";
  std::vector<std::vector<std::byte>> frames = {
    { std::byte{ 0x11 }, std::byte{ 0x22 } },
    { std::byte{ 0x00 } },
    { std::byte{ 0x33 }, std::byte{ 0x44 }, std::byte{ 0x55 } }
  };

  std::cout << "Frame 1: ";
  for (auto b : frames[0])
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\nFrame 2: ";
  for (auto b : frames[1])
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\nFrame 3: ";
  for (auto b : frames[2])
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  auto multi_encoded = frames | encode(true);
  std::vector<std::byte> multi_encoded_data;
  for (auto b : multi_encoded)
  {
    multi_encoded_data.push_back(b);
  }

  std::cout << "Encoded:       ";
  for (auto b : multi_encoded_data)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n\n";

  // Demonstrate single byte encoding
  std::cout << "Single byte encoding:\n";
  std::byte single_byte{ 0x42 };
  std::cout << "Input byte:    " << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<unsigned>(single_byte) << "\n";

  auto single_encoded = single_byte | encode(false);
  std::vector<std::byte> single_encoded_data;
  for (auto b : single_encoded)
  {
    single_encoded_data.push_back(b);
  }

  std::cout << "Encoded:       ";
  for (auto b : single_encoded_data)
  {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << " ";
  }
  std::cout << "\n";

  return 0;
}