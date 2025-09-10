/**
 * @file memory_utils.hpp
 * @author Carlos Salguero
 * @brief Memory utility functions and classes for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <cstdlib>
#include <memory>
#include <span>
#include <utility>

namespace velox::utils {
/// @brief Memory utilities
namespace memory {
/// @brief Aligned memory allocator
template <size_t Alignment = 64> class AlignedAllocator {
public:
  using value_type = uint8_t;

public:
  [[nodiscard]] void *allocate(size_t size) {
    return std::aligned_alloc(Alignment,
                              (size + Alignment - 1) & ~(Alignment - 1));
  }

  void deallocate(void *ptr, size_t /*size*/) noexcept { std::free(ptr); }

  template <typename T> [[nodiscard]] T *allocate_typed(size_t count = 1) {
    return static_cast<T *>(allocate(sizeof(T) * count));
  }
};

/// @brief RAII wrapper for aligned memory
template <size_t Alignment = 64> class AlignedBuffer {
public:
  explicit AlignedBuffer(size_t size) : m_size(size) {
    if (m_size > 0) {
      m_data = m_allocator.allocate(m_size);
    }
  }

  ~AlignedBuffer() {
    if (m_data) {
      m_allocator.deallocate(m_data, m_size);
    }
  }

  // Non-copyable, movable
  AlignedBuffer(const AlignedBuffer &) = delete;
  AlignedBuffer &operator=(const AlignedBuffer &) = delete;

  AlignedBuffer(AlignedBuffer &&other) noexcept
      : m_data(std::exchange(other.m_data, nullptr)),
        m_size(std::exchange(other.m_size, 0)) {}

  AlignedBuffer &operator=(AlignedBuffer &&other) noexcept {
    if (this != &other) {
      if (m_data)
        m_allocator.deallocate(m_data, m_size);

      m_data = std::exchange(other.m_data, nullptr);
      m_size = std::exchange(other.m_size, 0);
    }

    return *this;
  }

  [[nodiscard]] void *data() noexcept { return m_data; }
  [[nodiscard]] const void *data() const noexcept { return m_data; }
  [[nodiscard]] size_t size() const noexcept { return m_size; }
  [[nodiscard]] std::span<uint8_t> span() noexcept {
    return std::span<uint8_t>(static_cast<uint8_t *>(m_data), m_size);
  }

  [[nodiscard]] std::span<const uint8_t> span() const noexcept {
    return std::span<const uint8_t>(static_cast<const uint8_t *>(m_data),
                                    m_size);
  }

private:
  void *m_data{nullptr};
  size_t m_size{0};
  AlignedAllocator<Alignment> m_allocator;
};

/// @brief Memory pool for frequent allocations
class MemoryPool {
public:
  explicit MemoryPool(size_t block_size = 4096, size_t max_blocks = 1024);
  ~MemoryPool();

  // Non-copyable, non-movable
  MemoryPool(const MemoryPool &) = delete;
  MemoryPool &operator=(const MemoryPool &) = delete;
  MemoryPool(MemoryPool &&) = delete;
  MemoryPool &operator=(MemoryPool &&) = delete;

  [[nodiscard]] void *allocate(size_t size);
  void deallocate(void *ptr, size_t size) noexcept;

  [[nodiscard]] size_t bytes_allocated() const noexcept;
  [[nodiscard]] size_t bytes_available() const noexcept;

  void reset() noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};
} // namespace memory
} // namespace velox::utils