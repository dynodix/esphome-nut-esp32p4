#include "components/ups_nut_esp32s3/ups_hid/maintenance_values.h"
#include "components/ups_nut_esp32s3/ups_hid/hid_descriptor.h"
#include <cassert>
#include <iostream>
using namespace esphome::ups_hid;

int main() {
  assert(apc_battery_date(0x112525) == "2025-11-25");
  assert(apc_battery_date(0x022924) == "2024-02-29");
  assert(apc_battery_date(0x022923).empty());
  assert(apc_battery_date(0x043025) == "2025-04-30");
  assert(apc_battery_date(0x043125).empty());
  assert(apc_battery_date(0x1A2525).empty());
  assert(apc_battery_date(0x1000000).empty());
  assert(apc_battery_date(0) == "not set");
  assert(sbs_manufacturer_date((2015-1980)*512+12*32+4) == "2015-12-04");
  assert(sbs_manufacturer_date((2000-1980)*512+2*32+29) == "2000-02-29");
  assert(sbs_manufacturer_date((2100-1980)*512+2*32+29).empty());
  assert(sbs_manufacturer_date(0) == "not set");
  assert(sbs_manufacturer_date(0x10000).empty());
  assert(hid_self_test_result(1) == "Done and passed");
  assert(hid_self_test_result(3) == "Done and error");
  assert(hid_self_test_result(4) == "Aborted");
  assert(hid_self_test_result(5) == "In progress");
  assert(hid_self_test_result(6) == "No test initiated");
  assert(hid_self_test_result(7) == "Test scheduled");
  assert(hid_self_test_result(99) == "Unknown (99)");
  uint32_t encoded;
  assert(encode_apc_battery_date("2025-11-25",encoded) && encoded == 0x112525);
  assert(encode_apc_battery_date("2024-02-29",encoded));
  assert(!encode_apc_battery_date("2023-02-29",encoded));
  assert(!encode_apc_battery_date("2070-01-01",encoded));
  assert(!encode_apc_battery_date("2025-1-01",encoded));
  assert(!encode_apc_battery_date("2025-01-0a",encoded));
  HidField field;
  field.report_id = 0x46; field.offset = 8; field.size = 24;
  field.logical_max = 0xFFFFFF; field.physical_max = 100;
  double value;
  assert(field.extract({0x46,0x25,0x25,0x11}, value, false));
  assert(apc_battery_date(value) == "2025-11-25");
  assert(!field.extract({0x46,0x25},value,false));
  std::vector<uint8_t> update = {0x46,0x00,0x00,0x00,0xA5};
  assert(field.insert(update,0x112525));
  assert(update == std::vector<uint8_t>({0x46,0x25,0x25,0x11,0xA5}));
  std::vector<uint8_t> short_report = {0x46,0x00};
  assert(!field.insert(short_report,0x112525));
  assert(!field.insert(update,0x1000000));
  // A packed field must preserve unrelated bits on both sides.
  field.offset=11; field.size=13; field.logical_max=8191;
  update = {0x46,0xA5,0x5A,0xC3};
  assert(field.insert(update,1234));
  assert((update[1]&7) == (0xA5&7) && update[3] == 0xC3);
  assert(field.extract(update,value,false) && value == 1234);
  std::cout << "Battery dates, invalid encodings, raw metadata and self-test results: PASS\n";
}
