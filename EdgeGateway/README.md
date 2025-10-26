# EdgeGateway

Modular edge-data aggregation and publishing pipeline that orchestrates simulated sensor telemetry, configurable channels, and downstream agent integration. The module mirrors a realistic gateway node by scheduling sensors, routing `SensorLogRow` samples, and coordinating with the embedded EdgeAgent for further processing.

## Table of Contents
- [Overview](#overview)
- [Architecture](#architecture)
- [Configuration Example](#configuration-example)
- [Building & Running](#building--running)
- [Sample Output](#sample-output)
- [Testing](#testing)
- [Extensibility](#extensibility)

## Overview
EdgeGateway acts as the central coordinator for the SecureEdgePlatform. It parses a JSON configuration, spins up the requested gateway channels, schedules sensors through `SensorScheduler`, and publishes each sample to every active channel. Typical use cases include validating new channel implementations, exercising EdgeAgent buffering flows, and verifying gateway behaviour under different configuration profiles.

Key traits:
- Modern C++20 codebase built with CMake (>= 3.10).
- Configuration parsing powered by [nlohmann/json](../third_party/json).
- Tight integration with `SensorScheduler`, `MiniDB`, and the `EdgeAgent` telemetry stack.
- Ships with a runnable `EdgeGatewayRunner` executable for standalone command-line runs.

## Architecture
- **EdgeGateway** – entry point that loads configuration, registers sensors, configures the scheduler, and republishes each sample to the configured channels. Exposes `runLoop()` and `stopLoop()` for lifecycle control.
- **SensorScheduler** – ticks registered sensors at fixed periods and emits `SensorLogRow` instances back to the gateway.
- **Gateway Channels** – implementations of `channel::IGatewayChannel`:
  - `ConsoleChannel` – pretty-prints samples as JSON to stdout.
  - `FileChannel` – appends telemetry to a JSON file.
  - `AgentChannel` – forwards rows to the embedded `EdgeAgent`.
- **EdgeAgent** – buffers incoming rows and can flush them to console or file via `TelemetryPublisher`.

```
┌────────────────────────┐        ┌────────────────────┐
│  gateway_config.json   │ ─────▶ │  EdgeGateway       │
└────────────────────────┘        │ • sensor schedule  │
                                  │ • channel fan-out  │
                                  └──────┬─────────────┘
                                         │ publish(row)
                                         ▼
                         ┌───────────────────────────────┐
                         │ channel::IGatewayChannel      │◀───┐
                         │ • Console / File / Agent      │    │
                         └─────────────┬─────────────────┘    │
                                       │                      │
                                       ▼                      │
                         ┌───────────────────────────────┐    │
                         │ SensorScheduler               │    │
                         │ • periodic tick               │    │
                         └─────────────┬─────────────────┘    │
                                       │                      │
                                       ▼                      │
                         ┌───────────────────────────────┐    │
                         │ Sensor implementations        │────┘
                         └───────────────────────────────┘
```

## Configuration Example
The default JSON configuration lives at `EdgeGateway/data/gateway_config.json`:

```json
{
  "channels": [
    { "type": "console" },
    { "type": "file", "path": "logs/output.json" },
    { "type": "agent" }
  ]
}
```

Tips:
- File-channel paths are resolved relative to the gateway working directory.
- Channel `type` must match a concrete `IGatewayChannel` implementation.
- When `EdgeGateway::start` is called without an explicit path, it resolves the default configuration using the module’s source location.

## Building & Running
Requirements: a C++20 compiler, CMake 3.10+, and a standard build toolchain (Make/Ninja).

From the repository root:

```bash
cmake -S . -B build
cmake --build build --target EdgeGatewayRunner
```

Run the standalone gateway with the bundled configuration:

```bash
./build/EdgeGateway/EdgeGatewayRunner
```

To run the full SecureEdgePlatform stack:

```bash
./build/SecureEdgePlatformRunner/SecureEdgePlatformRunner start
```

Stop the loop with `Ctrl+C`; the controller invokes `EdgeGateway::stopLoop()` so the worker thread exits cleanly.

## Sample Output
A typical run with the default sensor profile produces output similar to:

```
[EdgeGateway] Starting run loop. Press Ctrl+C to exit.
[Tick @ 1000]  Sensor TEMP-001 → value: 25
[ConsoleChannel] Publishing row:
{
    "faultType": [],
    "sensorId": "TEMP-001",
    "timestamp": 1000,
    "value": 25.0
}
```

After issuing a stop (either via `Ctrl+C` or `SecureEdgePlatformController::stop()`), the gateway shuts down gracefully:

```
[Controller] Stopping SecureEdgePlatform...
[EdgeGateway] Stop requested. Waiting for loop to exit...
[EdgeGateway] Run loop stopped.
[Runner] Stopped cleanly.
```

## Testing
Unit tests live under `EdgeGateway/tests` and use Catch2:
- `tests/test_gateway.cpp` – verifies that samples are delivered to every configured channel.

Build and run the dedicated test binary:

```bash
cmake --build build --target test_edgegateway
./build/EdgeGateway/test_edgegateway
```

Alternatively, leverage CTest:

```bash
ctest --test-dir build/EdgeGateway -R edgegateway
```

## Extensibility
- Implement new channels by subclassing `channel::IGatewayChannel` and registering them via configuration.
- Add sensors with `SensorScheduler::addScheduledSensor` and tune sampling periods per sensor.
- Extend the EdgeAgent pipeline by customising `TelemetryPublisher` or adding additional flush targets.
- Enhance `EdgeGateway::start` to support dynamic configuration reloads, validation rules, or environment-specific defaults.

---

## Contact
Questions, feedback, or ideas?

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
