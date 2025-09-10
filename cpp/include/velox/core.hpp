/**
 * @file core.hpp
 * @author Carlos Salguero
 * @brief Core definitions and common includes for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

// Standard library includes
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

// Third-party includes
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <tl/expected.hpp>

/// @brief Main namespace for VeloxDB
namespace velox {
/// @brief Version information
namespace version {
constexpr int MAJOR = 0;
constexpr int MINOR = 1;
constexpr int PATCH = 0;
constexpr std::string_view VERSION_STRING = "0.1.0";
constexpr std::string_view BUILD_TYPE =
#ifdef NDEBUG
    "Release";
#else
    "Debug";
#endif
} // namespace version

/// @brief Common type aliases
using Byte = uint8_t;
using ByteSpan = std::span<Byte>;
using ConstByteSpan = std::span<const Byte>;
using ByteVector = std::vector<Byte>;
using ByteArray = std::array<Byte, 16>;

/// @brief Time-related types
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::microseconds;
using SystemTimePoint = std::chrono::system_clock::time_point;

/// @brief ID types for database objects
using ObjectId = uint64_t;
using PageId = uint64_t;
using RecordId = uint64_t;
using TransactionId = uint64_t;
using TableId = uint32_t;
using ColumnId = uint16_t;
using IndexId = uint32_t;

/// @brief Size and offset types
using Size = size_t;
using Offset = size_t;
using FileOffset = uint64_t;

/// @brief Constants for invalid/special values
namespace constants {
constexpr PageId INVALID_PAGE_ID = 0;
constexpr RecordId INVALID_RECORD_ID = 0;
constexpr TransactionId INVALID_TRANSACTION_ID = 0;
constexpr TableId INVALID_TABLE_ID = 0;
constexpr IndexId INVALID_INDEX_ID = 0;

// Page and block sizes
constexpr Size PAGE_SIZE = 4096;
constexpr Size CACHE_LINE_SIZE = 64;
constexpr Size DISK_BLOCK_SIZE = 512;

// Limits
constexpr Size MAX_TABLE_NAME_LENGTH = 128;
constexpr Size MAX_COLUMN_NAME_LENGTH = 64;
constexpr Size MAX_INDEX_NAME_LENGTH = 64;
constexpr Size MAX_RECORD_SIZE = PAGE_SIZE / 2;
constexpr Size MAX_KEY_SIZE = 255;
constexpr Size MAX_VALUE_SIZE = 65535;

// Buffer pool defaults
constexpr Size DEFAULT_BUFFER_POOL_SIZE = 1000;
constexpr Size MIN_BUFFER_POOL_SIZE = 10;
constexpr Size MAX_BUFFER_POOL_SIZE = 1000000;
} // namespace constants

/// @brief Forward declarations for core types
namespace storage {
class StorageEngine;
class Page;
class BufferPool;
class TransactionManager;
class WALManager;
} // namespace storage

namespace index {
template <typename Key, typename Value> class BPlusTree;
class IndexManager;
} // namespace index

namespace query {
class QueryProcessor;
class QueryOptimizer;
struct QueryPlan;
} // namespace query

namespace sql {
class Parser;
class Analyzer;
struct Statement;
} // namespace sql

namespace network {
class Server;
class Client;
class Protocol;
} // namespace network

/// @brief Error handling
namespace error {

/// @brief Base error codes for the system
enum class ErrorCode : uint32_t {
  SUCCESS = 0,

  // General errors (1-99)
  UNKNOWN_ERROR = 1,
  INVALID_ARGUMENT = 2,
  OUT_OF_MEMORY = 3,
  NOT_IMPLEMENTED = 4,
  INTERNAL_ERROR = 5,

  // Storage errors (100-199)
  STORAGE_ERROR = 100,
  PAGE_NOT_FOUND = 101,
  RECORD_NOT_FOUND = 102,
  TABLE_NOT_FOUND = 103,
  BUFFER_FULL = 104,
  DISK_FULL = 105,
  IO_ERROR = 106,
  CORRUPTION = 107,

  // Transaction errors (200-299)
  TRANSACTION_ERROR = 200,
  TRANSACTION_ABORTED = 201,
  DEADLOCK_DETECTED = 202,
  LOCK_TIMEOUT = 203,
  ISOLATION_VIOLATION = 204,

  // Query errors (300-399)
  QUERY_ERROR = 300,
  SYNTAX_ERROR = 301,
  SEMANTIC_ERROR = 302,
  TYPE_MISMATCH = 303,
  CONSTRAINT_VIOLATION = 304,

  // Network errors (400-499)
  NETWORK_ERROR = 400,
  CONNECTION_FAILED = 401,
  PROTOCOL_ERROR = 402,
  TIMEOUT = 403,

  // Index errors (500-599)
  INDEX_ERROR = 500,
  INDEX_NOT_FOUND = 501,
  DUPLICATE_KEY = 502,
  KEY_NOT_FOUND = 503
};

/// @brief Convert error code to string
[[nodiscard]] constexpr std::string_view to_string(ErrorCode code) noexcept {
  switch (code) {
  case ErrorCode::SUCCESS:
    return "SUCCESS";
  case ErrorCode::UNKNOWN_ERROR:
    return "UNKNOWN_ERROR";
  case ErrorCode::INVALID_ARGUMENT:
    return "INVALID_ARGUMENT";
  case ErrorCode::OUT_OF_MEMORY:
    return "OUT_OF_MEMORY";
  case ErrorCode::NOT_IMPLEMENTED:
    return "NOT_IMPLEMENTED";
  case ErrorCode::INTERNAL_ERROR:
    return "INTERNAL_ERROR";
  case ErrorCode::STORAGE_ERROR:
    return "STORAGE_ERROR";
  case ErrorCode::PAGE_NOT_FOUND:
    return "PAGE_NOT_FOUND";
  case ErrorCode::RECORD_NOT_FOUND:
    return "RECORD_NOT_FOUND";
  case ErrorCode::TABLE_NOT_FOUND:
    return "TABLE_NOT_FOUND";
  case ErrorCode::BUFFER_FULL:
    return "BUFFER_FULL";
  case ErrorCode::DISK_FULL:
    return "DISK_FULL";
  case ErrorCode::IO_ERROR:
    return "IO_ERROR";
  case ErrorCode::CORRUPTION:
    return "CORRUPTION";
  case ErrorCode::TRANSACTION_ERROR:
    return "TRANSACTION_ERROR";
  case ErrorCode::TRANSACTION_ABORTED:
    return "TRANSACTION_ABORTED";
  case ErrorCode::DEADLOCK_DETECTED:
    return "DEADLOCK_DETECTED";
  case ErrorCode::LOCK_TIMEOUT:
    return "LOCK_TIMEOUT";
  case ErrorCode::ISOLATION_VIOLATION:
    return "ISOLATION_VIOLATION";
  case ErrorCode::QUERY_ERROR:
    return "QUERY_ERROR";
  case ErrorCode::SYNTAX_ERROR:
    return "SYNTAX_ERROR";
  case ErrorCode::SEMANTIC_ERROR:
    return "SEMANTIC_ERROR";
  case ErrorCode::TYPE_MISMATCH:
    return "TYPE_MISMATCH";
  case ErrorCode::CONSTRAINT_VIOLATION:
    return "CONSTRAINT_VIOLATION";
  case ErrorCode::NETWORK_ERROR:
    return "NETWORK_ERROR";
  case ErrorCode::CONNECTION_FAILED:
    return "CONNECTION_FAILED";
  case ErrorCode::PROTOCOL_ERROR:
    return "PROTOCOL_ERROR";
  case ErrorCode::TIMEOUT:
    return "TIMEOUT";
  case ErrorCode::INDEX_ERROR:
    return "INDEX_ERROR";
  case ErrorCode::INDEX_NOT_FOUND:
    return "INDEX_NOT_FOUND";
  case ErrorCode::DUPLICATE_KEY:
    return "DUPLICATE_KEY";
  case ErrorCode::KEY_NOT_FOUND:
    return "KEY_NOT_FOUND";
  }
  return "UNKNOWN";
}

/// @brief Result type for operations that can fail
template <typename T> using Result = tl::expected<T, ErrorCode>;

/// @brief Void result type
using VoidResult = Result<void>;

/// @brief Create a success result
template <typename T> [[nodiscard]] constexpr Result<T> ok(T &&value) {
  return Result<T>(std::forward<T>(value));
}

/// @brief Create a void success result
[[nodiscard]] constexpr VoidResult ok() { return VoidResult{}; }

/// @brief Create an error result
template <typename T> [[nodiscard]] constexpr Result<T> error(ErrorCode code) {
  return tl::unexpected(code);
}
} // namespace error

/// @brief Logging utilities
namespace log {

/// @brief Get logger for a component
[[nodiscard]] std::shared_ptr<spdlog::logger>
get_logger(const std::string &name);

/// @brief Initialize logging system
void initialize(
    spdlog::level::level_enum level = spdlog::level::info,
    const std::string &pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");

/// @brief Set global log level
void set_level(spdlog::level::level_enum level);

} // namespace log

/// @brief Memory management utilities
namespace memory {

/// @brief Custom deleter for aligned memory
struct AlignedDeleter {
  void operator()(void *ptr) const noexcept { std::free(ptr); }
};

/// @brief Unique pointer for aligned memory
template <typename T>
using aligned_unique_ptr = std::unique_ptr<T, AlignedDeleter>;

/// @brief Allocate aligned memory
template <typename T>
[[nodiscard]] aligned_unique_ptr<T>
make_aligned(size_t alignment = constants::CACHE_LINE_SIZE) {
  void *ptr = std::aligned_alloc(alignment, sizeof(T));
  if (!ptr) {
    throw std::bad_alloc();
  }

  return aligned_unique_ptr<T>(static_cast<T *>(ptr));
}

/// @brief Allocate aligned array
template <typename T>
[[nodiscard]] aligned_unique_ptr<T[]>
make_aligned_array(size_t count,
                   size_t alignment = constants::CACHE_LINE_SIZE) {
  size_t size = count * sizeof(T);
  size = (size + alignment - 1) & ~(alignment - 1);
  void *ptr = std::aligned_alloc(alignment, size);
  if (!ptr) {
    throw std::bad_alloc();
  }

  return aligned_unique_ptr<T[]>(static_cast<T *>(ptr));
}
} // namespace memory

/// @brief Thread utilities
namespace thread {

/// @brief Get number of hardware threads
[[nodiscard]] inline size_t hardware_concurrency() noexcept {
  auto n = std::thread::hardware_concurrency();
  return n > 0 ? n : 1;
}

/// @brief Thread-local storage for frequently used objects
template <typename T> class ThreadLocal {
public:
  template <typename... Args>
  ThreadLocal(Args &&...args) : m_factory([args...] { return T(args...); }) {}

  T &get() {
    static thread_local std::optional<T> instance;
    if (!instance) {
      instance = m_factory();
    }

    return *instance;
  }

  const T &get() const { return const_cast<ThreadLocal *>(this)->get(); }

private:
  std::function<T()> m_factory;
};
} // namespace thread

/// @brief Configuration management
namespace config {

/// @brief System configuration
struct SystemConfig {
  size_t buffer_pool_size = constants::DEFAULT_BUFFER_POOL_SIZE;
  size_t max_connections = 1000;
  size_t worker_threads = thread::hardware_concurrency();
  std::filesystem::path data_directory = "./data";
  std::filesystem::path log_directory = "./logs";
  bool enable_wal = true;
  bool enable_checksums = true;
  bool enable_compression = false;
  spdlog::level::level_enum log_level = spdlog::level::info;

  /// @brief Validate configuration
  [[nodiscard]] bool validate() const noexcept;

  /// @brief Load from file
  [[nodiscard]] static error::Result<SystemConfig>
  load(const std::filesystem::path &file);

  /// @brief Save to file
  [[nodiscard]] error::VoidResult save(const std::filesystem::path &file) const;
};

/// @brief Get global configuration
[[nodiscard]] SystemConfig &global_config();
} // namespace config

/// @brief Utility macros
#define VELOX_LIKELY(x) [[likely]] (x)
#define VELOX_UNLIKELY(x) [[unlikely]] (x)
#define VELOX_UNREACHABLE() __builtin_unreachable()

#define VELOX_FORCE_INLINE [[gnu::always_inline]] inline
#define VELOX_NO_INLINE [[gnu::noinline]]

// Branch prediction hints
#define VELOX_PREDICT_TRUE(x) __builtin_expect(!!(x), 1)
#define VELOX_PREDICT_FALSE(x) __builtin_expect(!!(x), 0)

// Cache line alignment
#define VELOX_CACHE_ALIGNED alignas(velox::constants::CACHE_LINE_SIZE)

// Disable copy/move macros
#define VELOX_NON_COPYABLE(ClassName)                                          \
  ClassName(const ClassName &) = delete;                                       \
  ClassName &operator=(const ClassName &) = delete;

#define VELOX_NON_MOVABLE(ClassName)                                           \
  ClassName(ClassName &&) = delete;                                            \
  ClassName &operator=(ClassName &&) = delete;

#define VELOX_NON_COPYABLE_NON_MOVABLE(ClassName)                              \
  VELOX_NON_COPYABLE(ClassName)                                                \
  VELOX_NON_MOVABLE(ClassName)

} // namespace velox

/// @brief Global operator overloads for formatting
template <> struct fmt::formatter<velox::error::ErrorCode> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(velox::error::ErrorCode code, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", velox::error::to_string(code));
  }
};