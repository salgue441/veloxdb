/**
 * @file storage_engine.hpp
 * @author Carlos Salguero
 * @brief High-performance storage engine for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <array>
#include <assert.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

namespace velox::storage {
// Forward declarations
class BufferPool;
class IndexManager;
class TransactionManager;
class WALManager;

/// @brief Common type aliases for better readability
using PageId = uint64_t;
using RecordId = uint64_t;
using TransactionId = uint64_t;
using LogSequenceNumber = uint64_t;
using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

/// @brief Constants for storage engine configuration
namespace config {
constexpr size_t PAGE_SIZE = 4096;
constexpr size_t DEFAULT_BUFFER_POOL_SIZE = 1000;
constexpr PageId INVALID_PAGE_ID = 0;
constexpr RecordId INVALID_RECORD_ID = 0;
constexpr size_t MAX_RECORD_SIZE = PAGE_SIZE / 2;
constexpr size_t PAGE_HEADER_SIZE = 64;
constexpr size_t PAGE_DATA_SIZE = PAGE_SIZE - PAGE_HEADER_SIZE;
} // namespace config

/// @brief Error types for storage options
enum class StorageError {
  OK,                   ///< Operation successful
  PAGE_NOT_FOUND,       ///< Requested page does not exist
  RECORD_NOT_FOUND,     ///< Requested record does not exist
  BUFFER_FULL,          ///< Buffer pool is full
  IO_ERROR,             ///< File I/O operation failed
  CORRUPTION,           ///< Data corruption detected
  OUT_OF_SPACE,         ///< No space available for operation
  INVALID_OPERATION,    ///< Operation not allowed in current state
  TRANSACTION_ABORTED,  ///< Transaction was aborted
  DEADLOCK_DETECTED,    ///< Deadlock was detected
  CONSTRAINT_VIOLATION, ///< Constraint violation
  INVALID_ARGUMENT      ///< Invalid argument provided
};

/// @brief Convert StorageError to string for logging/debugging
[[nodiscard]] constexpr std::string_view
to_string(StorageError error) noexcept {
  switch (error) {
  case StorageError::OK:
    return "OK";
  case StorageError::PAGE_NOT_FOUND:
    return "PAGE_NOT_FOUND";
  case StorageError::RECORD_NOT_FOUND:
    return "RECORD_NOT_FOUND";
  case StorageError::BUFFER_FULL:
    return "BUFFER_FULL";
  case StorageError::IO_ERROR:
    return "IO_ERROR";
  case StorageError::CORRUPTION:
    return "CORRUPTION";
  case StorageError::OUT_OF_SPACE:
    return "OUT_OF_SPACE";
  case StorageError::INVALID_OPERATION:
    return "INVALID_OPERATION";
  case StorageError::TRANSACTION_ABORTED:
    return "TRANSACTION_ABORTED";
  case StorageError::DEADLOCK_DETECTED:
    return "DEADLOCK_DETECTED";
  case StorageError::CONSTRAINT_VIOLATION:
    return "CONSTRAINT_VIOLATION";
  case StorageError::INVALID_ARGUMENT:
    return "INVALID_ARGUMENT";
  }

  return "UNKNOWN_ERROR";
}

/// @brief Result type using expected for error handling
template <typename T> using Result = tl::expected<T, StorageError>;

/// @brief Page types for different kinds of pages
enum class PageType : uint32_t {
  FREE_PAGE = 0,      ///< Unallocated page
  TABLE_ROOT = 1,     ///< Root page of a table
  TABLE_DATA = 2,     ///< Data page containing records
  INDEX_ROOT = 3,     ///< Root page of an index
  INDEX_INTERNAL = 4, ///< Internal index page
  INDEX_LEAF = 5,     ///< Leaf index page
  OVERFLOW = 6,       ///< Overflow page for large records
  METADATA = 7        ///< System metadata page
};

/// @brief Page header structure (64 bytes, cache-line aligned)
struct alignas(64) PageHeader {
  PageType page_type;
  uint32_t free_space_offset;
  uint32_t free_space_size;
  uint16_t record_count;
  uint16_t flags;
  PageId page_id;
  PageId next_page;
  PageId prev_page;
  LogSequenceNumber lsn;
  uint32_t checksum;
  uint8_t reserved[12];

  /// @brief Default constructor
  PageHeader() = default;

  /**
   * @brief Constructor with page ID and type
   *
   * @param id Page identifier
   * @param type Page type
   */
  explicit PageHeader(PageId id, PageType type = PageType::FREE_PAGE);

  /// @brief Calculate and update checksum
  void update_checksum() noexcept;

  /**
   * @brief Verify page checksum
   * @return true if checksum is valid
   */
  [[nodiscard]] bool verify_checksum() const noexcept;
};

