# SecureEdgePlatformRunner

SecureEdgePlatformRunner is the entry-point executable for the SecureEdgePlatform. It wires together the `SecureEdgePlatformController` and the interactive platform shell, providing an operator-friendly CLI to boot, start, stop, and inspect the platform during development or demos.

---

## Table of Contents

1. [Highlights](#highlights)
2. [Architecture](#architecture)
3. [Build](#build)
4. [Run](#run)
5. [Command Reference](#command-reference)
6. [Integration Points](#integration-points)
7. [Troubleshooting](#troubleshooting)
8. [Project Layout](#project-layout)
9. [Contact](#contact)

---

## Highlights

- **Single binary launcher** – wraps the controller and exposes a ready-to-use shell without extra wiring.
- **Graceful lifecycle management** – the shell routes `start`/`stop` commands through the controller, ensuring the gateway loop exits cleanly.
- **Secure boot awareness** – boot commands trigger the simulated secure boot flow before telemetry begins.
- **Sensor management bridge** – provides a hand-off into the sensor shell (`sensor::EdgeShell`) for detailed sensor operations.
- **Catch2 integration tests** – shares the same controller APIs validated by `tests/test_full_platform.cpp`.

---

## Architecture

```
┌────────────────────────────┐
│ SecureEdgePlatformRunner   │  (main.cpp)
└──────────────┬─────────────┘
               │ instantiates
               ▼
┌────────────────────────────┐
│ SecureEdgePlatformController│
│ • boot/start/stop          │
│ • EdgeGateway lifecycle    │
└──────────────┬─────────────┘
               │ exposes scheduler
               ▼
┌────────────────────────────┐
│ PlatformShell              │
│ • top-level REPL           │
│ • bridges to sensor shell  │
└──────────────┬─────────────┘
               │ optional
               ▼
┌────────────────────────────┐
│ sensor::EdgeShell          │
└────────────────────────────┘
```

The runner performs three steps:
1. Construct `SecureEdgePlatformController`.
2. Create `PlatformShell` with a reference to the controller so the shell can call controller methods.
3. Invoke `shell.run()` to start the REPL loop.

The process blocks inside the shell until the user exits. All cleanup (including stopping the gateway thread) is delegated to the controller and shell destructors.

---

## Build

The runner is built as part of the top-level CMake project. From the repository root:

```bash
cmake -S . -B build
cmake --build build --target SecureEdgePlatformRunner
```

The resulting executable is emitted to `build/SecureEdgePlatformRunner/SecureEdgePlatformRunner`.

---

## Run

Launch the shell:

```bash
./build/SecureEdgePlatformRunner/SecureEdgePlatformRunner
```

Sample session:

```
[SecureEdgePlatform Shell] Type 'help' for commands.
> start
Selected operation: start
[Controller] Starting SecureEdgePlatform...
[BootPhase] Starting secure boot...
...
> status
[Shell] System running.
> sensors
[Shell] Entering Sensor Management Mode...
EdgeShell> list
TEMP-001
EdgeShell> exit
[Shell] Exited Fault Injection Mode.
> stop
Selected operation: stop
[Controller] Stopping SecureEdgePlatform...
> exit
[Shell] Exiting...
```

The shell remains active until the user types `exit` or sends EOF (`Ctrl+D`). Any active gateway loop is halted automatically on exit.

---

## Command Reference

The runner re-exports the `PlatformShell` commands:

| Command   | Description |
|-----------|-------------|
| `help`    | Print available commands. |
| `boot`    | Execute the secure boot phase once. |
| `start`   | Boot (if needed) and start the gateway run loop in the background. |
| `stop`    | Stop the gateway run loop and join the worker thread. |
| `status`  | Print a terse running-state message (placeholder for richer diagnostics). |
| `flush`   | Reserved for future telemetry flushing features. |
| `sensors` | Suspend the controller and enter the sensor shell in restricted mode. |
| `exit`    | Stop the controller (if running) and terminate the program. |

---

## Integration Points

- **SecureEdgePlatformController** – shared library that encapsulates boot and gateway lifecycle. See `SecureEdgePlatformController/README.md` for deeper details.
- **EdgeGateway** – publishes telemetry to configured channels once started.
- **SensorSimulator** – accessed indirectly via `sensor::EdgeShell` when the `sensors` command is used.
- **Full-platform tests** – `tests/test_full_platform.cpp` exercises the controller/gateway interactions the runner relies on.

---

## Troubleshooting

- **Gateway fails to start** – check logs for `[BootPhase]` errors indicating missing or malformed boot configuration files (`SecureBootSimulator/data/boot_config.json`).
- **Run loop does not stop** – ensure `stop` is invoked before exiting; if the shell appears hung, the gateway may be waiting for `stopLoop()`. The shell’s `exit` command calls `stop()` automatically.
- **Configuration updates ignored** – the gateway loads configuration on each `start()` call. Use `stop` followed by `start` after editing `EdgeGateway/data/gateway_config.json`.

---

## Project Layout

```
SecureEdgePlatformRunner/
├── CMakeLists.txt
└── src/
    └── main.cpp
```

---

## Contact

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
