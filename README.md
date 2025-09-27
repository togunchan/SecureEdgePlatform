# SensorSimulator

SensorSimulator is the edge-focused simulation layer of the SecureEdgePlatform. It emulates temperature and pressure devices, orchestrates their sampling cadence, and exposes an interactive command shell for injecting realistic faults, exporting data, and exercising recovery flows. The module ships with a deterministic scheduler, a lightweight MiniDB-backed logger, and comprehensive Catch2 tests so that you can iterate quickly on new sensor behaviours or CLI workflows.

---

## Table of Contents

1. [Highlights](#highlights)
2. [Architecture](#architecture)
3. [Prerequisites](#prerequisites)
4. [Build & Install](#build--install)
5. [Running the Shell](#running-the-shell)
6. [Command Reference](#command-reference)
7. [Scheduling & Data Logging](#scheduling--data-logging)
8. [Testing](#testing)
9. [Project Layout](#project-layout)
10. [Extending the Module](#extending-the-module)
11. [Troubleshooting](#troubleshooting)
12. [Dependencies](#dependencies)
13. [Contact](#contact)

---

## Highlights

- **Interactive REPL (`EdgeShell`)** &mdash; manage multiple sensors, advance simulation time, and explore fault scenarios in a single terminal session.
- **Deterministic sensor models** &mdash; `SimpleSensor` provides reproducible behaviour with configurable spike, dropout, and stuck faults.
- **Cooperative scheduler** &mdash; `SensorScheduler` keeps per-sensor timing state, issues samples, and coordinates with the logging backend.
- **MiniDB integration** &mdash; optional data persistence with CSV-like tables, JSON export/import, and rich query commands.
- **Comprehensive tests** &mdash; Catch2 suites validate CLI flows (`test_cli`) and sensor internals (`test_sensors`).

At startup, `EdgeShell` registers a default temperature sensor (`TEMP-001`). Additional `TEMP-*` or `PRES-*` sensors can be created dynamically with user-defined sampling periods.

---

## Architecture

```
+--------------------+     +---------------------+     +------------------+     +-------------------+
|     EdgeShell      | --> |   SensorScheduler   | --> |    sensor_core   | --> |     CppMiniDB     |
| Command handling   |     | Virtual timekeeping |     | Sensor interfaces|     | Tabular logging   |
| Fault orchestration|     | Periodic sampling   |     | SimpleSensor impl|     | JSON import/export|
+-------------------+      +---------------------+     +------------------+     +-------------------+
        ^                                                                                           |
        |                                                                                           |
        +---------------------------------------------------------------+                           |
                            MiniDB-backed command responses             |                           |
+-----------------------------------------------------------------------+------------------------+
```

- **Command processing**: `EdgeShell::run()` builds a registry of command objects (see `include/cli/commands`). Each command parses arguments and delegates to shell helpers or the scheduler.
- **Scheduling**: `SensorScheduler` tracks `period_ms` and `next_sample_time_ms` for each sensor. Calling `tick(delta_ms)` advances the global clock, triggers ready sensors, and (if configured) appends rows to MiniDB.
- **Fault modelling**: commands such as `inject`, `reset`, or `remove` manipulate `SimpleSensor` state to simulate transient or persistent anomalies.
- **Persistence**: when a `MiniDB` instance is supplied via `EdgeShell::setDatabase`, logging commands persist readings and fault flags to disk (`./data` by default).

---

## Prerequisites

- CMake 3.10 or newer
- A C++20-capable compiler (GCC ≥ 11, Clang ≥ 13, Apple Clang ≥ 14, or MSVC ≥ 19.30)
- Git (for fetching submodules, if not already present)

All third-party dependencies are vendored with the repository, so no additional package manager is required.

---

## Build & Install

From the repository root (`SecureEdgePlatform/`):

```bash
mkdir -p build
cd build
cmake ..                 # Configure SensorSimulator, CppMiniDB, and tests
cmake --build .          # Build every target
```

Build specific targets as needed:

```bash
cmake --build . --target EdgeShell       # Interactive shell executable
cmake --build . --target test_cli        # CLI integration tests
cmake --build . --target test_sensors    # Sensor behaviour tests
```

Executables are emitted under `build/SensorSimulator/`.

---

## Running the Shell

Launch the interactive shell from the build directory:

```bash
./SensorSimulator/EdgeShell
```

You will see a welcome banner, the command summary, and a prompt (`>`). The shell automatically adds `TEMP-001` at 1 Hz. Try a short session:

```
> list
TEMP-001
> add PRES-101 500
Sensor added: PRES-101
> inject spike TEMP-001 4.2 0.4
Triggered transient spike on TEMP-001 [mag=4.2, sigma=0.4]
> run
Started real-time simulation. Use 'stop' to halt.
> stop
Stopped real-time simulation
> exportlog filename=session.json
```

Type `help` at any time to redisplay the full command overview. Use `exit` to terminate the shell cleanly.

---

## Command Reference

The commands below are case-sensitive. Sensor IDs are normalised to uppercase internally, so `temp-002` and `TEMP-002` map to the same sensor.

### Sensor Lifecycle

| Command | Parameters | Description |
| --- | --- | --- |
| `add` | `<id> [period_ms]` | Create a `TEMP-*` or `PRES-*` sensor and schedule it with the provided period (default 1000 ms). |
| `remove` | `<id>` | Unschedule and erase the sensor, cancelling any future ticks. |
| `list` | – | Display every registered sensor ID. |
| `reset` | `<id>` | Re-seed the sensor and clear active faults. |

### Sampling & Time Control

| Command | Parameters | Description |
| --- | --- | --- |
| `step` | `<id>` | Generate one sample from a specific sensor. |
| `step all` | – | Sample all registered sensors once (advances internal wall-clock by 1 s). |
| `tick` | `<delta_ms>` | Advance virtual time; the scheduler will emit samples for any sensor whose next trigger time is reached. |
| `run` | – | Start the real-time loop (scheduler wakes every second using a condition variable). |
| `stop` | – | Stop the real-time loop and join the worker thread. |

### Fault Injection & Diagnostics

| Command | Parameters | Description |
| --- | --- | --- |
| `inject` | `<spike|stuck|dropout> <id> [args]` | Trigger the requested fault type. `spike` accepts magnitude and sigma, `stuck` takes duration (ms), `dropout` takes duration (ms). |
| `status` | `<id>` | Display currently active faults for the sensor. |
| `plot` | `<id>` | Render a fixed-width ASCII plot of the sensor history. |
| `runplot` / `stopplot` | `<id>` | Stream live ASCII plots in a background thread until stopped. |

### Logging & Persistence

| Command | Parameters | Description |
| --- | --- | --- |
| `logstatus` | `[filters]` | Show MiniDB log rows; supports filters like `last=5` or `sensor=TEMP-001`. |
| `savelog` | – | Persist the in-memory MiniDB table to disk (`./data`). |
| `loadlog` | – | Load previously saved tables back into memory. |
| `clearlog` | – | Remove logs from memory and disk. |
| `exportlog` | `filename=... [source=memory\|disk]` | Export logs to JSON. |
| `importlog` | `filename=... [target=memory\|disk]` | Import JSON logs. |
| `querylog` | `column=<name> op=<operator> value=<...> [source=memory\|disk]` | Run column-based queries (e.g., `querylog column=value op== value=25.0`). |

### Utility

| Command | Parameters | Description |
| --- | --- | --- |
| `help` | – | Print detailed usage hints. |
| `exit` | – | Quit the shell. |

> **Note**: Logging commands require that a `MiniDB` instance has been attached. The default `EdgeShell` binary (see `src/main_shell.cpp`) sets this up automatically using a database named `sensor_simulator_logs`.

---

## Scheduling & Data Logging

- **Time advancement**: `tickTime(delta_ms)` multiplies the requested delta by 25 internally to accelerate simulation runs. Combined with `run`, this allows you to stress-test scenarios quickly.
- **Global time**: `EdgeShell` maintains a `global_time` counter (in milliseconds) for ad-hoc sampling commands like `step`.
- **MiniDB schema**: when a database is set, columns are created for `timestamp_ms`, `sensor_id`, `value`, and `fault_flags`. Custom metadata can be added by modifying the schema in `EdgeShell::run()`.
- **Data export**: `exportlog` emits JSON files compatible with `importlog`. Use these to build dashboards or offline analytics.

---

## Testing

After building, run the full suite from the build directory:

```bash
ctest --output-on-failure
```

- `SensorSimulator/test_cli` &mdash; covers command parsing, sensor lifecycle operations, and run/stop behaviour by scripting `std::cin`.
- `SensorSimulator/test_sensors` &mdash; validates `SimpleSensor` sampling, reset determinism, and scheduling hooks.
- `CppMiniDB/test_minidb` &mdash; optional, ensures the storage backend behaves as expected.

Add Catch2 cases whenever you introduce new commands or extend sensor functionality to maintain coverage.

---

## Project Layout

```text
SensorSimulator/
├── include/
│   ├── cli/               # EdgeShell interface and command definitions
│   ├── scheduler/         # SensorScheduler API
│   └── sensors/           # Sensor specs, fault models, concrete sensors
├── src/
│   ├── cli/               # Command implementations and shell logic
│   ├── scheduler/         # Scheduler implementation
│   └── main_shell.cpp     # Entry point wiring MiniDB and EdgeShell together
├── tests/
│   ├── test_cli.cpp       # CLI integration tests
│   └── test_sensors.cpp   # Sensor behaviour tests
└── CMakeLists.txt         # Build targets and Catch2 discovery
```

---

## Extending the Module

1. **Add a new sensor type**
   - Implement a class deriving from `ISensor` (see `include/sensors`).
   - Update `EdgeShell::addScheduledSensor` to recognise the new prefix and instantiate your sensor.
   - Provide fixtures in `tests/test_sensors.cpp` to validate behaviour.

2. **Introduce a new CLI command**
   - Derive from `cli::ICommand`, implement `name()` and `execute()`.
   - Register the command in `EdgeShell::run()` via `registry_->registerCommand(...)`.
   - Add coverage in `tests/test_cli.cpp` to prevent regressions.

3. **Augment logging**
   - Extend the MiniDB schema and enrich `LogStatus`, `ExportLog`, or related commands to surface the new data.
   - Provide migration helpers if you intend to read older log files.

---

## Troubleshooting

- **Shell appears to hang**: `EdgeShell::run()` blocks on `std::getline`. When scripting commands, redirect `std::cin` (as done in `test_cli.cpp`).
- **Real-time loop keeps running**: Always run `stop` before exiting or resetting tests to ensure the worker thread joins cleanly.
- **Sensor creation rejected**: Only `TEMP-*` and `PRES-*` prefixes are currently recognised. Extend the factory if you need new sensor families.
- **Build errors referencing Catch2 or JSON**: Initialise git submodules or ensure the `third_party/` directory is present; CMake expects those headers locally.
- **Executable not found**: Run the binaries from the build directory (`build/SensorSimulator/EdgeShell`) or adjust your PATH accordingly.

---

## Dependencies

- **Catch2 v3** &mdash; test framework used across SensorSimulator and CppMiniDB.
- **nlohmann/json** &mdash; single-header JSON library leveraged by MiniDB import/export.
- **CppMiniDB** &mdash; lightweight column store bundled as a sibling project for persistence.
- **C++ Standard Library** &mdash; threads, atomics, condition variables, chrono utilities, and algorithm headers form the backbone of scheduling and fault injection logic.

All dependencies are supplied with the repository; no external downloads are required after cloning.

---

## Contact
Questions, feedback, ideas?

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
