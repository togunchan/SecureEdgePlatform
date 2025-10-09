# SecureBootSimulator

Modular secure-boot pipeline simulator for embedded-style platforms. It mirrors a realistic firmware bring-up by validating configuration, checking firmware integrity, executing staged initialization, and surfacing deterministic failures for diagnostics.

## Table of Contents
- [Overview](#overview)
- [Architecture](#architecture)
- [Configuration Example](#configuration-example)
- [Building & Running](#building--running)
- [Sample Output](#sample-output)
- [Testing](#testing)
- [Extensibility](#extensibility)

## Overview
SecureBootSimulator lets firmware and platform teams prototype secure boot logic without hardware in the loop. The module is structured around four core components—`BootConfig`, `BootStage`, `BootSimulator`, and `SignatureVerifier`—so individual responsibilities stay focused and easy to extend. Typical workflows include validating new boot sequences, stress-testing failure handling, and experimenting with firmware verification policies.

Key traits:
- Written in modern C++20 with CMake (>= 3.10).
- Uses [nlohmann/json](../third_party/json) for configuration parsing.
- Leverages [Catch2](../third_party/Catch2) for unit testing.
- Ships with a runnable `SecureBootRunner` executable for CLI driven simulations.

## Architecture
- **BootConfig** – parses a JSON configuration file, validates required fields, normalizes paths, and exposes typed accessors (firmware path, boot mode, entry point, expected hash).
- **BootStage** – encapsulates a single boot step with ordering, success/failure tracking, timings, and optional failure callbacks. Exceptions of type `StageFailure` propagate consistent error codes.
- **BootSimulator** – assembles the configured stages, verifies the firmware image, enforces execution order, and records the boot result.
- **SignatureVerifier** – computes the firmware digest (currently SHA-256) and performs hash comparison.

```
┌─────────────┐    loads    ┌────────────────────────────┐
│ good_config │ ──────────▶ │ BootConfig                 │
└─────────────┘             │ • validates JSON           │
                            │ • resolves firmware path   │
                            └─────────┬──────────────────┘
                                      │
                                      ▼
                            ┌────────────────────────────┐
                            │ BootSimulator              │
                            │ • verifies firmware hash   │◀────────┐
                            │ • executes ordered stages  │         │
                            └─────────┬──────────────────┘         │
                                      │                            │
                                      ▼                            │
                            ┌────────────────────────────┐         │
                            │ BootStage                  │         │
                            │ • worker callbacks         │         │
                            │ • duration + error codes   │         │
                            └────────────────────────────┘         │
                                      ▲                            │
                                      └────── SignatureVerifier ───┘
```

## Configuration Example
`tests/data/good_config.json` ships with the repository and illustrates the minimal configuration accepted by `BootConfig`:

```json
{
  "firmware_path": "fake_firmware.bin",
  "expected_sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
  "boot_mode": "NORMAL",
  "entry_point": "0x0"
}
```

Hints:
- Relative firmware paths are resolved against the configuration file location.
- `expected_sha256` must contain exactly 64 hexadecimal characters.
- `entry_point` accepts decimal or hexadecimal notation (`"0x..."`).

## Building & Running
Requirements: a C++20 compiler, CMake 3.10+, and a standard build toolchain (Make/Ninja).

Configure and build from the repository root:

```bash
cmake -S . -B build
cmake --build build --target SecureBootRunner
```

Run the simulator with the bundled configuration:

```bash
./build/SecureBootSimulator/SecureBootRunner SecureBootSimulator/tests/data/good_config.json
```

Provide a custom configuration to explore alternative boot flows, signature mismatches, or stage-specific failures.

## Sample Output
Successful runs produce output similar to the following (voltage and timings vary because of simulated noise):

```
[SecureBoot] Starting boot process...

[SecureBoot] Verifying firmware signature...
[SecureBoot] ➤ Executing stage: VerifyPowerRails
[Stage] Checking power rails... measured 3264 mV
[SecureBoot] Stage 'VerifyPowerRails' completed in 154 ms

[SecureBoot] ➤ Executing stage: LoadFirmware
[Stage] LoadFirmware: done.
[SecureBoot] Stage 'LoadFirmware' completed in 0 ms

[SecureBoot] ➤ Executing stage: JumpToEntry
[Stage] JumpToEntry: done.
[SecureBoot] Stage 'JumpToEntry' completed in 0 ms

[SecureBoot] Boot process completed successfully.

[SecureBoot] Boot simulation completed successfully.
```

## Testing
Unit tests live under `SecureBootSimulator/tests` and rely on Catch2:
- `tests/test_boot_config.cpp` – configuration parsing and validation.
- `tests/test_boot_stage.cpp` – stage execution, timing, and error handling.
- `tests/test_bootsimulator.cpp` – full boot flow orchestration.

After generating the build directory, execute the suite:

```bash
ctest --test-dir build/SecureBootSimulator -R secureboot
```

Alternatively, run `cmake --build build --target test_secureboot` to compile the dedicated test binary.

## Extensibility
- Register new boot steps by constructing `BootStage` instances with unique names, ordering indices, and simulation lambdas, then invoke `BootSimulator::addStage`.
- Use `StageFailure` to surface deterministic error codes; missing handlers or unexpected exceptions fall back to `BootStage::kMissingHandlerErrorCode` and `BootStage::kUnhandledExceptionErrorCode`.
- Attach `onFail` callbacks for cleanup, telemetry, or retry hooks.
- Extend `SignatureVerifier` to support additional algorithms by enriching the `HashMethod` enum and providing the associated hash implementation.

These extension points make it straightforward to model sophisticated boot chains, integrate platform-specific diagnostics, or plug the simulator into CI pipelines for regression testing.


---

## Contact
Questions, feedback, ideas?

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Murat_Toğunçhan_Düzgün-blue.svg)](https://www.linkedin.com/in/togunchan/)
[![GitHub](https://img.shields.io/badge/GitHub-togunchan-black.svg)](https://github.com/togunchan)
