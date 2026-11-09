# CODEX TASK: ESP32-P4 USB HID UPS + NUT Server

## Objective

Repair and complete the existing ESPHome device `ups-nut-esp32p4` on my local Home Assistant system.

The goal is to turn an Ethernet-connected ESP32-P4 board into a reliable USB HID UPS monitor and Network UPS Tools (NUT) server compatible with Home Assistant.

Work autonomously through investigation, code changes, validation, compilation, and testing. Do not deploy firmware or perform potentially disruptive UPS operations without my explicit approval.

## Environment

- Codex runs locally on Windows.
- Project directory: `C:\AI\UPS-NUT-ESP32P4`
- Home Assistant runs on Home Assistant Green.
- SSH access should be configured using an SSH key.
- Home Assistant ESPHome configuration directory is typically `/config/esphome`; discover the actual path.
- An independent Debian Linux ESPHome build server may also be available.
- Target device name: `ups-nut-esp32p4`
- Friendly name: `UPS NUT ESP32-P4`
- NUT UPS name: `ups-esp32p4`
- Hardware: ESP32-P4, 16 MB flash, Ethernet IP101 PHY, USB Host.
- Framework: ESP-IDF.

## Phase 1 — Discover the environment and establish SSH access

1. Inspect the current working directory and available SSH configuration.
2. Reuse an existing appropriate SSH identity if available.
3. Look for a configured Home Assistant SSH host.
4. Do not scan the local network or attempt password guessing.
5. If the SSH endpoint is unknown, ask for its hostname/IP, port, and username.
6. Establish the SSH connection and confirm the effective permissions.
7. Locate the ESPHome configuration.
8. Locate `ups-nut-esp32p4.yaml` or its equivalent and any included package files.
9. Identify ESPHome version, execution environment, build tooling, and available resources.
10. Check whether there is an existing local Debian build environment suitable for ESPHome.

Read secrets only when absolutely necessary for validation. Never print credentials or copy real secrets into project logs.

## Phase 2 — Backup and baseline diagnostics

Before modifying anything:

1. Copy the existing device configuration into the local project.
2. Back up any related YAML packages and custom components.
3. Preserve the original secrets references without copying secret values.
4. Record the current git commit or component revision, ESPHome version, and platform versions.
5. Run a baseline YAML validation.
6. Attempt a baseline compilation if feasible.
7. Capture actual errors and categorize them as schema, dependency, platform, compiler, or runtime problems.

Create `docs/baseline.md` with findings.

Do not modify other ESPHome devices.

## Phase 3 — Examine upstream external components

Inspect this GitHub repository:

https://github.com/bullshit/esphome-components

Focus on:

- `components/ups_hid`
- `components/nut_server`

Read their README files, Python validation schemas, C++ headers, sources, dependencies, and ESP-IDF integration code.

Important: the upstream UPS HID implementation primarily targets ESP32-S3. ESP32-P4 compatibility must be established rather than assumed.

Check:

- Hard-coded ESP32-S3 board or CPU assumptions
- Unsupported USB Host APIs
- ESP-IDF version compatibility
- USB HID driver dependencies
- USB enumeration and descriptor parsing
- Interrupt endpoint handling
- Packet size and transfer allocation
- FreeRTOS synchronization and task handling
- USB disconnect/reconnect logic
- NUT server network socket implementation
- Ethernet compatibility
- NUT protocol correctness
- Authentication semantics
- Memory usage, stack allocation, and potential leaks

Compare requirements with current ESPHome and ESP-IDF documentation.

If upstream components are incompatible:

1. Create a local fork or a project-local `components/` directory.
2. Make the minimum changes necessary to support ESP32-P4.
3. Preserve existing ESP32-S3 compatibility wherever practical.
4. Avoid editing upstream cache files directly.
5. Document every change and its technical justification.
6. Do not publish or push anything to GitHub without permission.

Do not replace the original components with an unrelated implementation unless there is a documented technical reason.

## Phase 4 — Repair ESPHome configuration

Start from the existing configuration.

Preserve these hardware settings unless reliable board documentation or testing proves a change is required:

```yaml
esp32:
  board: esp32-p4-evboard
  flash_size: 16MB
  framework:
    type: esp-idf

ethernet:
  type: IP101
  mdc_pin: GPIO31
  mdio_pin: GPIO52
  power_pin: GPIO51
  clk:
    mode: CLK_EXT_IN
    pin: GPIO50
  phy_addr: 1
```

Preserve ESPHome API, OTA, logger, and Ethernet information sensors.

Activate and repair the USB UPS component:

```yaml
ups_hid:
  id: ups_monitor
  protocol: auto
  update_interval: 10s
```

