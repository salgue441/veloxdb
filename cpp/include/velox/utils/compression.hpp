/**
 * @file compression.hpp
 * @author Carlos Salguero
 * @brief Compression utility functions and classes for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace velox::utils {
/// @brief Compression utilities
namespace compression {
/// @brief Simple RLE compression for repetitive data
[[nodiscard]] std::vector<uint8_t> rle_compress(std::span<const uint8_t> data);
[[nodiscard]] std::vector<uint8_t>
rle_decompress(std::span<const uint8_t> compressed);

/// @brief Dictionary compression for string data
class DictionaryCompressor {
public:
  [[nodiscard]] std::vector<uint8_t>
  compress(const std::vector<std::string> &strings);
  
  [[nodiscard]] std::vector<std::string>
  decompress(std::span<const uint8_t> compressed);

  void clear() { m_dictionary.clear(); }
  [[nodiscard]] size_t m_dictionarysize() const { return m_dictionary.size(); }

private:
  std::unordered_map<std::string, uint32_t> m_dictionary;
  std::vector<std::string> m_reverse_dictionary;
};
} // namespace compression
} // namespace velox::utils