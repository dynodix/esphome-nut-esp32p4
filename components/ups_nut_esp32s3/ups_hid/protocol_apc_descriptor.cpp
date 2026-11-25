#include "protocol_apc.h"
#include "maintenance_values.h"
#include "esphome/core/log.h"
#include <map>
#include <set>

namespace esphome { namespace ups_hid {

static bool monitor_usage(uint32_t usage) {
  switch (usage) {
    case 0x850066: case 0x850068: case 0x850029: case 0x85002A:
    case 0x85008C: case 0x8500D0: case 0x850044: case 0x850045:
    case 0x850042: case 0x850043: case 0x85004B:
    case 0x840030: case 0x840032: case 0x840035: case 0x840040:
    case 0x840053: case 0x840054: case 0x840062: case 0x840065:
    case 0x840069:
    case 0x850085: case 0xFF860016: case 0x840058: return true;
    default: return false;
  }
}

bool ApcHidProtocol::set_battery_replacement_date(const std::string &date) {
  uint32_t encoded;
  if (!descriptor_ready_ || !encode_apc_battery_date(date, encoded)) return false;
  for (const auto &field : descriptor_.fields) {
    if (field.type != 3 || field.usage != 0xFF860016 || !field.within(0x840012)) continue;
    std::vector<uint8_t> report(256);
    size_t length = report.size();
    if (parent_->hid_get_report(3, field.report_id, report.data(), &length, 1000) != ESP_OK) return false;
    report.resize(length);
    // Preserve every other field in this Feature report, including its report ID.
    if (!field.insert(report, encoded)) return false;
    if (parent_->hid_set_report(3, field.report_id, report.data(), report.size(), 1000) != ESP_OK) {
      ESP_LOGW("ups_hid.maintenance", "UPS rejected the battery-date write");
      return false;
    }
    for (unsigned attempt = 0; attempt < 2; ++attempt) {
      vTaskDelay(pdMS_TO_TICKS(100));
      std::vector<uint8_t> response(256);
      length = response.size();
      if (parent_->hid_get_report(3, field.report_id, response.data(), &length, 1000) != ESP_OK) continue;
      response.resize(length);
      double actual;
      if (field.extract(response,actual,false) && uint32_t(actual) == encoded) {
        ESP_LOGI("ups_hid.maintenance", "Battery replacement date confirmed by UPS: %s", date.c_str());
        return true;
      }
    }
    ESP_LOGW("ups_hid.maintenance", "Battery-date write sent, but the UPS did not confirm the requested date");
    return false;
  }
  ESP_LOGW("ups_hid.maintenance", "UPS does not advertise an APC battery replacement-date field");
  return false;
}

bool ApcHidProtocol::read_descriptor_data(UpsData &data) {
  std::map<uint16_t, std::vector<uint8_t>> reports;
  std::set<uint16_t> attempted;
  std::set<uint32_t> decoded;
  bool ac_known = false, discharge_known = false;
  bool success = false;
  // Prefer Feature reports: older APC units do not implement GET_REPORT(Input).
  for (uint8_t report_type : {uint8_t(3), uint8_t(1)}) {
    for (const auto &field : descriptor_.fields) {
      if (field.type != report_type || !monitor_usage(field.usage)) continue;
      // Collection context distinguishes input/output voltage and duplicate usages.
      uint32_t identity = field.usage;
      if (field.within(0x84001A)) identity ^= 0x10000000;
      else if (field.within(0x84001C)) identity ^= 0x20000000;
      else if (field.within(0x840012)) identity ^= 0x40000000;
      else if (field.within(0x840024)) identity ^= 0x80000000;
      if (decoded.count(identity)) continue;
      uint16_t key = uint16_t(field.type) << 8 | field.report_id;
      if (!attempted.count(key)) {
        attempted.insert(key);
        // Back-UPS CS units can return more bytes than their report declares.
        // As with NUT's max-report quirk, request a full report-sized buffer.
        std::vector<uint8_t> buffer(256);
        size_t size = buffer.size();
        if (parent_->hid_get_report(field.type, field.report_id, buffer.data(), &size, 1000) == ESP_OK) {
          buffer.resize(size);
          reports.emplace(key, std::move(buffer));
        }
      }
      auto report = reports.find(key);
      if (!diagnostics_logged_ && field.type == 3 &&
          (field.usage == 0x840030 || field.usage == 0x840040)) {
        std::string raw;
        if (report != reports.end()) {
          for (size_t i = 0; i < report->second.size() && i < 8; ++i) {
            char byte[4]; snprintf(byte, sizeof(byte), "%02X ", report->second[i]); raw += byte;
          }
        }
        ESP_LOGI("ups_hid.apc_descriptor",
                 "Voltage usage=%08X input=%d output=%d report=%02X offset=%u size=%u unit=%08X exponent=%d logical=%lld..%lld physical=%lld..%lld raw=%s",
                 unsigned(field.usage), field.within(0x84001A), field.within(0x84001C), field.report_id,
                 unsigned(field.offset), unsigned(field.size), unsigned(field.unit), field.exponent,
                 (long long)field.logical_min, (long long)field.logical_max,
                 (long long)field.physical_min, (long long)field.physical_max, raw.c_str());
      }
      double value;
      bool maintenance = field.usage == 0x850085 || field.usage == 0xFF860016 || field.usage == 0x840058;
      if (report == reports.end() || !field.extract(report->second, value, !maintenance)) continue;
      decoded.insert(identity);
      const bool input = field.within(0x84001A), output = field.within(0x84001C);
      switch (field.usage) {
        case 0x850085: {
          std::string date = sbs_manufacturer_date(static_cast<uint32_t>(value));
          if (field.within(0x840012)) data.battery.mfr_date = date;
          else if (!field.within(0x84001A) && !field.within(0x84001C)) data.device.mfr_date = date;
          break;
        }
        case 0xFF860016: {
          std::string date = apc_battery_date(static_cast<uint32_t>(value));
          if (field.within(0x840012)) data.battery.date = date;
          else if (field.within(0x840024) && data.battery.mfr_date.empty()) data.battery.mfr_date = date;
          break;
        }
        case 0x840058:
          if (field.within(0x840012)) data.test.ups_test_result = hid_self_test_result(static_cast<uint32_t>(value));
          break;
        case 0x850066:
          if (value >= 0 && value <= 100) { data.battery.level = value; success = true; }
          break;
        case 0x850068: data.battery.runtime_minutes = field.scaled(value) / 60; success = true; break;
        case 0x850029: data.battery.charge_low = value; break;
        case 0x85002A: data.battery.runtime_low = field.scaled(value) / 60; break;
        case 0x85008C: data.battery.charge_warning = value; break;
        case 0x8500D0: data.power.ac_present = value != 0; ac_known = true; success = true; break;
        case 0x850045: data.power.discharging = value != 0; discharge_known = true; success = true; break;
        case 0x850044: data.power.charging = value != 0; break;
        case 0x850042: case 0x850043: case 0x840069: data.power.low_battery |= value != 0; break;
        case 0x85004B: data.power.replace_battery = value != 0; break;
        case 0x840062: data.power.internal_failure = value != 0; break;
        case 0x840065: data.power.overload = value != 0; break;
        case 0x840030: case 0x840040: {
          // PDC voltage units are expressed in the HID centimetre/gram system;
          // exponent seven represents volts. Never interpret an untyped field.
          if (field.unit != 0x00F0D121) break;
          float volts = field.scaled(value, 7);
          if (volts < 0 || volts > 500) break;
          bool nominal = field.usage == 0x840040;
          if (input) {
            (nominal ? data.power.input_voltage_nominal : data.power.input_voltage) = volts;
          } else if (output) {
            (nominal ? data.power.output_voltage_nominal : data.power.output_voltage) = volts;
          } else if (field.within(0x840024) || field.within(0x840012)) {
            (nominal ? data.battery.voltage_nominal : data.battery.voltage) = volts;
          }
          success = true;
          break;
        }
        case 0x840032: if (input) data.power.frequency = field.scaled(value); break;
        case 0x840035: if (value >= 0 && value <= 100) data.power.load_percent = value; break;
        case 0x840053: data.power.input_transfer_low = field.scaled(value, 7); break;
        case 0x840054: data.power.input_transfer_high = field.scaled(value, 7); break;
      }
    }
  }
  data.power.status_valid = ac_known || discharge_known;
  if (!ac_known && discharge_known) data.power.ac_present = !data.power.discharging;
  if (!discharge_known && ac_known) data.power.discharging = !data.power.ac_present;
  if (data.power.status_valid) {
    data.power.status = data.power.discharging ? "OB" : "OL";
    if (data.power.low_battery || data.battery.is_low()) data.power.status += " LB";
    if (data.power.charging) data.power.status += " CHRG";
    if (data.power.replace_battery) data.power.status += " RB";
    if (data.power.overload) data.power.status += " OVER";
    if (data.power.internal_failure) data.power.status += " ALARM";
  }
  // Device-provided USB indices also vary between APC model families.
  auto read_string = [this](uint8_t kind, std::string &destination) {
    uint8_t index = parent_->get_device_string_index(kind);
    if (index) parent_->get_string_descriptor(index, destination);
  };
  read_string(1, data.device.manufacturer);
  read_string(2, data.device.model);
  read_string(3, data.device.serial_number);
  size_t firmware = data.device.model.find(" FW:");
  if (firmware != std::string::npos) {
    data.device.firmware_version = data.device.model.substr(firmware + 4);
    data.device.model.resize(firmware);
  }
  diagnostics_logged_ = true;
  return success;
}

}}  // namespace esphome::ups_hid
