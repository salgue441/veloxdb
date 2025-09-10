/**
 * @file hash_utils.hpp
 * @author Carlos Salguero
 * @brief Hash utility functions and classes for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

namespace velox::utils {
/// @brief Hash utilities
namespace hash {
/**
 * @brief Combine hash values
 *
 * @tparam T Type of the hash values
 * @param seed Seed value to combine with
 * @param value Value to combine
 */
template <typename T>
void hash_combine(std::size_t &seed, const T &value) noexcept {
  seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/**
 * @brief Hash multiple values
 *
 * @tparam Args Types of the values
 * @param args Values to hash
 * @return std::size_t Combined hash value
 */
template <typename... Args>
[[nodiscard]] size_t hash_values(const Args &...args) noexcept {
  size_t seed = 0;

  (hash_combine(seed, args), ...);
  return seed;
}

/// @brief FNV-1a hash function
[[nodiscard]] uint32_t fnv1a_32(std::span<const uint8_t> data) noexcept;
[[nodiscard]] uint64_t fnv1a_64(std::span<const uint8_t> data) noexcept;

/// @brief CRC32 checksum
[[nodiscard]] uint32_t crc32(std::span<const uint8_t> data) noexcept;

/// @brief xxHash function (fast non-cryptographic hash)
[[nodiscard]] uint64_t xxhash64(std::span<const uint8_t> data,
                                uint64_t seed = 0) noexcept;

}; // namespace hash
} // namespace velox::utils
