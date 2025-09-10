/**
 * @file concepts.hpp
 * @author Carlos Salguero
 * @brief Concepts for type-safety and better error messages
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <chrono>
#include <concepts>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

namespace velox::concepts {
/**
 * @brief Concept for types that can be used as keys in indexes
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be copyable, movable, comparable, and hashable
 */
template <typename T>
concept Indexable = requires {
  requires std::copyable<T>;
  requires std::movable<T>;
  requires std::totally_ordered<T>;
  requires requires(const T &t) { std::hash<T>{}(t); };
};

/**
 * @brief Concept for serializable types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to have serialize and deserialize member functions
 */
template <typename T>
concept Serializable =
    requires(T t, std::span<uint8_t> buffer, std::span<const uint8_t> data) {
      { t.serialize(buffer) } -> std::same_as<size_t>;
      { T::deserialize(data) } -> std::same_as<T>;
      { t.serialized_size() } -> std::same_as<size_t>;
    };

/// @brief Concept for string-like types
template <typename T>
concept StringLike = std::convertible_to<T, std::string_view>;

/**
 * @brief Concept for byte-like containers
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be a contiguous container of bytes (uint8_t or char)
 *       and to support size() and data() member functions
 */
template <typename T>
concept ByteContainer = requires(T container) {
  requires std::ranges::contiguous_range<T>;
  requires std::same_as<std::ranges::range_value_t<T>, uint8_t> ||
               std::same_as<std::ranges::range_value_t<T>, char> ||
               std::same_as<std::ranges::range_value_t<T>, std::byte>;

  { container.data() };
  { container.size() } -> std::same_as<size_t>;
};

/**
 * @brief Container for record-like types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to have an ID, must be serializable to bytes, and
 *       must be constructible from a span of bytes
 */
template <typename T>
concept Record = requires(const T &record) {
  { record.id() } -> std::convertible_to<uint64_t>;
  { record.to_bytes() } -> ByteContainer;
  requires std::constructible_from<T, std::span<const uint8_t>>;
};

/**
 * @brief Concept for page-like types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to have an ID, data access, and dirty flag operations
 */
template <typename T>
concept Page = requires(T &page, const T &const_page) {
  { const_page.id() } -> std::convertible_to<uint64_t>;
  { page.data() } -> std::convertible_to<std::span<uint8_t>>;
  { const_page.data() } -> std::convertible_to<std::span<const uint8_t>>;
  { const_page.is_dirty() } -> std::convertible_to<bool>;
  { page.mark_dirty() } -> std::same_as<void>;
  { page.mark_clean() } -> std::same_as<void>;
};

/**
 * @brief Concept for buffer pool-like types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to have get and put page operations, statistics, and
 *       must be able to flush
 */
template <typename T>
concept BufferPool = requires(T &pool, uint64_t page_id) {
  { pool.get_page(page_id) };
  { pool.put_page(page_id, std::declval<typename T::page_type>()) };
  { pool.get_statistics() };
  { pool.flush() };
};

/**
 * @brief Concept for transaction-like types
 *
 * @tparam T Type to check against the concept
 * @note Must have transaction ID, state operations, and timestamp
 */
template <typename T>
concept Transaction = requires(T &txn) {
  { txn.id() } -> std::convertible_to<uint64_t>;
  { txn.commit() };
  { txn.rollback() };
  { txn.is_active() } -> std::convertible_to<bool>;
  { txn.begin_time() };
};

/**
 * @brief Concept for index-like types
 *
 * @tparam T Type to check against the concept
 * @tparam Key Type of the key
 * @tparam Value Type of the value
 * @note Requires T to support insert, find, and erase operations, range
 *       queries, and must be iterable
 */
template <typename T, typename Key, typename Value>
concept Index = requires(T &index, const Key &key, const Value &value) {
  { index.insert(key, value) };
  { index.find(key) };
  { index.remove(key) };
  { index.range(key, key) };
  requires std::ranges::range<T>;
};

/**
 * @brief Concept for thread-safe types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be copyable or movable and to support locking
 */
template <typename T>
concept Lock = requires(T &lock) {
  { lock.lock() };
  { lock.unlock() };
  { lock.try_lock() } -> std::convertible_to<bool>;
};

/**
 * @brief Concept for thread-safe types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be copyable or movable and to support locking
 */
template <typename T>
concept Logger = requires(T &logger, std::string_view message) {
  { logger.debug(message) };
  { logger.info(message) };
  { logger.warn(message) };
  { logger.error(message) };
  { logger.set_level(std::declval<int>()) };
};