static_assert(sizeof(PageHeader) == config::PAGE_HEADER_SIZE,
              "PageHeader size must be exactly 64 bytes");

/**
 * @brief Memory-mapped page wrapper with RAII semantics
 */
class Page {
public:
  /**
   * @brief Constructor
   *
   * @param id Page identifier
   */
  explicit Page(PageId id);

  /// @brief Destructor
  ~Page() = default;

  /// @brief Non-copyable, movable
  Page(const Page &) = delete;
  Page &operator=(const Page &) = delete;
  Page(Page &&) noexcept = default;
  Page &operator=(Page &&) noexcept = default;

  /**
   * @brief Get page header (mutable)
   * @return Reference to page header
   */
  [[nodiscard]] PageHeader &header() noexcept { return m_header; }

  /**
   * @brief Get page header (const)
   * @return Const reference to page header
   */
  [[nodiscard]] const PageHeader &header() const noexcept { return m_header; }

  /**
   * @brief Get page data span (mutable)
   * @return Span of page data
   */
  [[nodiscard]] std::span<uint8_t> data() noexcept {
    return std::span<uint8_t>(m_data.data(), m_data.size());
  }

  /**
   * @brief Get page data span (const)
   * @return Const span of page data
   */
  [[nodiscard]] std::span<const uint8_t> data() const noexcept {
    return std::span<const uint8_t>(m_data.data(), m_data.size());
  }

  /**
   * @brief Get page ID
   * @return Page identifier
   */
  [[nodiscard]] PageId id() const noexcept { return m_header.page_id; }

  /**
   * @brief Check if page is dirty
   * @return true if page has been modified
   */
  [[nodiscard]] bool is_dirty() const noexcept {
    return m_dirty.load(std::memory_order_acquire);
  }

  /**
   * @brief Mark page as dirty
   */
  void mark_dirty() noexcept {
    m_dirty.store(true, std::memory_order_release);
    m_last_modified = std::chrono::steady_clock::now();
  }

  /**
   * @brief Mark page as clean
   */
  void mark_clean() noexcept {
    m_dirty.store(false, std::memory_order_release);
  }

  /**
   * @brief Pin page in memory
   */
  void pin() noexcept { m_pin_count.fetch_add(1, std::memory_order_acq_rel); }

  /**
   * @brief Unpin page from memory
   */
  void unpin() noexcept {
    auto count = m_pin_count.fetch_sub(1, std::memory_order_acq_rel);
    assert(count > 0 && "Cannot unpin unpinned page");
  }

  /**
   * @brief Check if page is pinned
   * @return true if page is pinned in memory
   */
  [[nodiscard]] bool is_pinned() const noexcept {
    return m_pin_count.load(std::memory_order_acquire) > 0;
  }

  /**
   * @brief Get the pin count
   * @return Current pint count
   */
  [[nodiscard]] int32_t pin_count() const noexcept {
    return m_pin_count.load(std::memory_order_acquire);
  }

  /**
   * @brief Get the last accessed time
   * @return Timestamp of last access
   */
  [[nodiscard]] Timestamp last_accessed() const noexcept {
    return m_last_accessed;
  }

  /**
   * @brief Update the last accessed time to now
   */
  void touch() noexcept { m_last_accessed = std::chrono::steady_clock::now(); }

