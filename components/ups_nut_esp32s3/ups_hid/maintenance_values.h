#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

namespace esphome { namespace ups_hid {

inline std::string maintenance_date(int year, int month, int day) {
  if (year < 1980 || year > 2107 || month < 1 || month > 12) return {};
  static constexpr int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int limit = days[month - 1];
  if (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ++limit;
  if (day < 1 || day > limit) return {};
  char result[11];
  snprintf(result, sizeof(result), "%04d-%02d-%02d", year, month, day);
  return result;
}

inline std::string sbs_manufacturer_date(uint32_t value) {
  if (value == 0) return "not set";
  if (value > 0xFFFF) return {};
  return maintenance_date(1980 + (value >> 9), (value >> 5) & 15, value & 31);
}

inline std::string apc_battery_date(uint32_t value) {
  if (value == 0) return "not set";
  if (value > 0xFFFFFF) return {};
  for (unsigned shift = 0; shift < 24; shift += 4)
    if (((value >> shift) & 15) > 9) return {};
  auto bcd = [](unsigned value) { return int((value >> 4) * 10 + (value & 15)); };
  int year = bcd(value & 255);
  year += year >= 70 ? 1900 : 2000;
  return maintenance_date(year, bcd(value >> 16), bcd((value >> 8) & 255));
}

inline bool encode_apc_battery_date(const std::string &date, uint32_t &value) {
  if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
  for (unsigned i = 0; i < date.size(); ++i)
    if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return false;
  int year = (date[0]-'0')*1000 + (date[1]-'0')*100 + (date[2]-'0')*10 + date[3]-'0';
  int month = (date[5]-'0')*10 + date[6]-'0';
  int day = (date[8]-'0')*10 + date[9]-'0';
  if (year < 1980 || year > 2069 || maintenance_date(year,month,day) != date) return false;
  auto bcd = [](unsigned number) { return (number/10 << 4) | (number%10); };
  value = (bcd(month) << 16) | (bcd(day) << 8) | bcd(year%100);
  return true;
}

inline std::string hid_self_test_result(uint32_t value) {
  switch (value) {
    case 1: return "Done and passed";
    case 2: return "Done and warning";
    case 3: return "Done and error";
    case 4: return "Aborted";
    case 5: return "In progress";
    case 6: return "No test initiated";
    case 7: return "Test scheduled";
    default: return "Unknown (" + std::to_string(value) + ")";
  }
}

}}
