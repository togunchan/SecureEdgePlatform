# CppMiniDB

CppMiniDB is a lightweight C++20 library for tabular data storage. It offers an in-memory table abstraction with optional persistence to CSV-style files and JSON import/export utilities. The library is intended for embedded scenarios where a full database engine would be overkill, yet structured sensor or logging data must be captured, queried, and replayed reliably.

---

## Table of Contents
1. [Highlights](#highlights)
2. [Architecture](#architecture)
3. [Building](#building)
4. [Usage](#usage)
5. [API Overview](#api-overview)
6. [Persistence](#persistence)
7. [Queries & Updates](#queries--updates)
8. [JSON Utilities](#json-utilities)
9. [Testing](#testing)
10. [Project Layout](#project-layout)
11. [Extending MiniDB](#extending-minidb)
12. [Troubleshooting](#troubleshooting)
13. [Dependencies](#dependencies)
14. [Contact](#contact)

---

## Highlights

- **Header-first design** with a compact implementation in `src/MiniDB.cpp`.
- **Schema-aware table**: define columns and types, then insert rows aligned to the schema.
- **In-memory operations**: insert, update, delete, and query rows without disk I/O.
- **Persistence hooks**: save to `.tbl` files and reload on demand.
- **JSON integration**: import/export JSON arrays for interoperability with external tools.
- **Condition-based filtering** across memory or disk, including multi-clause queries.

CppMiniDB serves as the storage engine for `SensorSimulator` but can be embedded in any standalone C++ project that needs lightweight tabular logging.

---

## Architecture

```
+-------------------+      +------------------+      +--------------------+
|  Application Code | ---> |     MiniDB       | ---> |   File System /    |
|  (e.g. EdgeShell) |      |  In-memory table |      |   JSON Export      |
+-------------------+      +------------------+      +--------------------+
         |                         |                           |
         |                         |                           |
         |                         v                           |
         |                +------------------+                 |
         |                | Query / Update   |                 |
         |                | Operations       |                 |
         |                +------------------+                 |
         |                         |                           |
         +-------------------------+---------------------------+
```

- **In-memory table**: rows and columns stored as vectors of strings with per-column type metadata to aid comparisons.
- **Persistence layer**: `.tbl` files are written under `data/` by default; JSON export/import bridges MiniDB with REST APIs or scripting environments.
- **Concurrency**: a `std::mutex` guards shared state so that read/write operations can be invoked from multiple threads.

---

## Building

CppMiniDB is included as a subdirectory of the SecureEdgePlatform project. To build it individually:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target minidb         # library
cmake --build . --target test_minidb    # Catch2 test suite
```

The library target is named `minidb`. You can link it into your projects using CMake:

```cmake
add_subdirectory(CppMiniDB)
target_link_libraries(my_app PRIVATE minidb)
```

---

## Usage

### 1. Create a MiniDB instance

```c++
#include "cppminidb/MiniDB.hpp"

MiniDB db("temperature_logs");
db.setColumns({"timestamp_ms", "sensor_id", "value"},
              {MiniDB::ColumnType::Int, MiniDB::ColumnType::String, MiniDB::ColumnType::Float});
```

### 2. Insert rows

```c++
db.insertRow({"1000", "TEMP-001", "24.8"});
db.insertRow({"2000", "TEMP-001", "24.9"});
```

### 3. Query

```c++
auto recent = db.selectWhereFromMemory("sensor_id", "==", "TEMP-001");
for (const auto &row : recent)
{
    std::cout << row.at("timestamp_ms") << " → " << row.at("value") << '\n';
}
```

### 4. Persist and reload

```c++
db.save();                 // writes data/temperature_logs.tbl
auto fromDisk = db.loadFromDisk();
```

---

## API Overview

The core API lives in `include/cppminidb/MiniDB.hpp`. Key operations include:

- `setColumns(names)` / `setColumns(names, types)` &mdash; define the table schema.
- `insertRow(values)` &mdash; append a row (vector size must match the column count).
- `selectAll()` &mdash; return all in-memory rows as a vector of maps.
- `save()` &mdash; flush in-memory rows to disk (CSV-like format).
- `clear()` &mdash; remove all rows and truncate the `.tbl` file while preserving the header.
- `clearMemory()` / `clearDisk()` &mdash; clear only memory or the on-disk file.
- `columnTypeOf(name)` &mdash; inspect declared column types.
- `rowCount()` / `columnCount()` &mdash; quick metrics for diagnostics.
- `appendLog()` / `getLogs()` &mdash; specialised helpers used by SensorSimulator for structured sensor logs.

Refer to the header for additional helpers such as `tryParseInt`, `tryParseFloat`, or `hasColumn`.

---

## Persistence

### File Layout

The `.tbl` format uses a CSV-like structure:

```
timestamp_ms,sensor_id,value
1000,TEMP-001,24.8
2000,TEMP-001,24.9
```

- Files are written to `data/<tableName>.tbl` unless you customise `getTableFilePath()`.
- `save()` overwrites the file with the current schema and rows.
- `loadFromDisk()` reads existing `.tbl` files and returns rows as maps.
- `clearDisk(true)` truncates the file but preserves the header, which is useful for resetting logs between runs.

### Disk vs Memory Queries

Most operations have twin variants that read either the in-memory state or the persisted file:

- `selectWhereFromMemory` / `selectWhereFromDisk`
- `updateWhereFromMemory` / `updateWhereFromDisk`
- `deleteWhereFromMemory` / `deleteWhereFromDisk`

This allows you to treat the persisted file as the source of truth when necessary.

---

## Queries & Updates

MiniDB filters rows using simple relational operators (`==`, `!=`, `<`, `>`, `<=`, `>=`). The type metadata defined in `setColumns` ensures numeric comparisons are handled correctly. For multi-clause filtering, use `selectWhereMulti` with a vector of `Condition` objects.

Updates and deletes follow the same interface and can operate on memory or disk. Disk operations rewrite the underlying file, so consider calling `save()` after in-memory updates if you want changes persisted.

---

## JSON Utilities

- `exportToJson()` &mdash; serialize in-memory rows as a JSON array.
- `exportToJsonFromDisk()` &mdash; read the `.tbl` file and convert it to JSON.
- `importFromJson(jsonString)` &mdash; populate in-memory rows from a JSON array.
- `importFromJsonToDisk(jsonString, append)` &mdash; write rows directly to disk, optionally appending to existing files.

These helpers rely on the bundled `nlohmann::json` header (`third_party/json`). They are useful for REST endpoints, telemetry dumps, or integration tests that exchange JSON payloads.

---

## Testing

CppMiniDB ships with a Catch2-based suite in `tests/test_minidb.cpp`. Run it from the build directory:

```bash
ctest --output-on-failure
# or
./CppMiniDB/test_minidb
```

The tests cover schema setup, row insertion, persistence, and JSON workflows. Extend them if you add new functionality.

---

## Project Layout

```text
CppMiniDB/
├── include/
│   └── cppminidb/
│       └── MiniDB.hpp      # Public API
├── src/
│   └── MiniDB.cpp          # Implementation
├── tests/
│   └── test_minidb.cpp     # Catch2 tests
└── CMakeLists.txt          # CMake targets and dependencies
```

---

## Extending MiniDB

1. **Custom storage locations**
   - Override `getTableFilePath()` or wrap MiniDB to choose a different folder structure.

2. **Additional column types**
   - Extend `ColumnType` enum and update parsing/comparison helpers accordingly.

3. **Advanced querying**
   - Build higher-level DSL helpers around `selectWhereMulti` if you need AND/OR combinations or aggregates.

4. **Threading model**
   - `mtx_` protects core data structures. For high-throughput workloads, consider adding read-write locks or batched operations.

---

## Troubleshooting

- **Empty results after reload**: ensure `save()` has been called before `loadFromDisk()`. The file is created lazily.
- **Schema mismatch during JSON import**: if columns already exist, JSON input must match the declared schema exactly.
- **Numeric comparisons failing**: verify that the column type was set to `Int` or `Float`. String columns compare lexicographically.
- **File permission errors**: the default `data/` directory must be writable. Create it manually if the application lacks permissions.

---

## Dependencies

- **nlohmann/json** (bundled in `third_party/json`) for JSON parsing/serialization.
- **Catch2 v3** for unit testing (pulled in via the parent project).
- **C++ Standard Library** (threads, mutex, filesystem, iostream, etc.).

No external database engine or runtime is required.

---

## Contact
Questions, feedback, ideas?

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