  /**
   * @brief Get the shared lock for reading
   * @return Shared lock guard
   */
  [[nodiscard]] std::shared_lock<std::shared_mutex> read_lock() const {
    return std::shared_lock<std::shared_mutex>(m_mutex);
  }

  /**
   * @brief Get the exclusive lock for writing
   * @return Unique lock guard
   */
  [[nodiscard]] std::unique_lock<std::shared_mutex> write_lock() const {
    return std::unique_lock<std::shared_mutex>(m_mutex);
  }

private:
  PageHeader m_header;
  std::array<uint8_t, config::PAGE_DATA_SIZE> m_data;
  std::atomic<bool> m_dirty{false};
  std::atomic<int32_t> m_pin_count{0};
  mutable std::shared_mutex m_mutex;
  Timestamp m_last_accessed;
  Timestamp m_last_modified;

  friend class BufferPool;
};

/// @brief Record structure for table data
struct Record {
  RecordId id;
  uint32_t size;
  std::vector<uint8_t> data;

  /// @brief Default constructor
  Record() = default;

  /**
   * @brief Constructor with data
   *
   * @param record_id Record identifier
   * @param record_data Record data span
   */
  Record(RecordId record_id, std::span<const uint8_t> record_data);

  /// @brief Move semantics
  Record(Record &&) noexcept = default;
  Record &operator=(Record &&) noexcept = default;

  /// @brief Non-copyable
  Record(const Record &) = delete;
  Record &operator=(const Record &) = delete;
};

/// @brief Statistics for monitoring storage engine performance
struct StorageStatistics {
  uint64_t total_pages{0};      ///< Total number of pages
  uint64_t free_pages{0};       ///< Number of free pages
  uint64_t buffer_hits{0};      ///< Buffer pool cache hits
  uint64_t buffer_misses{0};    ///< Buffer pool cache misses
  uint64_t disk_reads{0};       ///< Number of disk reads
  uint64_t disk_writes{0};      ///< Number of disk writes
  uint64_t records_inserted{0}; ///< Number of records inserted
  uint64_t records_updated{0};  ///< Number of records updated
  uint64_t records_deleted{0};  ///< Number of records deleted
  double cache_hit_ratio{0.0};  ///< Cache hit ratio (0.0 - 1.0)

  /// @brief Update cache hit ratio
  void update_cache_hit_ratio() noexcept {
    auto total = buffer_hits + buffer_misses;
    cache_hit_ratio =
        total > 0 ? static_cast<double>(buffer_hits) / total : 0.0;
  }
};

/// @brief Configuration for storage engine
struct StorageConfig {
  std::filesystem::path data_directory; ///< Data directory path
  size_t buffer_pool_size{
      config::DEFAULT_BUFFER_POOL_SIZE}; ///< Buffer pool size
  bool enable_wal{true};                 ///< Enable write-ahead logging
  bool enable_checksums{true};           ///< Enable page checksums
  bool enable_compression{false};        ///< Enable page compression
  size_t max_file_size{1ULL << 32};      ///< Maximum file size (4GB)

  /// @brief Validate configuration
  /// @return true if configuration is valid
  [[nodiscard]] bool is_valid() const noexcept;
};

/**
 * @brief Main storage engine class
 */
class StorageEngine {
public:
  /**
   * @brief Constructor
   * @param config Storage engine configuration
   */
  explicit StorageEngine(StorageConfig config);

  /// @brief Destructor
  ~StorageEngine();

  /// @brief Non-copyable, non-movable
  StorageEngine(const StorageEngine &) = delete;
  StorageEngine &operator=(const StorageEngine &) = delete;
  StorageEngine(StorageEngine &&) = delete;
  StorageEngine &operator=(StorageEngine &&) = delete;

  /**
   * @brief Initialize storage engine
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> initialize();

  /**
   * @brief Shutdown storage engine gracefully
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> shutdown();

  /**
   * @brief Check if storage engine is initialized
   * @return true if initialized
   */
  [[nodiscard]] bool is_initialized() const noexcept;

