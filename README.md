# UPS NUT on ESP32-P4 and ESP32-S3

[Slovenska različica](README_SLO.MD)

ESPHome bridges a USB HID UPS to Ethernet: an embedded Network UPS Tools
(NUT) server on TCP 3493 and the native ESPHome API for Home Assistant.
This repository contains two independently tested combinations. Its original
name is retained; each variant has its own YAML and isolated component directory.

| | P4 baseline | S3 maintenance variant |
|---|---|---|
| Tested UPS | APC Back-UPS RS 650MI, `939.g4` | APC Back-UPS CS 350, `807.q10 .I USB FW:q10` |
| Ethernet | IP101 PHY, native EMAC/RMII | W5500, SPI |
| USB interpretation | Fixed APC report layouts for the tested UPS | HID descriptor: usages, report IDs, bit positions, scaling |
| NUT name | YAML setting | Persistent runtime setting; default `racktest` |
| NUT authentication | Credentials configured in YAML/secrets | Runtime switch; disabled by default |
| NUT writes | Read commands tested; no global read-only policy | `read_only: true`; writes and control commands rejected |
| Battery maintenance | No date writer in this configuration | Read dates/test result; write replacement date through ESPHome |
| Hardware validation | 2026-11-09 | 2026-11-25 |

Both UPS units communicate over **USB HID**. Ethernet hardware depends on the
ESP board; HID report layout depends on the UPS model/firmware. These are separate
differences. The S3 parser has not been validated as a replacement for the P4 code.

## Choose a variant

| | Configuration | Components | Details |
|---|---|---|---|
| P4 / RS 650MI | [ups-nut-esp32p4.yaml](ups-nut-esp32p4.yaml) | [ups_nut_esp32p4](components/ups_nut_esp32p4) | [Original testing](docs/testing.md) |
| S3 / CS 350 | [ups-nut-esp32s3.yaml](ups-nut-esp32s3.yaml) | [ups_nut_esp32s3](components/ups_nut_esp32s3) | [S3 setup and maintenance](docs/esp32-s3-apc-cs350.md) |

- [P4 versus S3: hardware, communication and fixes](docs/p4-vs-s3.md) (Slovenian).
- [S3 validation, measured values and remaining limits](docs/testing-s3.md) (Slovenian).
- [P4 baseline](docs/baseline.md), [changes](docs/changes.md) and [deployment](docs/deployment.md) are historical P4 records.

For S3, copy the YAML and its component directory into your ESPHome configuration
directory. Create `secrets.yaml` from [secrets.example.yaml](secrets.example.yaml)
and replace the placeholders. Secret references must resolve even with NUT
authentication disabled. The optional [S3 runtime wrapper](ups-nut-esp32s3-runtime.yaml)
uses `UPS_NUT_RUNTIME_USERNAME` and `UPS_NUT_RUNTIME_PASSWORD` environment variables;
enabling authentication still requires the runtime switch.

```powershell
uv run --python 3.14 --with esphome==2026.11.1 esphome config ups-nut-esp32s3.yaml
uv run --python 3.14 --with esphome==2026.11.1 esphome compile ups-nut-esp32s3.yaml
```

The S3 firmware exposes `battery.date`, `battery.mfr.date` and `ups.test.result`
as read-only NUT variables. Its ESPHome text entity/action can write the replacement
date to the UPS with readback verification. The live write test used the existing
date; changing it to a different date and retention through UPS power loss have
not been tested. `Last self test` is a **result**, not a timestamp.

The P4 configuration and component code remain unchanged. Build success does not
establish compatibility with other UPS models; generic/CyberPower paths in the
source have not been hardware-tested in this project. No UPS shutdown or self-test
command was executed during either validation.