/**
 * @brief Concept for allocator types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to support allocate and deallocate methods
 */
template <typename T>
concept Allocator = requires(T &allocator, size_t size, void *ptr) {
  { allocator.allocate(size) } -> std::convertible_to<void *>;
  { allocator.deallocate(ptr, size) } -> std::same_as<void>;
  {
    allocator.allocate_aligned(size, std::declval<size_t>())
  } -> std::convertible_to<void *>;
};

/**
 * @brief Concept for cache-like types
 *
 * @tparam T Type to check against the concept
 * @tparam Key Type of the key
 * @tparam Value Type of the value
 * @note Requires T to support get, put, and remove operations
 */
template <typename T, typename Key, typename Value>
concept Cache = requires(T &cache, const Key &key, const Value &value) {
  { cache.get(key) };
  { cache.put(key, value) };
  { cache.evict(key) };
  { cache.clear() };
  { cache.size() } -> std::convertible_to<size_t>;
  { cache.capacity() } -> std::convertible_to<size_t>;
};

/**
 * @brief Concept for compressor types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to support compress, decompress, and max_compressed_size
 *       methods
 */
template <typename T>
concept Compressor = requires(T &compressor, std::span<const uint8_t> input,
                              std::span<uint8_t> output) {
  { compressor.compress(input, output) } -> std::convertible_to<size_t>;
  { compressor.decompress(input, output) } -> std::convertible_to<size_t>;
  {
    compressor.max_compressed_size(input.size())
  } -> std::convertible_to<size_t>;
};

/**
 * @brief Concept for hash functions
 *
 * @tparam T Type to check against the concept
 * @tparam Input Type of the input
 * @note Requires T to produce a hash value and must be consistent
 */
template <typename T, typename Input>
concept Hasher = requires(const T &hasher, const Input &input) {
  { hasher(input) } -> std::convertible_to<size_t>;
  requires std::regular<T>;
};

/**
 * @brief Concept for metrics collector types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to support counter, gauge, histogram, and get_metrics
 *       methods
 */
template <typename T>
concept MetricsCollector =
    requires(T &collector, std::string_view name, double value) {
      { collector.counter(name, value) };
      { collector.gauge(name, value) };
      { collector.histogram(name, value) };
      { collector.get_metrics() };
    };

/**
 * @brief Concept for configuration types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be valid and serializable
 */
template <typename T>
concept Configuration = requires(const T &config) {
  { config.is_valid() } -> std::convertible_to<bool>;
  requires Serializable<T>;
};

/**
 * @brief Concept for thread-safe types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to define a nested type `thread_safe_tag` equal to
 *       `std::true_type`
 */
template <typename T>
concept ThreadSafe = requires {
  typename T::thread_safe_tag;
  requires std::same_as<typename T::thread_safe_tag, std::true_type>;
};

/**
 * @brief Concept for arithmetic types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be either an integral or floating-point type
 */
template <typename T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;

/**
 * @brief Concept for duration types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be convertible to milliseconds and to have a count()
 *       method returning an arithmetic type
 */
template <typename T>
concept Duration = requires(const T &duration) {
  { std::chrono::duration_cast<std::chrono::milliseconds>(duration) };
  { duration.count() } -> Arithmetic;
};

/**
 * @brief Concept for database value types
 *
 * @tparam T Type to check against the concept
 * @note Requires T to be one of: nullptr_t (NULL), integral (INTEGER),
 *       floating-point (REAL), string-like (TEXT), or byte container (BLOB)
 */
template <typename T>
concept DatabaseValue = requires {
  requires std::same_as<T, std::nullptr_t> || // NULL
               std::integral<T> ||            // INTEGER
               std::floating_point<T> ||      // REAL
               StringLike<T> ||               // TEXT
               ByteContainer<T>;              // BLOB
};

} // namespace velox::concepts

/// @brief Convenience macros for concept checking
#define VELOX_REQUIRE_INDEXABLE(T)                                             \
  static_assert(velox::concepts::Indexable<T>, #T " must be indexable")
#define VELOX_REQUIRE_SERIALIZABLE(T)                                          \
  static_assert(velox::concepts::Serializable<T>, #T " must be serializable")
#define VELOX_REQUIRE_RECORD(T)                                                \
  static_assert(velox::concepts::Record<T>, #T " must be a record type")
#define VELOX_REQUIRE_PAGE(T)                                                  \
  static_assert(velox::concepts::Page<T>, #T " must be a page type")
#define VELOX_REQUIRE_THREAD_SAFE(T)                                           \
  static_assert(velox::concepts::ThreadSafe<T>, #T " must be thread-safe")