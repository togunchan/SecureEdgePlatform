# CppMiniDB

A minimal, embeddable table store for C++ with **in-memory** and **on-disk** (CSV-like) persistence, **type-aware filtering**, and **JSON import/export**. Designed for constrained or simulated embedded environments where a full RDBMS is overkill.  
CppMiniDB is part of the broader **SecureEdgePlatform** project, providing a lightweight data layer for embedded and simulated environments.
## Features

- **Schema definition**: `setColumns(names)` or `setColumns(names, types)`
- **Types**: `String`, `Int`, `Float` (per-column)
- **Insert / Select / Update / Delete**
  - In-memory operations
  - On-disk operations via `.tbl` files (CSV-like)
- **Type-aware filtering**
  - Strings: `==`, `!=`
  - Numeric (Int/Float): `<`, `<=`, `==`, `!=`, `>=`, `>`
- **JSON import/export**
  - Export in-memory table to JSON array (`exportToJson`)
  - Export on-disk table to JSON array (`exportToJsonFromDisk`)
  - Import JSON array into memory (`importFromJson`)
  - Write JSON array directly to disk (`importFromJsonToDisk`, append or overwrite)
- **Clear helpers**
  - `clear()` (truncate file & keep header), `clearMemory()`, `clearDisk(keepHeader)`

## File Layout & Persistence

- Tables are stored as CSV-like `.tbl` files under `./data/`.
- First line is the **header** (column names, comma-separated).
- Each subsequent line is a row; cells are written as raw strings (no quoting/escaping by default).

**Example Table (.tbl format):**

| Name  | Age | Country |
|-------|-----|---------|
| Alice | 30  | USA     |
| Bob   | 25  | Canada  |

**Equivalent JSON (after export):**

```json
[
  { "Name": "Alice", "Age": 30, "Country": "USA" },
  { "Name": "Bob",   "Age": 25, "Country": "Canada" }
]
```
> ⚠️ Note: Because the on-disk format is a simple CSV-like text, values containing commas/newlines/quotes are not escaped in the current version. Prefer simple, delimiter-free strings or extend the writer/reader to add quoting/escaping.

## Build & Test

This module uses CMake. Dependencies:
- **Catch2** (for tests) — already present under `third_party/Catch2`
- **nlohmann/json** — present under `third_party/json` (single-header include)

```bash
# From project root
mkdir -p build && cd build
cmake ..
make

# Run unit tests
ctest --output-on-failure
# or
./test_minidb
```
## Usage Examples

### 1. Define Schema and Insert Rows
```cpp
MiniDB db;
db.setColumns({"Name", "Age", "Country"}, {MiniDB::String, MiniDB::Int, MiniDB::String});

db.insertRow({"Alice", "30", "USA"});
db.insertRow({"Bob", "25", "Canada"});
```

### 2. Select with Filter (In-Memory)
```cpp
auto results = db.selectWhereFromMemory("Age", ">", "26");
// returns only Alice
```

### 3. Export to JSON
```cpp
std::string jsonOut = db.exportToJson();
// => "[{\"Name\":\"Alice\",\"Age\":30,\"Country\":\"USA\"}, ...]"
```

### 4. Import from JSON
```cpp
std::string jsonData = R"([
  {"Name": "Charlie", "Age": 40, "Country": "UK"}
])";
db.importFromJson(jsonData);
```

### 5. Work with Disk Persistence
```cpp
MiniDB fileDb("users"); // creates ./data/users.tbl
fileDb.setColumns({"ID", "Score"}, {MiniDB::Int, MiniDB::Float});
fileDb.insertRow({"1", "98.5"});
fileDb.insertRow({"2", "76.0"});

// Export file-based table to JSON
std::string jsonDisk = fileDb.exportToJsonFromDisk();
```

## Quick Status

| Feature                     | Status   | Notes                                     |
|------------------------------|----------|------------------------------------------|
| Core CRUD (memory & disk)   | ✅ Done  | Fully implemented                         |
| Type-aware filtering        | ✅ Done  | Supports string and numeric comparisons   |
| JSON import/export          | ✅ Done  | In-memory and on-disk                     |
| Extensive unit tests        | ✅ Done  | Catch2-based, covers edge cases           |
| ORDER BY / LIMIT / LIKE     | ⏭️ Optional | Planned for future versions            |
| Simple indexing / quoting   | ⏭️ Optional | Not yet implemented                    |

## API Reference

### Types
```cpp
enum class MiniDB::ColumnType { String, Int, Float };
```

---

### Construction
| Signature | Description |
|---|---|
| `MiniDB(const std::string& tableName)` | Creates an instance bound to a table name; the same name is used for persistence (`./data/<table>.tbl`). |

---

### Schema
| Signature | Description |
|---|---|
| `void setColumns(const std::vector<std::string>& names)` | Defines schema with names only; all column types default to `String`. |
| `void setColumns(const std::vector<std::string>& names, const std::vector<ColumnType>& types)` | Defines schema with explicit per-column types; throws if sizes mismatch or names are empty. |
| `ColumnType columnTypeOf(const std::string& name) const` | Returns the declared type of a column; throws if the column does not exist. |
| `bool hasColumn(const std::string& name) const` | Checks whether a column exists in the current schema. |
| `std::size_t columnCount() const noexcept` | Returns number of defined columns. |

