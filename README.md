# VeloxDB

### High-Performance Custom Database Engine

<div align="center">

[![Rust](https://img.shields.io/badge/rust-1.70+-000000?style=for-the-badge&logo=rust)](https://www.rust-lang.org/)
[![C++](https://img.shields.io/badge/C++-20-00599C?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge&logo=github-actions)](https://github.com)

**A blazingly fast, modern database engine built with Rust 🦀 and C++ ⚡**

_Demonstrating advanced systems programming with hybrid language architecture_

</div>

## 🚀 Quick Start

```bash
# 🔥 One-command setup
curl -sSL https://install.veloxdb.dev | bash

# 🏗️ Or build from source
git clone https://github.com/yourusername/veloxdb.git
cd veloxdb && ./scripts/build.sh

# ⚡ Start the server
cargo run --bin velox-server --release
```

## 💡 Usage Example

```rust
use velox_client::VeloxClient;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 🔌 Connect to VeloxDB
    let client = VeloxClient::connect("localhost:5432").await?;

    // 📊 Create table with modern syntax
    client.execute("
        CREATE TABLE analytics (
            id BIGINT PRIMARY KEY,
            event_name VARCHAR(100) NOT NULL,
            user_id BIGINT,
            properties JSONB,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ").await?;

    // ⚡ Lightning-fast inserts
    client.execute("
        INSERT INTO analytics (id, event_name, user_id, properties)
        VALUES (1, 'page_view', 12345, '{\"page\": \"/dashboard\"}')
    ").await?;

    // 🔍 Query with blazing speed
    let results = client.query("
        SELECT event_name, COUNT(*) as count
        FROM analytics
        WHERE timestamp >= '2024-01-01'
        GROUP BY event_name
        ORDER BY count DESC
    ").await?;

    println!("📊 Results: {:?}", results);
    Ok(())
}
```

---

## 🏗️ Architecture Overview

<div align="center">

```mermaid
graph TB
    subgraph "🦀 Rust Layer - Safety & Interface"
        A[SQL Parser] --> B[Query Planner]
        B --> C[Execution Engine]
        D[Network Protocol] --> E[Connection Manager]
        F[Client Libraries]
    end

    subgraph "⚡ C++ Core - Maximum Performance"
        G[Storage Engine] --> H[Buffer Pool]
        H --> I[B+ Tree Indexes]
        J[Transaction Manager] --> K[WAL Manager]
    end

    C --> G
    E --> C
    F --> D

    style A fill:#dea584
    style G fill:#83c5be
    style D fill:#006d77
```

</div>

### 🎨 **Hybrid Design Philosophy**

| Component             | Language | Why?                                       |
| --------------------- | -------- | ------------------------------------------ |
| **🔍 SQL Parser**     | 🦀 Rust  | Memory safety, excellent parsing libraries |
| **⚡ Storage Engine** | ⚙️ C++   | Maximum performance, direct memory control |
| **🌐 Networking**     | 🦀 Rust  | Async I/O, safe concurrency                |
| **📊 Indexing**       | ⚙️ C++   | Cache optimization, SIMD instructions      |

---

## 📁 Project Structure

```
veloxdb/
├── 📋 README.md                 # You are here!
├── 🦀 Cargo.toml               # Rust workspace
├── ⚙️ CMakeLists.txt           # C++ build config
├── 🐳 docker-compose.yml       # Dev environment
├── 📚 docs/                    # Architecture & API docs
├── 🔧 scripts/                 # Build & utility scripts
├── 📦 crates/                  # Rust components
│   ├── 🌐 velox-server/        # Database server
│   ├── 📱 velox-client/        # Client library
│   ├── 🔍 velox-query/         # SQL parser & planner
│   ├── 📡 velox-protocol/      # Network protocol
│   └── 🔗 velox-bindings/      # C++ FFI bindings
├── ⚡ cpp/                     # C++ core engine
│   ├── 📂 include/storage/     # Storage engine headers
│   ├── 🗃️ src/storage/         # Implementation
│   └── 🧪 tests/               # C++ unit tests
├── 🧪 tests/                   # Integration tests
└── 📖 examples/                # Usage examples
```