Use only options actually supported by the installed or repaired component schema.

Activate the NUT server:

```yaml
nut_server:
  ups_hid_id: ups_monitor
  ups_name: ${ups_name}
  port: 3493
  username: !secret nut_username
  password: !secret nut_password
```

The example configuration is a target, not an assertion that all options are currently valid.

Use the existing secrets management.

Resolve ESPHome USB Host initialization, ownership, and dependency handling correctly. Do not initialize the USB Host driver twice.

Avoid redundant USB initialization if the custom component already owns the interface.

## Phase 5 — Telemetry

Expose useful UPS data to Home Assistant via ESPHome API, where supported by the hardware and component:

- UPS USB connection status
- Manufacturer
- Model
- Battery charge (%)
- Battery runtime (seconds or minutes)
- UPS load (%)
- Input voltage (V)
- Output voltage (V)
- Battery voltage (V)
- AC mains present
- UPS operating on battery
- Low battery
- UPS fault
- Last successful communication
- Communication health / stale data detection

Avoid creating fake sensor values or treating missing HID fields as zero.

Make all entities use consistent names, units, device classes, and state classes.

Expose the same valid telemetry through the NUT server.

## Phase 6 — Reliability and safety

Implement or verify:

1. USB automatic discovery after boot.
2. Retry when the UPS is connected later.
3. Automatic recovery after USB disconnection.
4. Safe handling of malformed HID reports.
5. USB timeouts without permanent blocking.
6. Ethernet reconnection.
7. NUT client disconnection/reconnection.
8. Multiple NUT clients where supported.
9. Thread-safe sharing of UPS data.
10. No stale telemetry presented as live data.
11. Non-blocking behavior for the ESPHome main loop.
12. Clear debug logging without excessive output.
13. Bounded buffers and safe handling of unexpected client requests.

Security:

- Keep NUT on the trusted LAN.
- No unauthenticated administrative UPS commands.
- Do not enable remote UPS shutdown, load-off, restart, or destructive test actions by default.
- Keep all potentially dangerous commands disabled unless expressly requested.

## Phase 7 — Build and verify

Perform as much validation as possible:

1. YAML validation
2. Dependency validation
3. ESP-IDF compilation for ESP32-P4
4. Static checks on modified C++ code
5. USB HID parser tests using recorded or simulated safe inputs
6. NUT protocol tests using a mock UPS provider
7. NUT TCP message framing and response tests
8. Recovery tests for disconnects and stale telemetry

Use a local isolated ESPHome environment or the dedicated Debian ESPHome builder if appropriate.

Do not modify the system-wide ESPHome installation just to compile this project.

If validation cannot succeed, continue diagnosing and fixing the underlying problem rather than merely commenting out broken features.

Do not report hardware functionality as verified unless it was actually tested on the physical board with a connected UPS.

## Phase 8 — NUT client verification

When actual hardware is available, verify read-only NUT access using a standard NUT client.

Expected target:

`ups-esp32p4@DEVICE_IP:3493`

Test NUT identification and UPS variables, including:

- `ups.status`
- `battery.charge`
- `battery.runtime`
- `ups.load`
- `input.voltage`
- `output.voltage`

Only require values supported by the UPS.

Verify that Home Assistant can connect with its NUT integration.

Never run UPS power-control or shutdown commands during testing.

## Phase 9 — Deliverables

Create these project outputs:

- `ups-nut-esp32p4.yaml` — corrected ESPHome configuration
- `components/` — only if custom C++/Python components need modification
- `docs/baseline.md` — original issues and environment
- `docs/changes.md` — implemented fixes
- `docs/testing.md` — validation and actual test results
- `docs/deployment.md` — safe installation and rollback procedure
- `README.md` — project overview and maintenance instructions

Provide a final status report covering:

- SSH connection status
- ESPHome version
- ESP-IDF version
- Upstream compatibility
- YAML validation: PASS/FAIL
- Firmware compilation: PASS/FAIL
- USB HID tests: PASS/FAIL/NOT TESTED
- NUT protocol tests: PASS/FAIL/NOT TESTED
- Physical UPS tests: PASS/FAIL/NOT TESTED
- Home Assistant integration: PASS/FAIL/NOT TESTED
- Remaining blockers
- Required manual actions

## Change control

You are authorized to inspect, diagnose, edit local project files, and execute non-disruptive tests.

Before modifying the live Home Assistant configuration, show the diff and obtain approval.

Do not flash firmware, run OTA, reboot the ESP32-P4, restart Home Assistant, change production network settings, modify other ESPHome configurations, or send UPS control commands without explicit approval.

Begin with SSH and environment discovery, and then proceed through all safe stages autonomously.
