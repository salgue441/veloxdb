/**
 * @file random_utils.hpp
 * @author Carlos Salguero
 * @brief Random utility functions and classes for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace velox::utils {
/// @brief Random utilities
namespace random {
/// @brief Thread-local random number generator
class Generator {
public:
  Generator();
  explicit Generator(uint64_t seed);

  template <std::integral T>
  [[nodiscard]] T next(T min_val = std::numeric_limits<T>::min(),
                       T max_val = std::numeric_limits<T>::max()) {
    std::uniform_int_distribution<T> dist(min_val, max_val);
    return dist(m_rng);
  }

  template <std::floating_point T>
  [[nodiscard]] T next(T min_val = std::numeric_limits<T>::min(),
                       T max_val = std::numeric_limits<T>::max()) {
    std::uniform_real_distribution<T> dist(min_val, max_val);
    return dist(m_rng);
  }

  [[nodiscard]] std::vector<uint8_t> bytes(size_t count);
  [[nodiscard]] std::string
  string(size_t length,
         std::string_view charset =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

private:
  std::mt19937_64 m_rng;
};

/// @brief Get thread-local random generator
[[nodiscard]] Generator &get_generator();

/// @brief Generate random bytes
[[nodiscard]] std::vector<uint8_t> random_bytes(size_t count);

/// @brief Generate random string
[[nodiscard]] std::string random_string(size_t length);

/// @brief Generate random UUID
[[nodiscard]] std::array<uint8_t, 16> random_uuid();
} // namespace random
} // namespace velox::utils