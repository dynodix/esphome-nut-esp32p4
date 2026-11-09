# UPS NUT ESP32-P4

[Slovenska različica](README_SLO.MD)

An ESPHome project for an ESP32-P4 with IP101 Ethernet, a USB HID UPS client,
and a Network UPS Tools (NUT) server on TCP port 3493. The project uses patched,
project-local copies of the `ups_hid` and `nut_server` components, leaving shared
components used by other ESPHome devices untouched.

## Contents

- `ups-nut-esp32p4.yaml` — the device configuration candidate.
- `ups-nut-esp32p4-runtime.yaml` — a local deployment wrapper that supplies NUT
  credentials through process environment variables without storing them in the
  project.
- `components/ups_nut_esp32p4/` — isolated, patched ESPHome components.
- `backup/ha-current/` — an unchanged local backup of the Home Assistant state.
- `upstream/esphome-components/` — the inspected copy of the upstream repository.
- `docs/` — baseline findings, implemented changes, testing results, and safe
  deployment instructions.

The real `nut_username` and `nut_password` values remain in Home Assistant's
`secrets.yaml`. The local `secrets.yaml` is excluded from Git and must never be
committed.

## Local validation

```powershell
.\.venv\Scripts\esphome.exe config .\ups-nut-esp32p4.yaml
.\.venv\Scripts\esphome.exe compile .\ups-nut-esp32p4.yaml
```

Successful compilation alone does not verify USB enumeration, HID data, or NUT
operation with every UPS model. An APC Back-UPS RS 650MI was successfully tested
on 2026-11-09 through USB HID, the ESPHome API, and read-only NUT commands. No UPS
control command was executed.
