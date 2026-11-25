# NUT server: S3 maintenance variant

Embedded TCP 3493 server, used with the project-local `ups_hid` component.
The [S3 YAML](../../../ups-nut-esp32s3.yaml) sets `read_only: true`, default UPS
name `racktest` and authentication off. Runtime ESPHome entities provide a
persistent, validated UPS name and an authentication switch. Enabling the switch
uses credentials supplied at build time; it does not enable NUT writes.

`SET VAR`, `INSTCMD` and `FSD` are rejected in read-only mode; `LIST CMD` is empty.
Read variables include `battery.date`, `battery.mfr.date` and `ups.test.result`.
Battery-date writes use a separate ESPHome action/text entity and USB HID path.
TCP commands are buffered by line to handle fragmented and pipelined requests.

See [setup](../../../docs/esp32-s3-apc-cs350.md),
[comparison](../../../docs/p4-vs-s3.md) and [tests](../../../docs/testing-s3.md).
This isolated copy does not change the P4 server or components shared by other
ESPHome devices.
