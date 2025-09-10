/**
 * @file byte_utils.hpp
 * @author Carlos Salguero
 * @brief Byte manipulation utilities for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

namespace velox::utils {
/// @brief Byte manipulation utilities
namespace bytes {
/**
 * @brief Ensure value is in little-endian format
 *
 * @tparam T Integral type to convert
 * @param value  Value to convert
 * @return constexpr T Value in little-endian format
 */
template <std::integral T>
[[nodiscard]] constexpr T to_little_endian(T value) noexcept {
  if constexpr (std::endian::native == std::endian::little) {
    return value;
  } else {
    return std::byteswap(value);
  }
}

/**
 * @brief Ensure value is in big-endian format
 *
 * @tparam T Integral type to convert
 * @param value  Value to convert
 * @return constexpr T Value in big-endian format
 */
template <std::integral T>
[[nodiscard]] constexpr T to_bid_endian(T value) noexcept {
  if constexpr (std::endian::native == std::endian::big) {
    return value;
  } else {
    return std::byteswap(value);
  }
}

/**
 * @brief Convert value from little-endian format to native format
 *
 * @tparam T Integral type to convert
 * @param value  Value to convert
 * @return constexpr T Value in native format
 */
template <std::integral T>
[[nodiscard]] constexpr T from_little_endian(T value) noexcept {
  return to_little_endian(value);
}

/**
 * @brief Convert value from big-endian format to native format
 *
 * @tparam T Integral type to convert
 * @param value  Value to convert
 * @return constexpr T Value in native format
 */
template <std::integral T>
[[nodiscard]] constexpr T from_big_endian(T value) noexcept {
  return to_bid_endian(value);
}

/**
 * @brief Write value to buffer in little-endian format
 *
 * @tparam T Integral type to write
 * @param buffer Destination buffer
 * @param value Value to write
 * @param offset Offset in buffer to write to
 */
template <std::integral T>
void write_le(std::span<uint8_t> buffer, T value, size_t offset = 0) {
  static_assert(std::is_trivially_copyable_v<T>,
                "T must be trivially copyable");

  auto le_vaue = to_little_endian(value);
  std::memcpy(buffer.data() + offset, &le_vaue, sizeof(T));
};

/**
 * @brief Write value to buffer in big-endian format
 *
 * @tparam T Integral type to write
 * @param buffer Destination buffer
 * @param value Value to write
 * @param offset Offset in buffer to write to
 */
template <std::integral T>
void write_be(std::span<uint8_t> buffer, T value, size_t offset = 0) {
  static_assert(std::is_trivially_copyable_v<T>,
                "T must be trivially copyable");

  auto be_vaue = to_bid_endian(value);
  std::memcpy(buffer.data() + offset, &be_vaue, sizeof(T));
};

/**
 * @brief Read value from buffer in little-endian format
 *
 * @tparam T Integral type to read
 * @param buffer Source buffer
 * @param offset Offset in buffer to read from
 * @return T Value read from buffer
 */
template <std::integral T>
[[nodiscard]] T read_le(std::span<const uint8_t> buffer, size_t offset = 0) {
  static_assert(std::is_trivially_copyable_v<T>,
                "T must be trivially copyable");

  T value;
  std::memcpy(&value, buffer.data() + offset, sizeof(T));
  return from_little_endian(value);
};

/**
 * @brief Read value from buffer in big-endian format
 *
 * @tparam T Integral type to read
 * @param buffer Source buffer
 * @param offset Offset in buffer to read from
 * @return T Value read from buffer
 */
template <std::integral T>
[[nodiscard]] T read_be(std::span<const uint8_t> buffer, size_t offset = 0) {
  static_assert(std::is_trivially_copyable_v<T>,
                "T must be trivially copyable");

  T value;
  std::memcpy(&value, buffer.data() + offset, sizeof(T));
  return from_big_endian(value);
};

/// @brief Secure memory comparison (constant time)
[[nodiscard]] bool secure_compare(std::span<const uint8_t> a,
                                  std::span<const uint8_t> b) noexcept;

/// @brief Clear sensitive memory
void secure_zero(std::span<uint8_t> buffer) noexcept;
} // namespace bytes
} // namespace velox::utils