---

### Insert / Read (Memory & Disk)
| Signature | Description |
|---|---|
| `void insertRow(const std::vector<std::string>& values)` | Inserts one row into memory; value count must equal column count. |
| `std::vector<std::map<std::string,std::string>> selectAll() const` | Returns all in-memory rows as `{col→value}` maps. |
| `std::vector<std::map<std::string,std::string>> loadFromDisk() const` | Loads rows from disk and returns `{col→value}` maps (aligned by header). |
| `void save() const` | Writes the in-memory table to `./data/<table>.tbl` (header + rows). |

---

### Update
| Signature | Description |
|---|---|
| `void updateWhereFromMemory(const std::string& column, const std::string& op, const std::string& value, const std::map<std::string,std::string>& updateMap)` | Updates matching rows in memory according to `updateMap`. |
| `void updateWhereFromDisk(const std::string& column, const std::string& op, const std::string& value, const std::map<std::string,std::string>& updateMap)` | Updates matching rows on disk using a temporary file for atomic replacement. |

---

### Delete
| Signature | Description |
|---|---|
| `void deleteWhereFromMemory(const std::string& column, const std::string& op, const std::string& value)` | Removes matching rows from memory. |
| `void deleteWhereFromDisk(const std::string& column, const std::string& op, const std::string& value)` | Removes matching rows on disk by writing only non-matching rows to a temp file, then renaming. |

---

### JSON I/O
| Signature | Description |
|---|---|
| `std::string exportToJson() const` | Serializes the in-memory table to a JSON array string (nlohmann/json-based). |
| `std::string exportToJsonFromDisk() const` | Serializes the on-disk table to a JSON array string. |
| `std::string exportToJsonLegacy() const` | Legacy JSON exporter (manual string building); kept for compatibility. |
| `void importFromJson(const std::string& jsonString)` | Loads rows into memory from a JSON array of objects; infers schema if empty, otherwise validates. |
| `void importFromJsonToDisk(const std::string& jsonString, bool append=false)` | Writes a JSON array of objects directly to the `.tbl` file; `append=true` enforces header compatibility. |

---

### Clear Helpers
| Signature | Description |
|---|---|
| `void clear()` | Clears memory and truncates the on-disk file to header (schema preserved). |
| `void clearMemory()` | Clears only in-memory rows (does not touch disk or schema). |
| `void clearDisk(bool keepHeader=true)` | Clears the on-disk file; keeps the header when `keepHeader` is true. |
| `std::size_t rowCount() const noexcept` | Returns the count of in-memory rows (not on-disk). |

---

### Filtering & Operators
| Signature | Description |
|---|---|
| `std::vector<std::map<std::string,std::string>> selectWhereFromMemory(const std::string& column, const std::string& op, const std::string& value) const` | In-memory selection by condition. |
| `std::vector<std::map<std::string,std::string>> selectWhereFromDisk(const std::string& column, const std::string& op, const std::string& value) const` | On-disk selection by condition. |
| `static bool isOpAllowedForType(const std::string& op, ColumnType t)` | Validates that an operator is permitted for the column type (String: `==`,`!=`; Numeric: `<,<=,==,!=,>=,>`). |
| `bool compareNumeric(int a, const std::string& op, int b) const` | Applies a comparison operator to integers. |
| `bool compareString(std::string a, const std::string& op, std::string b) const` | Applies `==`/`!=` to strings. |
| `static bool tryParseInt(const std::string& s, int& out)` | Attempts to parse a signed integer (validate + `stoi`). |
| `static bool tryParseFloat(const std::string& s, double& out)` | Attempts to parse a floating-point number (validate + `stof`). |

> **Note – Type-Aware Filtering**  
> • `String` columns support only `==` and `!=`.  
> • `Int`/`Float` columns support `<, <=, ==, !=, >=, >`.  
> • For numeric comparisons, the right-hand-side value is parsed once; each row's cell is parsed per-iteration.  
> • Cells that cannot be parsed as numbers (e.g., `"N/A"`) are skipped for numeric comparisons.

## Design Notes & Limits

- **CSV-like format**: Minimalist, no quoting/escaping — keep values simple.
- **Type enforcement**: Basic `String`, `Int`, `Float` only. No composite/complex types.
- **Atomic writes**: Current version appends directly; no transactional guarantees.
- **Indexing**: None; full scan per query. Suitable for small/medium tables.
- **Filtering rules**: Strings → only `==` / `!=`; Numerics → full comparison ops.

## Roadmap

- Add support for `ORDER BY`, `LIMIT`, and `LIKE` operations.
- Optional indexing (hash-based or B-tree) for faster lookups.
- Escaped/quoted CSV writing and parsing.
- Basic transactional file writes (atomic replace strategy).

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Contact
Questions, feedback, ideas?

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
