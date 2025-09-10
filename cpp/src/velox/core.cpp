#include <fstream>
#include <mutex>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <unordered_map>
#include <velox/core.hpp>

namespace velox {
namespace log {
namespace {
std::mutex loggers_mutex;
std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers;
bool initialized = false;
} // namespace

void initialize(spdlog::level::level_enum level, const std::string &pattern) {
  std::lock_guard<std::mutex> lock(loggers_mutex);
  if (initialized) {
    return;
  }

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(level);
  console_sink->set_pattern(pattern);

  auto default_logger =
      std::make_shared<spdlog::logger>("default", console_sink);
  default_logger->set_level(level);

  spdlog::set_default_logger(default_logger);
  spdlog::set_level(level);

  loggers["default"] = default_logger;
  initialized = true;

  default_logger->info("VeloxDB logging initialized (level: {})",
                       spdlog::level::to_string_view(level));
}

std::shared_ptr<spdlog::logger> get_logger(const std::string &name) {
  std::lock_guard<std::mutex> lock(loggers_mutex);
  if (!initialized) {
    initialize();
  }

  auto it = loggers.find(name);
  if (it != loggers.end()) {
    return it->second;
  }

  auto default_logger = spdlog::default_logger();
  auto logger = std::make_shared<spdlog::logger>(
      name, default_logger->sinks().begin(), default_logger->sinks().end());

  logger->set_level(default_logger->level());
  loggers[name] = logger;

  return logger;
}

void set_level(spdlog::level::level_enum level) {
  spdlog::set_level(level);
  std::lock_guard<std::mutex> lock(loggers_mutex);

  for (auto &[name, logger] : loggers) {
    logger->set_level(level);
  }
}
} // namespace log

namespace config {
bool SystemConfig::validate() const noexcept {
  return buffer_pool_size >= constants::MIN_BUFFER_POOL_SIZE &&
         buffer_pool_size <= constants::MAX_BUFFER_POOL_SIZE &&
         max_connections > 0 && worker_threads > 0 && !data_directory.empty() &&
         !log_directory.empty();
}

error::Result<SystemConfig>
SystemConfig::load(const std::filesystem::path &file) {
  if (!std::filesystem::exists(file)) {
    return error::error<SystemConfig>(error::ErrorCode::IO_ERROR);
  }

  std::ifstream infile(file);
  if (!infile) {
    return error::error<SystemConfig>(error::ErrorCode::IO_ERROR);
  }

  SystemConfig config;
  std::string line;

  try {
    while (std::getline(infile, line)) {
      if (line.empty() || line[0] == '#') {
        continue;
      }

      auto pos = line.find("=");
      if (pos == std::string::npos) {
        continue;
      }

      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      key.erase(0, key.find_first_not_of(" \t"));
      key.erase(key.find_last_not_of(" \t") + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);

      if (key == "buffer_pool_size") {
        config.buffer_pool_size = std::stoull(value);
      } else if (key == "max_connections") {
        config.max_connections = std::stoull(value);
      } else if (key == "worker_threads") {
        config.worker_threads = std::stoull(value);
      } else if (key == "data_directory") {
        config.data_directory = value;
      } else if (key == "log_directory") {
        config.log_directory = value;
      } else if (key == "enable_wal") {
        config.enable_wal = (value == "true" || value == "1");
      } else if (key == "enable_checksums") {
        config.enable_checksums = (value == "true" || value == "1");
      } else if (key == "enable_compression") {
        config.enable_compression = (value == "true" || value == "1");
      } else if (key == "log_level") {
        if (value == "trace")
          config.log_level = spdlog::level::trace;
        else if (value == "debug")
          config.log_level = spdlog::level::debug;
        else if (value == "info")
          config.log_level = spdlog::level::info;
        else if (value == "warn")
          config.log_level = spdlog::level::warn;
        else if (value == "error")
          config.log_level = spdlog::level::err;
        else if (value == "critical")
          config.log_level = spdlog::level::critical;
      }
    }
  } catch (const std::exception &e) {
    return error::error<SystemConfig>(error::ErrorCode::INVALID_ARGUMENT);
  }

  if (!config.validate()) {
    return error::error<SystemConfig>(error::ErrorCode::INVALID_ARGUMENT);
  }

  return error::ok(std::move(config));
}

error::VoidResult SystemConfig::save(const std::filesystem::path &file) const {
  if (!validate()) {
    return error::error<void>(error::ErrorCode::INVALID_ARGUMENT);
  }

  std::ofstream outfile(file);
  if (!outfile) {
    return error::error<void>(error::ErrorCode::IO_ERROR);
  }

  try {
    outfile << "# VeloxDB Configuration File\n";
    outfile << "# Generated automatically\n\n";

    outfile << "buffer_pool_size=" << buffer_pool_size << "\n";
    outfile << "max_connections=" << max_connections << "\n";
    outfile << "worker_threads=" << worker_threads << "\n";
    outfile << "data_directory=" << data_directory.string() << "\n";
    outfile << "log_directory=" << log_directory.string() << "\n";
    outfile << "enable_wal=" << (enable_wal ? "true" : "false") << "\n";
    outfile << "enable_checksums=" << (enable_checksums ? "true" : "false")
            << "\n";
    outfile << "enable_compression=" << (enable_compression ? "true" : "false")
            << "\n";

    std::string level_str;
    switch (log_level) {
    case spdlog::level::trace:
      level_str = "trace";
      break;
    case spdlog::level::debug:
      level_str = "debug";
      break;
    case spdlog::level::info:
      level_str = "info";
      break;
    case spdlog::level::warn:
      level_str = "warn";
      break;
    case spdlog::level::err:
      level_str = "error";
      break;
    case spdlog::level::critical:
      level_str = "critical";
      break;
    default:
      level_str = "info";
      break;
    }
    outfile << "log_level=" << level_str << "\n";

  } catch (const std::exception &) {
    return error::error<void>(error::ErrorCode::IO_ERROR);
  }

  return error::ok();
}

namespace {
SystemConfig global_config_instance;
std::once_flag config_init_flag;
} // namespace

SystemConfig &global_config() {
  std::call_once(config_init_flag, []() {
    auto config_file = std::filesystem::current_path() / "veloxdb.conf";
    if (std::filesystem::exists(config_file)) {
      auto loaded = SystemConfig::load(config_file);
      if (loaded.has_value()) {
        global_config_instance = std::move(loaded.value());
        return;
      }
    }
  });

  return global_config_instance;
}

} // namespace config
} // namespace velox