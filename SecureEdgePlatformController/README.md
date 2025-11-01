# SecureEdgePlatformController

The SecureEdgePlatformController is the orchestration layer of the SecureEdgePlatform. It coordinates secure boot validation, configures and supervises the `EdgeGateway` runtime loop, and exposes a user-facing shell for lifecycle control and sensor management. The controller is designed to mimic the operational responsibilities of a production edge supervisor while remaining developer-friendly for demos, testing, and experimentation.

---

## Table of Contents

1. [Highlights](#highlights)
2. [Architecture](#architecture)
3. [Lifecycle](#lifecycle)
4. [Platform Shell](#platform-shell)
5. [Boot Phase](#boot-phase)
6. [Building](#building)
7. [Running](#running)
8. [Testing](#testing)
9. [Extending the Controller](#extending-the-controller)
10. [Project Layout](#project-layout)
11. [Contact](#contact)

---

## Highlights

- **Secure boot simulation** – drives the `SecureBootSimulator` stages before enabling telemetry execution.
- **Gateway lifecycle management** – owns an `EdgeGateway` instance, manages its background loop thread, and routes stop requests.
- **Shell integration** – supplies a top-level CLI (`PlatformShell`) that bridges platform commands with the lower-level sensor management shell.
- **Thread-safe start/stop** – ensures only one gateway loop runs at a time and joins worker threads during shutdown.
- **Test hooks** – reuses the same controller entry point employed by the `SecureEdgePlatformRunner` and full-platform integration tests.

---

## Architecture

```
┌────────────────────────────┐
│ SecureEdgePlatformRunner   │  (main.cpp)
└──────────────┬─────────────┘
               │ constructs
               ▼
┌────────────────────────────┐        ┌──────────────────────────┐
│ SecureEdgePlatformController│──────▶│ SecureBootSimulator      │
│ • start / stop              │       │ • BootStage pipeline     │
│ • bootPhase()               │       └──────────────────────────┘
│ • runLoop() thread          │
│ • getGateway()              │               publishes rows
└──────┬───────────────┬──────┘               ▼
       │ owns          │ exposes      ┌──────────────────────────┐
       │               │              │ EdgeGateway              │
       │               │              │ • runLoop / stopLoop     │
       │               │              │ • SensorScheduler        │
       │         ┌─────▼─────┐        │ • Agent/File/Console ch. │
       │         │ Platform  │        └──────────────┬───────────┘
       │         │ Shell     │                      │
       │         │ (REPL)    │◀─────────────────────┘
       │         └───────────┘ uses scheduler
       │
       ▼
┌───────────────┐
│ EdgeAgent     │
└───────────────┘
```

The controller sits between the platform runner and the telemetry subsystems:

- `start()` invokes a secure boot flow and, on success, launches the `EdgeGateway::runLoop()` in a dedicated thread.
- `stop()` signals the gateway loop to halt (`EdgeGateway::stopLoop()`), waits for the loop thread, and resets the running flag.
- `PlatformShell` offers a REPL that issues controller commands, forwards to the gateway scheduler, or transitions into the sensor management shell (`sensor::EdgeShell`).

---

## Lifecycle

1. **Create** – `SecureEdgePlatformController controller;`
2. **Start** – `controller.start();`
   - Guarded by an atomic flag; repeated calls print `[Controller] Already running.`.
   - Executes `bootPhase()` before spawning the gateway thread.
3. **Run Loop** – `runLoop()` (private) loads the gateway configuration (defaulting to `EdgeGateway/data/gateway_config.json`) and invokes `EdgeGateway::runLoop()`, which blocks until `stopLoop()` is called.
4. **Stop** – `controller.stop();`
   - Signals the gateway to stop its loop and joins the worker thread if it is still active.
5. **Destroy** – the destructor invokes `stop()` to guarantee clean shutdowns.

The controller also exposes `getGateway()` overloads so tests or tooling can inspect the underlying scheduler or channel state.

---

## Platform Shell

`PlatformShell` is a lightweight CLI that wraps the controller. It is instantiated in `SecureEdgePlatformRunner/src/main.cpp` and is responsible for registering the following commands:

| Command  | Description |
|----------|-------------|
| `boot`   | Runs the secure boot phase once. Reports failure if any boot stage fails. |
| `start`  | Invokes `SecureEdgePlatformController::start()`. Safe to call repeatedly; subsequent calls no-op with a log message. |
| `stop`   | Invokes `SecureEdgePlatformController::stop()`, ensuring the gateway loop halts. |
| `status` | Prints a simple `[Shell] System running.` message (placeholder for richer status reporting). |
| `flush`  | Placeholder for future log flushing support. |
| `exit`   | Calls `stop()`, prints an exit message, and terminates the process. |
| `sensors`| Stops the controller, enters the `sensor::EdgeShell` in restricted mode, and returns to the platform shell when the nested session exits. |

`PlatformShell` shares the gateway's scheduler with the sensor shell (`edgeShell_.setScheduler(...)`), enabling consistent sensor state between shells.

---

## Boot Phase

Before the gateway loop can run, `bootPhase()` performs a simulated secure boot:

1. Resolves the boot configuration at `SecureBootSimulator/data/boot_config.json`.
2. Loads the configuration via `secureboot::BootConfig::loadFromFile`.
3. Constructs a `secureboot::BootSimulator` and enqueues three staged operations:
   - **Preflight Checks**
   - **Firmware Authentication**
   - **Subsystem Bring-up**
4. Executes the simulator and verifies `simulator.wasSuccessful()`; failure short-circuits `start()` with an error message.

The stage list can be extended to model additional policies or diagnostics.

---

## Building

The `platform_controller` library is part of the top-level CMake project and depends on the gateway, agent, secure boot, sensor, and MiniDB targets.

From the repository root:

```bash
cmake -S . -B build
cmake --build build --target platform_controller
```

The controller is automatically linked into the `SecureEdgePlatformRunner` executable:

```bash
cmake --build build --target SecureEdgePlatformRunner
```

---

## Running

A typical end-to-end session uses the platform runner (which embeds the controller and shell):

```bash
./build/SecureEdgePlatformRunner/SecureEdgePlatformRunner
```

At the prompt:

```
[SecureEdgePlatform Shell] Type 'help' for commands.
> start
Selected operation: start
[Controller] Starting SecureEdgePlatform...
[BootPhase] Starting secure boot...
...
> status
[Shell] System running.
> stop
Selected operation: stop
[Controller] Stopping SecureEdgePlatform...
```

To exit, use `exit` or `Ctrl+D`. The controller ensures the gateway thread stops before the process terminates.

---

## Testing

Higher-level integration coverage for the controller and gateway lives under `tests/test_full_platform.cpp`. Build and run the suite with:

```bash
cmake --build build --target test_full_platform
ctest --test-dir build -R FullPlatform::
```

These scenarios validate multi-channel fan-out, scheduler behaviour, concurrency guards, and configuration edge cases while exercising the controller lifecycle.

---

## Extending the Controller

- **Richer status reporting** – extend `status` to query gateway metrics or sensor health.
- **Graceful reloads** – add a command that reloads gateway configuration without restarting the process.
- **Telemetry flushing** – implement the `flush` stub to persist `EdgeAgent` buffers or rotate log files.
- **Observability hooks** – expose Prometheus-style metrics or structured logging as needed.
- **Policy enforcement** – incorporate additional secure boot stages or integrate with a cryptographic attestation module.

Contributions should include integration or unit tests (Catch2) that demonstrate the new behaviour within the full platform context.

---

## Project Layout

```
SecureEdgePlatformController/
├── CMakeLists.txt
├── include/
│   ├── SecureEdgePlatformController.hpp
│   └── shell/PlatformShell.hpp
└── src/
    ├── SecureEdgePlatformController.cpp
    └── shell/PlatformShell.cpp
```

---

## Contact

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
