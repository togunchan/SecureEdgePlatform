# EdgeAgent

EdgeAgent bridges the SecureEdgePlatform sensor pipeline with external telemetry sinks. It consumes `SensorLogRow` data produced by `SensorSimulator`/`CppMiniDB`, normalises it to JSON, and delivers the payload to console, file, or future network endpoints so that downstream systems can ingest clean time-series telemetry.

---

## Table of Contents

1. [Highlights](#highlights)
2. [Architecture](#architecture)
3. [Build & Link](#build--link)
4. [Usage](#usage)
5. [Testing](#testing)
6. [Project Layout](#project-layout)
7. [Extending](#extending)
8. [Dependencies](#dependencies)
9. [Contact](#contact)

---

## Highlights

- **JSON first exporter** — converts `cppminidb::SensorLogRow` batches into well-structured JSON arrays that other services can consume directly.
- **Multi-channel publishing** — ships CLI-friendly console dumps and file-based persistence today, with placeholders for MQTT and REST targets.
- **Seamless simulator integration** — reuses the same log schema emitted by [`SensorSimulator`](../SensorSimulator) so no additional mapping code is required.
- **Modern C++20** — light footprint library with zero global state, ready to embed in larger agents or edge services.

---

## Architecture

```
┌────────────────────┐
│  SensorSimulator   │
│  (EdgeShell, DB)   │
└─────────┬──────────┘
          │ SensorLogRow batches
          ▼
┌────────────────────┐
│ TelemetryPublisher │
│ • toJson           │
│ • publish* targets │
└─────────┬──────────┘
          │
          ├─────────────▶ Console pretty-print
          ├─────────────▶ JSON file export
          ├─────────────▶ MQTT topic (*stub*)
          └─────────────▶ REST POST (*stub*)
```

- **`TelemetryPublisher`**: stateless façade that translates typed sensor rows into JSON and dispatches them to the requested sink.
- **Sinks**: console and file publishing are implemented; `publishToMqtt` and `publishToRest` define the interface for upcoming transports.
- **Data model**: relies on `cppminidb::SensorLogRow` (timestamp, sensor id, value, fault flags) shared with [`CppMiniDB`](../CppMiniDB).

---

## Build & Link

Configure from the repository root and build the static library:

```bash
cmake -S . -B build
cmake --build build --target edge_agent
```

The resulting library lives under `build/EdgeAgent/libedge_agent.a` (or the platform equivalent). Include `<edgeagent/TelemetryPublisher.hpp>` in your application and link against `edge_agent`.

---

## Usage

The `tests/test_telemetry.cpp` target doubles as an executable example:

```cpp
#include "edgeagent/TelemetryPublisher.hpp"
#include "cppminidb/SensorLogRow.hpp"

int main() {
    std::vector<cppminidb::SensorLogRow> logs = {
        {1695979200000ULL, "sensor-001", 36.5, ""},
        {1695979205000ULL, "sensor-001", 36.9, "spike"}
    };

    edgeagent::TelemetryPublisher publisher;

    // Human-friendly dump
    publisher.publishToConsole(logs);

    // Persist to disk
    publisher.publishToFile(logs, "telemetry.json");
}
```

Running the sample binary prints the prettified JSON to stdout and writes a `telemetry.json` file:

```bash
./build/EdgeAgent/test_telemetry
```

---

## Testing

Build and execute the regression example after configuring the project:

```bash
cmake --build build --target test_telemetry
./build/EdgeAgent/test_telemetry
```

The program exits with a non-zero status if file export fails, making it suitable for simple CI checks.

---

## Project Layout

```text
EdgeAgent/
├── include/
│   ├── cppminidb/               # Shared SensorLogRow struct
│   └── edgeagent/               # Public TelemetryPublisher interface
├── src/
│   └── TelemetryPublisher.cpp   # JSON conversion + console/file sinks
└── tests/
    └── test_telemetry.cpp       # Usage demo and regression harness
```

---

## Extending

1. **Implement MQTT publishing**
   - Provide a transport in `TelemetryPublisher::publishToMqtt`.
   - Decide on QoS/topic conventions and surface configuration hooks in your agent.
2. **Add REST support**
   - Implement `publishToRest` to POST the JSON payload.
   - Consider batching and retry policies; expose them via wrapper classes.
3. **Custom sinks**
   - Add new methods (e.g., `publishToSerial`) and wire them into your agent without modifying existing callers.
   - Keep the `toJson` helper as the canonical serialization path to avoid schema drift.

---

## Dependencies

- [nlohmann/json](https://github.com/nlohmann/json/tree/e00484f8668dda6cfaf9887ec141c27c341e7de5)
- [`CppMiniDB`](/CppMiniDB)
- C++ Standard Library — `<vector>`, `<fstream>`, and `<iostream>` for storage and IO.

All dependencies are vendored; no additional package manager steps are required.

---

## Contact
Questions, feedback, ideas?

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)

