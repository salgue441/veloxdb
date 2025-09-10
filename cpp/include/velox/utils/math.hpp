/**
 * @file math.hpp
 * @author Carlos Salguero
 * @brief Math utility functions for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace velox::utils {
/// @brief Math utilities
namespace math {
/// @brief Check if number is power of 2
template <std::integral T>
[[nodiscard]] constexpr bool is_power_of_2(T value) noexcept {
  return value > 0 && (value & (value - 1)) == 0;
}

/// @brief Round up to next power of 2
template <std::integral T>
[[nodiscard]] constexpr T next_power_of_2(T value) noexcept {
  if (value <= 1)
    return 1;

  return T{1} << (std::bit_width(
             static_cast<std::make_unsigned_t<T>>(value - 1)));
}

/// @brief Align value up to alignment boundary
template <std::integral T>
[[nodiscard]] constexpr T align_up(T value, T alignment) noexcept {
  return (value + alignment - 1) & ~(alignment - 1);
}

/// @brief Align value down to alignment boundary
template <std::integral T>
[[nodiscard]] constexpr T align_down(T value, T alignment) noexcept {
  return value & ~(alignment - 1);
}

/// @brief Check if value is aligned
template <std::integral T>
[[nodiscard]] constexpr bool is_aligned(T value, T alignment) noexcept {
  return (value & (alignment - 1)) == 0;
}

/// @brief Clamp value between min and max
template <typename T>
[[nodiscard]] constexpr T clamp(T value, T min_val, T max_val) noexcept {
  return std::max(min_val, std::min(value, max_val));
}

/// @brief Linear interpolation
template <std::floating_point T>
[[nodiscard]] constexpr T lerp(T a, T b, T t) noexcept {
  return a + t * (b - a);
}
} // namespace math
} // namespace velox::utils