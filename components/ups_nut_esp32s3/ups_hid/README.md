# UPS HID: S3 / APC CS 350 variant

Project-local USB HID host components tested with ESP32-S3 and APC Back-UPS
CS 350 (`807.q10 .I USB FW:q10`). The APC path reads the device HID descriptor
instead of assuming the P4/RS 650MI report layout. Generic/CyberPower code remains
in the source but has not been hardware-tested here.

Use the complete [S3 YAML](../../../ups-nut-esp32s3.yaml) with this component
folder. `ups_hid` owns ESP-IDF USB Host; do not add a second `usb_host:` stack.

The configured read sensors include battery charge/runtime/voltage, output
voltage, load and status. Missing input voltage remains unavailable. Maintenance
text sensors expose replacement date, manufacturing date and the self-test
**result**, not a timestamp. The text platform and public setter support a
calendar-validated APC replacement-date write using read-modify-write and USB
readback. The live test wrote the same existing date; a different date and
persistence across UPS power loss remain untested.

See [S3 setup and maintenance](../../../docs/esp32-s3-apc-cs350.md),
[P4/S3 comparison](../../../docs/p4-vs-s3.md) and
[validation](../../../docs/testing-s3.md). Legacy control helpers in the source
are not enabled by the example or hardware-validated; no UPS shutdown or self-test
command was executed. The P4 component directory is maintained separately.
