# SecureEdgePlatform

SecureEdgePlatform is an end-to-end C++20 edge-computing sandbox that simulates a secure gateway device. It combines a secure boot pipeline, a sensor telemetry scheduler, multi-channel publishing, and an interactive command shell so you can validate edge scenarios before deploying to real hardware.

---

## Table of Contents

1. [Highlights](#highlights)
2. [Architecture](#architecture)
3. [Core Modules](#core-modules)
4. [Prerequisites](#prerequisites)
5. [Build](#build)
6. [Run the Platform](#run-the-platform)
7. [Command Reference](#command-reference)
8. [Configuration & Data](#configuration--data)
9. [Testing](#testing)
10. [Project Layout](#project-layout)
11. [Development Notes](#development-notes)
12. [Roadmap](#roadmap)
13. [Contact](#contact)

---

## Highlights

- **Secure boot simulation** – validates staged boot policies before telemetry starts.
- **Deterministic sensor scheduler** – produces reproducible sample streams with configurable fault injection.
- **Modular gateway** – publishes data to console, file, and agent channels; easy to extend with new transports.
- **Interactive shells** – operator-facing `PlatformShell` plus developer-oriented `sensor::EdgeShell`.
- **Lightweight persistence** – optional MiniDB-backed logging for sensor history analysis.
- **Catch2 coverage** – targeted unit tests and a full-platform integration suite.

---

## Architecture

```
┌─────────────────────────────┐
│ SecureEdgePlatformRunner    │
│ • CLI entry point           │
└──────────────▲──────────────┘
               │ constructs
               │
┌──────────────┴──────────────┐
│ SecureEdgePlatformController│
│ • Secure boot (BootSimulator)│
│ • EdgeGateway lifecycle      │
│ • Thread management          │
└───────┬───────────┬─────────┘
        │           │
        │           │ shares scheduler
        │           ▼
        │   ┌──────────────────────┐
        │   │ PlatformShell        │
        │   │ • Platform commands  │
        │   │ • Bridges to sensors │
        │   └───────────┬──────────┘
        │               │ enters on demand
        │               ▼
        │       ┌──────────────────┐
        │       │ sensor::EdgeShell│
        │       │ • Sensor control │
        │       └──────────────────┘
        │
        ▼
┌──────────────────────────────┐
│ EdgeGateway                  │
│ • SensorScheduler            │
│ • Channel fan-out            │
│ • EdgeAgent integration      │
└──────────────┬───────────────┘
               ▼
┌──────────────────────────────┐
│ EdgeAgent & MiniDB           │
│ • Telemetry buffering        │
│ • Optional persistence       │
└──────────────────────────────┘
```

---

## Core Modules

| Module | Summary |
|--------|---------|
| `SecureEdgePlatformRunner` | Binary entry point that launches the platform shell. |
| `SecureEdgePlatformController` | Orchestrates secure boot, manages the gateway loop, and exposes lifecycle hooks. |
| `SecureBootSimulator` | Supplies staged boot checks with configurable policies. |
| `SensorSimulator` | Hosts sensor definitions, the deterministic scheduler, and an interactive sensor shell. |
| `EdgeGateway` | Loads channel configs, schedules sensors, and publishes telemetry to console, file, and agent channels. |
| `EdgeAgent` | Buffers telemetry rows and supports custom publishers. |
| `CppMiniDB` | Lightweight tabular store for sensor logs and queries. |

Supporting libraries (Catch2, nlohmann/json) live under `third_party/`.

---

## Prerequisites

- CMake ≥ 3.10
- A C++20-capable compiler (GCC 11+, Clang 13+, Apple Clang 14+, MSVC 19.30+)
- Standard build toolchain (Make or Ninja)
- Git (optional, for submodule updates – all third-party code is vendored)

---

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

The default build produces every library, test, and executable target. Build individual components as needed, for example:

```bash
cmake --build build --target SecureEdgePlatformRunner
cmake --build build --target EdgeGatewayRunner
cmake --build build --target test_full_platform
```

---

## Run the Platform

Launch the interactive platform shell:

```bash
./build/SecureEdgePlatformRunner/SecureEdgePlatformRunner
```

Typical session:

```
[SecureEdgePlatform Shell] Type 'help' for commands.
> start
[Controller] Starting SecureEdgePlatform...
[BootPhase] Starting secure boot...
[EdgeGateway] Starting run loop. Press Ctrl+C to exit.
> status
[Shell] System running.
> sensors
[Shell] Entering Sensor Management Mode...
EdgeShell> list
TEMP-001
EdgeShell> exit
[Shell] Exited Fault Injection Mode.
> stop
[Controller] Stopping SecureEdgePlatform...
> exit
[Shell] Exiting...
```

Use `Ctrl+C` or the `stop` command to halt the gateway loop when running outside the shell context.

---

## Command Reference

`PlatformShell` commands available in the runner:

| Command | Purpose |
|---------|---------|
| `help` | List available commands. |
| `boot` | Execute the secure boot sequence without starting the gateway loop. |
| `start` | Boot (if needed) and launch the gateway loop on a background thread. |
| `stop` | Signal the gateway loop to stop and join the worker thread. |
| `status` | Print a basic running-state message. |
| `flush` | Placeholder for future telemetry flush functionality. |
| `sensors` | Pause the controller and enter the sensor shell (`sensor::EdgeShell`). |
| `exit` | Stop the controller (if running) and terminate the process. |

Inside the sensor shell you can manage sensors (`add`, `remove`, `inject`, `tick`, `export`, etc.). See `SensorSimulator/README.md` for the full catalog.

---

## Configuration & Data

- **Gateway configuration** – `EdgeGateway/data/gateway_config.json` enumerates channel backends (`console`, `file`, `agent`). File channel paths are resolved relative to the config file.
- **Boot configuration** – `SecureBootSimulator/data/boot_config.json` defines the staged boot policies consumed by the simulator.
- **MiniDB storage** – Sensor logs (when enabled) are written to `SensorSimulator/data/`.
- **Temporary files** – Integration tests create isolated temp directories via `std::filesystem::temp_directory_path()` and clean them up after execution.

Adjust these files to experiment with new channel configurations, telemetry destinations, or boot policies. Restart the platform (`stop` → `start`) after edits.

---

## Testing

Catch2-based suites cover individual modules and the full integration path:

| Target | Focus |
|--------|-------|
| `test_full_platform` | Exercises gateway channel fan-out, scheduler behaviour, concurrency guards, and controller lifecycle. |
| `EdgeGateway/test_edgegateway` | Validates channel publishing and configuration parsing. |
| `SensorSimulator/test_cli`, `SensorSimulator/test_sensors` | Cover shell workflows and sensor logic. |
| `SecureBootSimulator/tests/...` | Ensure boot stages and configuration loading behave as expected. |

Run everything with CTest:

```bash
ctest --test-dir build --output-on-failure
```

Or target specific suites:

```bash
cmake --build build --target test_full_platform
ctest --test-dir build -R FullPlatform::
```

---

## Project Layout

```
SecureEdgePlatform/
├── CMakeLists.txt
├── README.md
├── CppMiniDB/
├── EdgeAgent/
├── EdgeGateway/
├── SecureBootSimulator/
├── SecureEdgePlatformController/
├── SecureEdgePlatformRunner/
├── SensorSimulator/
├── tests/
└── third_party/
```

Each module includes its own README detailing design choices and usage.

---

## Development Notes

- **Coding standards** – modern C++20, prefer RAII, avoid raw pointers unless ownership is explicit. Logging uses `std::cout`/`std::cerr` for simplicity.
- **Threading** – the controller owns the gateway loop thread. Re-entrancy guards prevent concurrent `runLoop()` invocations.
- **Extensibility** – add new gateway channels by implementing `channel::IGatewayChannel`; register sensors through `SensorScheduler::addScheduledSensor`.
- **Testing discipline** – extend existing Catch2 suites when modifying behaviour. Integration tests live under `tests/test_full_platform.cpp`.
- **Versioning** – the repo is preparing for `v1.0`. Follow semantic versioning for tagged releases.

---

## Roadmap

- Richer platform status reporting (health metrics, channel counters).
- Configurable persistence backends for `EdgeAgent`.
- Hot-reloadable gateway configuration and shell autocompletion.
- Additional secure boot stages (cryptographic attestation, policy enforcement).
- CI integration for automated builds and test runs.

---

## Contact

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