  // Table Operations
  /**
   * @brief Create a new table
   *
   * @param table_name Name of the table
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> create_table(std::string_view table_name);

  /**
   * @brief Drop an existing table
   *
   * @param table_name Name of the table
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> drop_table(std::string_view table_name);

  /**
   * @brief Check if table exists
   *
   * @param table_name Name of the table
   * @return Result with boolean indicating existence or error
   */
  [[nodiscard]] Result<bool> table_exists(std::string_view table_name) const;

  /**
   * @brief Get the list of all tables
   * @return Result with vector of table names or error
   */
  [[nodiscard]] Result<std::vector<std::string>> list_tables() const;

  // Record operations
  /**
   * @brief Insert a record into a table
   *
   * @param table_name Name of the table
   * @param data Span of record data
   * @return Result with RecordId of inserted record or error
   */
  [[nodiscard]] Result<RecordId> insert_record(std::string_view table_name,
                                               std::span<const uint8_t> data);

  /**
   * @brief Get a record by ID
   *
   * @param table_name Name of the table
   * @param record_id Identifier of the record
   * @return Result with Record or error
   */
  [[nodiscard]] Result<Record> get_record(std::string_view table_name,
                                          RecordId record_id) const;

  /**
   * @brief Update a record
   *
   * @param table_name Name of the table
   * @param record_id Identifier of the record
   * @param data New record data span
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> update_record(std::string_view table_name,
                                           RecordId record_id,
                                           std::span<const uint8_t> data);

  /**
   * @brief Delete a record
   *
   * @param table_name Name of the table
   * @param record_id Identifier of the record
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> delete_record(std::string_view table_name,
                                           RecordId record_id);

  // Page operations (for internal use and advanced operations)
  /**
   * @brief Get a page by ID
   *
   * @param page_id Identifier of the page
   * @return Result with shared pointer to Page or error
   */
  [[nodiscard]] Result<std::shared_ptr<Page>> get_page(PageId page_id) const;

  /**
   * @brief Allocate a new page
   * @return Result with new page ID
   */
  [[nodiscard]] Result<PageId> allocate_page();

  /**
   * @brief Deallocate a page
   *
   * @param page_id Identifier of the page to deallocate
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> deallocate_page(PageId page_id);

  // Transaction operations
  /**
   * @brief Begin a new transaction
   * @return Result with new TransactionId or error
   */
  [[nodiscard]] Result<TransactionId> begin_transaction();

  /**
   * @brief Commit a transaction
   *
   * @param txn_id Transaction identifier
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> commit_transaction(TransactionId txn_id);

  /**
   * @brief Rollback a transaction
   *
   * @param txn_id Transaction identifier
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> rollback_transaction(TransactionId txn_id);

  // Maintenance operations
  /**
   * @brief Force a checkpoint
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> checkpoint();

  /**
   * @brief Vacuum unused space
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> vacuum();

  /**
   * @brief Verify database integrity
   * @return Result indicating success or error
   */
  [[nodiscard]] Result<void> verify_integrity();

  // Statistics and monitoring
  /**
   * @brief Get current storage statistics
   * @return StorageStatistics structure
   */
  [[nodiscard]] StorageStatistics get_statistics() const noexcept;

  /**
   * @brief Get configuration
   * @return Storage configuration
   */
  [[nodiscard]] const StorageConfig &get_config() const noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

// C-style inference for FFI (keep the same for Rust compatibility)
extern "C" {
void *velox_storage_create(const char *data_directory);
void velox_storage_destroy(void *engine);
int velox_storage_initialize(void *engine);
int velox_storage_create_table(void *engine, const char *table_name);
int velox_storage_table_exists(void *engine, const char *table_name);
uint64_t velox_storage_insert_record(void *engine, const char *table_name,
                                     const uint8_t *data, uint32_t size);
int velox_storage_get_record(void *engine, const char *table_name,
                             uint64_t record_id, uint8_t *buffer,
                             uint32_t *size);
}
} // namespace velox::storage

// Custom formatter for StorageError
template <> struct fmt::formatter<velox::storage::StorageError> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(velox::storage::StorageError error, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", velox::storage::to_string(error));
  }
};