/**
 * @file dtypes.hpp
 * @author Carlos Salguero
 * @brief Data type definitions and utilities for VeloxDB
 * @version 0.1
 * @date 2025-09-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <velox/concepts.hpp>

namespace velox::dtypes {
/// @brief Fundamental type identifiers
enum class TypeId : uint8_t {
  NULL_TYPE = 0,  ///< NULL value
  BOOLEAN = 1,    ///< Boolean (true/false)
  TINYINT = 2,    ///< 8-bit signed integer
  SMALLINT = 3,   ///< 16-bit signed integer
  INTEGER = 4,    ///< 32-bit signed integer
  BIGINT = 5,     ///< 64-bit signed integer
  REAL = 6,       ///< 32-bit floating point
  DOUBLE = 7,     ///< 64-bit floating point
  DECIMAL = 8,    ///< Fixed-point decimal
  VARCHAR = 9,    ///< Variable-length string
  CHAR = 10,      ///< Fixed-length string
  TEXT = 11,      ///< Large text object
  BLOB = 12,      ///< Binary large object
  DATE = 13,      ///< Date value
  TIME = 14,      ///< Time value
  TIMESTAMP = 15, ///< Timestamp value
  INTERVAL = 16,  ///< Time interval
  UUID = 17,      ///< UUID value
  JSON = 18,      ///< JSON document
  ARRAY = 19,     ///< Array of values
  STRUCT = 20,    ///< Structured type
  MAP = 21,       ///< Key-value mapping
  CUSTOM = 255    ///< Custom user-defined type
};

/// @brief Convert TypeId to string
[[nodiscard]] constexpr std::string_view to_string(TypeId type_id) noexcept {
  switch (type_id) {
  case TypeId::NULL_TYPE:
    return "NULL";
  case TypeId::BOOLEAN:
    return "BOOLEAN";
  case TypeId::TINYINT:
    return "TINYINT";
  case TypeId::SMALLINT:
    return "SMALLINT";
  case TypeId::INTEGER:
    return "INTEGER";
  case TypeId::BIGINT:
    return "BIGINT";
  case TypeId::REAL:
    return "REAL";
  case TypeId::DOUBLE:
    return "DOUBLE";
  case TypeId::DECIMAL:
    return "DECIMAL";
  case TypeId::VARCHAR:
    return "VARCHAR";
  case TypeId::CHAR:
    return "CHAR";
  case TypeId::TEXT:
    return "TEXT";
  case TypeId::BLOB:
    return "BLOB";
  case TypeId::DATE:
    return "DATE";
  case TypeId::TIME:
    return "TIME";
  case TypeId::TIMESTAMP:
    return "TIMESTAMP";
  case TypeId::INTERVAL:
    return "INTERVAL";
  case TypeId::UUID:
    return "UUID";
  case TypeId::JSON:
    return "JSON";
  case TypeId::ARRAY:
    return "ARRAY";
  case TypeId::STRUCT:
    return "STRUCT";
  case TypeId::MAP:
    return "MAP";
  case TypeId::CUSTOM:
    return "CUSTOM";
  }
  return "UNKNOWN";
}

/// @brief Get size in bytes for fixed-size types
[[nodiscard]] constexpr size_t type_size(TypeId type_id) noexcept {
  switch (type_id) {
  case TypeId::NULL_TYPE:
    return 0;
  case TypeId::BOOLEAN:
    return 1;
  case TypeId::TINYINT:
    return 1;
  case TypeId::SMALLINT:
    return 2;
  case TypeId::INTEGER:
    return 4;
  case TypeId::BIGINT:
    return 8;
  case TypeId::REAL:
    return 4;
  case TypeId::DOUBLE:
    return 8;
  case TypeId::DATE:
    return 4;
  case TypeId::TIME:
    return 8;
  case TypeId::TIMESTAMP:
    return 8;
  case TypeId::UUID:
    return 16;
  default:
    return 0;
  }
}

/// @brief Check if type is variable length
[[nodiscard]] constexpr bool is_variable_length(TypeId type_id) noexcept {
  return type_size(type_id) == 0 && type_id != TypeId::NULL_TYPE;
}

/// @brief Check if type is numeric
[[nodiscard]] constexpr bool is_numeric(TypeId type_id) noexcept {
  return type_id >= TypeId::TINYINT && type_id <= TypeId::DECIMAL;
}

/// @brief Check if type is string-like
[[nodiscard]] constexpr bool is_string(TypeId type_id) noexcept {
  return type_id == TypeId::VARCHAR || type_id == TypeId::CHAR ||
         type_id == TypeId::TEXT;
}

/// @brief Fixed-point decimal type
struct Decimal {
  int64_t value;     ///< Scaled integer value
  uint8_t precision; ///< Total number of digits
  uint8_t scale;     ///< Number of digits after decimal point

  Decimal() = default;
  Decimal(int64_t val, uint8_t prec, uint8_t sc)
      : value(val), precision(prec), scale(sc) {}

  /// @brief Create from double with specified precision/scale
  static Decimal from_double(double d, uint8_t precision = 10,
                             uint8_t scale = 2);

  /// @brief Convert to double
  [[nodiscard]] double to_double() const noexcept;

  /// @brief String representation
  [[nodiscard]] std::string to_string() const;

  // Arithmetic operators
  Decimal operator+(const Decimal &other) const;
  Decimal operator-(const Decimal &other) const;
  Decimal operator*(const Decimal &other) const;
  Decimal operator/(const Decimal &other) const;

  // Comparison operators
  bool operator==(const Decimal &other) const noexcept;
  bool operator!=(const Decimal &other) const noexcept;
  bool operator<(const Decimal &other) const noexcept;
  bool operator<=(const Decimal &other) const noexcept;
  bool operator>(const Decimal &other) const noexcept;
  bool operator>=(const Decimal &other) const noexcept;
};

/// @brief Date type (days since epoch)
struct Date {
  int32_t days_since_epoch;

  Date() = default;
  explicit Date(int32_t days) : days_since_epoch(days) {}

  /// @brief Create from year, month, day
  static Date from_ymd(int year, int month, int day);

  /// @brief Create from string (YYYY-MM-DD)
  static std::optional<Date> from_string(std::string_view str);

  /// @brief Convert to string
  [[nodiscard]] std::string to_string() const;

  /// @brief Get components
  struct YMD {
    int year, month, day;
  };
  [[nodiscard]] YMD to_ymd() const;

  // Arithmetic
  Date operator+(int days) const noexcept {
    return Date{days_since_epoch + days};
  }
  Date operator-(int days) const noexcept {
    return Date{days_since_epoch - days};
  }
  int operator-(const Date &other) const noexcept {
    return days_since_epoch - other.days_since_epoch;
  }

  // Comparison
  auto operator<=>(const Date &other) const noexcept = default;
};

/// @brief Time type (microseconds since midnight)
struct Time {
  int64_t microseconds_since_midnight;

  Time() = default;
  explicit Time(int64_t micros) : microseconds_since_midnight(micros) {}

  /// @brief Create from hours, minutes, seconds, microseconds
  static Time from_hms(int hour, int minute, int second, int microsecond = 0);

  /// @brief Create from string (HH:MM:SS[.ffffff])
  static std::optional<Time> from_string(std::string_view str);

  /// @brief Convert to string
  [[nodiscard]] std::string to_string() const;

  /// @brief Get components
  struct HMS {
    int hour, minute, second, microsecond;
  };
  [[nodiscard]] HMS to_hms() const;

  // Comparison
  auto operator<=>(const Time &other) const noexcept = default;
};

/// @brief Timestamp type (microseconds since Unix epoch)
struct Timestamp {
  int64_t microseconds_since_epoch;

  Timestamp() = default;
  explicit Timestamp(int64_t micros) : microseconds_since_epoch(micros) {}

  /// @brief Create from system clock
  static Timestamp now();

  /// @brief Create from string (ISO 8601 format)
  static std::optional<Timestamp> from_string(std::string_view str);

  /// @brief Convert to string
  [[nodiscard]] std::string to_string() const;

  /// @brief Convert to chrono time_point
  [[nodiscard]] std::chrono::system_clock::time_point to_chrono() const;

  // Arithmetic
  Timestamp operator+(std::chrono::microseconds duration) const;
  Timestamp operator-(std::chrono::microseconds duration) const;
  std::chrono::microseconds operator-(const Timestamp &other) const;

  // Comparison
  auto operator<=>(const Timestamp &other) const noexcept = default;
};

/// @brief UUID type (128-bit universally unique identifier)
struct UUID {
  std::array<uint8_t, 16> bytes;

  UUID() { bytes.fill(0); }
  explicit UUID(const std::array<uint8_t, 16> &b) : bytes(b) {}

  /// @brief Generate random UUID (version 4)
  static UUID generate();

  /// @brief Create from string representation
  static std::optional<UUID> from_string(std::string_view str);

  /// @brief Convert to string (canonical format)
  [[nodiscard]] std::string to_string() const;

  /// @brief Check if this is a nil UUID
  [[nodiscard]] bool is_nil() const noexcept;

  // Comparison
  auto operator<=>(const UUID &other) const noexcept = default;
};

/// @brief Variant type that can hold any database value
using Value = std::variant<std::nullptr_t,       // NULL
                           bool,                 // BOOLEAN
                           int8_t,               // TINYINT
                           int16_t,              // SMALLINT
                           int32_t,              // INTEGER
                           int64_t,              // BIGINT
                           float,                // REAL
                           double,               // DOUBLE
                           Decimal,              // DECIMAL
                           std::string,          // VARCHAR/CHAR/TEXT
                           std::vector<uint8_t>, // BLOB
                           Date,                 // DATE
                           Time,                 // TIME
                           Timestamp,            // TIMESTAMP
                           UUID                  // UUID
                           >;

/// @brief Get TypeId for a Value
[[nodiscard]] TypeId get_type_id(const Value &value);

/// @brief Get string representation of a Value
[[nodiscard]] std::string value_to_string(const Value &value);

/// @brief Compare two Values
[[nodiscard]] int compare_values(const Value &a, const Value &b);

/// @brief Check if Value is null
[[nodiscard]] bool is_null(const Value &value);

/// @brief Cast Value to specific type
template <typename T>
[[nodiscard]] std::optional<T> cast_value(const Value &value);

/// @brief Serialize Value to bytes
[[nodiscard]] std::vector<uint8_t> serialize_value(const Value &value);

/// @brief Deserialize Value from bytes
[[nodiscard]] std::optional<Value>
deserialize_value(std::span<const uint8_t> data);

/// @brief Type information structure
struct TypeInfo {
  TypeId type_id;
  size_t max_length{0}; ///< For VARCHAR/CHAR types
  uint8_t precision{0}; ///< For DECIMAL types
  uint8_t scale{0};     ///< For DECIMAL types
  bool nullable{true};  ///< Whether NULL values are allowed

  TypeInfo() = default;
  TypeInfo(TypeId id) : type_id(id) {}
  TypeInfo(TypeId id, size_t len) : type_id(id), max_length(len) {}
  TypeInfo(TypeId id, uint8_t prec, uint8_t sc)
      : type_id(id), precision(prec), scale(sc) {}

  /// @brief Get size in bytes for this type
  [[nodiscard]] size_t size() const noexcept;

  /// @brief Check if type is compatible with value
  [[nodiscard]] bool is_compatible(const Value &value) const;

  /// @brief Validate value against this type
  [[nodiscard]] bool validate(const Value &value) const;

  /// @brief String representation
  [[nodiscard]] std::string to_string() const;

  // Comparison
  auto operator<=>(const TypeInfo &other) const noexcept = default;
};

/// @brief Column definition
struct ColumnInfo {
  std::string name;
  TypeInfo type;
  bool primary_key{false};
  bool unique{false};
  bool not_null{false};
  Value default_value{nullptr};
  std::string comment;

  ColumnInfo() = default;
  ColumnInfo(std::string n, TypeInfo t) : name(std::move(n)), type(t) {}

  /// @brief Check if column accepts NULL values
  [[nodiscard]] bool is_nullable() const noexcept {
    return !not_null && !primary_key && type.nullable;
  }

  /// @brief Validate value for this column
  [[nodiscard]] bool validate_value(const Value &value) const;

  /// @brief Get default value for this column
  [[nodiscard]] Value get_default_value() const;
};

/// @brief Table schema definition
struct TableSchema {
  std::string name;
  std::vector<ColumnInfo> columns;
  std::vector<std::string> primary_key_columns;
  std::vector<std::vector<std::string>> unique_constraints;
  std::string comment;

  TableSchema() = default;
  explicit TableSchema(std::string table_name) : name(std::move(table_name)) {}

  /// @brief Add column to schema
  void add_column(ColumnInfo column);

  /// @brief Get column by name
  [[nodiscard]] const ColumnInfo *get_column(const std::string &name) const;

  /// @brief Get column index by name
  [[nodiscard]] std::optional<size_t>
  get_column_index(const std::string &name) const;

  /// @brief Get primary key columns
  [[nodiscard]] std::vector<const ColumnInfo *> get_primary_key_columns() const;

  /// @brief Validate row data against schema
  [[nodiscard]] bool validate_row(const std::vector<Value> &row) const;

  /// @brief Get row size estimate
  [[nodiscard]] size_t estimate_row_size() const;

  /// @brief Serialize schema
  [[nodiscard]] std::vector<uint8_t> serialize() const;

  /// @brief Deserialize schema
  static std::optional<TableSchema> deserialize(std::span<const uint8_t> data);
};

/// @brief Row data structure
struct Row {
  std::vector<Value> values;

  Row() = default;
  explicit Row(size_t column_count) : values(column_count) {}
  explicit Row(std::vector<Value> vals) : values(std::move(vals)) {}

  /// @brief Get value at column index
  [[nodiscard]] const Value &operator[](size_t index) const {
    return values[index];
  }
  [[nodiscard]] Value &operator[](size_t index) { return values[index]; }

  /// @brief Get value by column name (requires schema)
  [[nodiscard]] const Value &get(const TableSchema &schema,
                                 const std::string &column_name) const;
  [[nodiscard]] Value &get(const TableSchema &schema,
                           const std::string &column_name);

  /// @brief Set value at column index
  void set(size_t index, Value value) { values[index] = std::move(value); }

  /// @brief Set value by column name (requires schema)
  void set(const TableSchema &schema, const std::string &column_name,
           Value value);

  /// @brief Number of columns
  [[nodiscard]] size_t size() const noexcept { return values.size(); }

  /// @brief Check if row is empty
  [[nodiscard]] bool empty() const noexcept { return values.empty(); }

  /// @brief Resize row
  void resize(size_t new_size) { values.resize(new_size); }

  /// @brief Clear row
  void clear() { values.clear(); }

  /// @brief Validate row against schema
  [[nodiscard]] bool validate(const TableSchema &schema) const;

  /// @brief Serialize row
  [[nodiscard]] std::vector<uint8_t> serialize() const;

  /// @brief Deserialize row
  static std::optional<Row> deserialize(std::span<const uint8_t> data);

  /// @brief Compare rows (lexicographic order)
  [[nodiscard]] int compare(const Row &other) const;

  // Comparison operators
  bool operator==(const Row &other) const;
  bool operator!=(const Row &other) const;
  bool operator<(const Row &other) const;
  bool operator<=(const Row &other) const;
  bool operator>(const Row &other) const;
  bool operator>=(const Row &other) const;
};

/// @brief Key extraction utilities
namespace key {

/// @brief Extract key from row using column indices
[[nodiscard]] std::vector<Value>
extract_key(const Row &row, const std::vector<size_t> &column_indices);

/// @brief Extract primary key from row
[[nodiscard]] std::vector<Value> extract_primary_key(const Row &row,
                                                     const TableSchema &schema);

/// @brief Create composite key from multiple values
[[nodiscard]] std::vector<uint8_t>
create_composite_key(const std::vector<Value> &key_values);

/// @brief Compare two keys
[[nodiscard]] int compare_keys(const std::vector<Value> &a,
                               const std::vector<Value> &b);

} // namespace key

/// @brief Type conversion utilities
namespace convert {

/// @brief Convert between compatible types
template <typename From, typename To>
[[nodiscard]] std::optional<To> convert(const From &value);

/// @brief Convert string to Value with type inference
[[nodiscard]] Value parse_value(std::string_view str);

/// @brief Convert string to Value with explicit type
[[nodiscard]] std::optional<Value> parse_value(std::string_view str,
                                               TypeId target_type);

/// @brief Convert Value to string representation
[[nodiscard]] std::string format_value(const Value &value);

/// @brief Convert Value to SQL literal string
[[nodiscard]] std::string to_sql_literal(const Value &value);

} // namespace convert

/// @brief Validation utilities
namespace validate {

/// @brief Check if string is valid for VARCHAR/CHAR with length limit
[[nodiscard]] bool is_valid_string(const std::string &str, size_t max_length);

/// @brief Check if decimal value fits in precision/scale
[[nodiscard]] bool is_valid_decimal(const Decimal &decimal, uint8_t precision,
                                    uint8_t scale);

/// @brief Check if date is valid
[[nodiscard]] bool is_valid_date(int year, int month, int day);

/// @brief Check if time is valid
[[nodiscard]] bool is_valid_time(int hour, int minute, int second,
                                 int microsecond = 0);

} // namespace validate

/// @brief Hash functions for database types
namespace hash {

/// @brief Hash function for Value
struct ValueHasher {
  [[nodiscard]] size_t operator()(const Value &value) const noexcept;
};

/// @brief Hash function for Row
struct RowHasher {
  [[nodiscard]] size_t operator()(const Row &row) const noexcept;
};

/// @brief Hash function for composite keys
[[nodiscard]] size_t hash_key(const std::vector<Value> &key);

} // namespace hash
} // namespace velox::dtypes

// Custom formatters for fmt library
template <> struct fmt::formatter<velox::dtypes::TypeId> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(velox::dtypes::TypeId type_id, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", velox::dtypes::to_string(type_id));
  }
};

template <> struct fmt::formatter<velox::dtypes::Value> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const velox::dtypes::Value &value, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}",
                          velox::dtypes::value_to_string(value));
  }
};

template <> struct fmt::formatter<velox::dtypes::Decimal> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const velox::dtypes::Decimal &decimal, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", decimal.to_string());
  }
};

template <> struct fmt::formatter<velox::dtypes::Date> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const velox::dtypes::Date &date, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", date.to_string());
  }
};

template <> struct fmt::formatter<velox::dtypes::Time> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const velox::dtypes::Time &time, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", time.to_string());
  }
};

template <> struct fmt::formatter<velox::dtypes::Timestamp> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const velox::dtypes::Timestamp &timestamp, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", timestamp.to_string());
  }
};

template <> struct fmt::formatter<velox::dtypes::UUID> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const velox::dtypes::UUID &uuid, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", uuid.to_string());
  }
};