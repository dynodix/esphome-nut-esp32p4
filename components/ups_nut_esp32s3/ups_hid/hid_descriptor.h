#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <vector>

namespace esphome { namespace ups_hid {

// HID report IDs and byte positions belong to a device's descriptor, not to
// its vendor. In particular, the APC CS and RS families use different layouts.
struct HidField {
  uint32_t usage{};
  std::vector<uint32_t> path;
  uint8_t type{}, report_id{};
  uint32_t offset{}, size{};
  int64_t logical_min{}, logical_max{}, physical_min{}, physical_max{};
  int exponent{};
  uint32_t unit{};

  bool within(uint32_t parent) const {
    for (auto usage : path) if (usage == parent) return true;
    return false;
  }
  bool extract(const std::vector<uint8_t> &report, double &value, bool physical = true) const {
    if (!size || size > 32 || offset + size > report.size() * 8 ||
        (report_id && report[0] != report_id)) return false;
    uint64_t bits = 0;
    for (uint32_t i = 0; i < size; ++i)
      bits |= uint64_t((report[(offset + i) / 8] >> ((offset + i) % 8)) & 1) << i;
    int64_t raw = bits;
    if (logical_min < 0 && (bits & (uint64_t(1) << (size - 1))))
      raw -= int64_t(uint64_t(1) << size);
    if (raw < logical_min || raw > logical_max) return false;
    value = raw;
    if (physical && physical_max != physical_min && logical_max != logical_min)
      value = physical_min + (value - logical_min) *
              double(physical_max - physical_min) / double(logical_max - logical_min);
    return true;
  }
  double scaled(double value, int base_exponent = 0) const {
    return value * std::pow(10.0, exponent - base_exponent);
  }
  bool insert(std::vector<uint8_t> &report, uint32_t value) const {
    if (!size || size > 32 || offset + size > report.size()*8 ||
        (report_id && report[0] != report_id) ||
        uint64_t(value) >= (uint64_t(1) << size) ||
        int64_t(value) < logical_min || int64_t(value) > logical_max) return false;
    for (uint32_t i = 0; i < size; ++i) {
      uint8_t mask = uint8_t(1U << ((offset+i)%8));
      auto &byte = report[(offset+i)/8];
      byte = (byte & ~mask) | (((value >> i) & 1U) ? mask : 0);
    }
    return true;
  }
};

class HidDescriptor {
 public:
  std::vector<HidField> fields;
  std::map<uint16_t, uint32_t> bits;

  bool parse(const std::vector<uint8_t> &descriptor) {
    fields.clear(); bits.clear();
    struct Global {
      uint32_t page{}, size{}, count{}, unit{};
      uint8_t id{};
      int64_t lmin{}, lmax{}, pmin{}, pmax{};
      int exponent{};
    } g;
    std::vector<Global> stack;
    std::vector<uint32_t> usages, path;
    uint32_t usage_min = 0, usage_max = 0;
    auto qualify = [&g](uint32_t value, unsigned length) {
      return length == 4 ? value : (g.page << 16) | value;
    };
    for (size_t pos = 0; pos < descriptor.size();) {
      uint8_t prefix = descriptor[pos++];
      if (prefix == 0xFE) {
        if (pos + 2 > descriptor.size()) return false;
        unsigned length = descriptor[pos]; pos += 2;
        if (pos + length > descriptor.size()) return false;
        pos += length; continue;
      }
      unsigned length = prefix & 3;
      if (length == 3) length = 4;
      if (pos + length > descriptor.size()) return false;
      uint32_t value = 0;
      for (unsigned i = 0; i < length; ++i) value |= uint32_t(descriptor[pos++]) << (8 * i);
      int64_t signed_value = value;
      if (length && (value & (uint32_t(1) << (8 * length - 1))))
        signed_value -= int64_t(uint64_t(1) << (8 * length));
      unsigned type = (prefix >> 2) & 3, tag = prefix >> 4;
      if (type == 1) {
        switch (tag) {
          case 0: g.page = value; break;
          case 1: g.lmin = signed_value; break;
          case 2: g.lmax = g.lmin < 0 ? signed_value : value; break;
          case 3: g.pmin = signed_value; break;
          case 4: g.pmax = g.pmin < 0 ? signed_value : value; break;
          case 5: g.exponent = int(value & 15); if (g.exponent >= 8) g.exponent -= 16; break;
          case 6: g.unit = value; break;
          case 7: g.size = value; break;
          case 8: if (!value || value > 255) return false; g.id = value; break;
          case 9: g.count = value; break;
          case 10: if (stack.size() > 32) return false; stack.push_back(g); break;
          case 11: if (stack.empty()) return false; g = stack.back(); stack.pop_back(); break;
        }
      } else if (type == 2) {
        if (tag == 0) usages.push_back(qualify(value, length));
        else if (tag == 1) usage_min = qualify(value, length);
        else if (tag == 2) usage_max = qualify(value, length);
      } else if (type == 0) {
        if (tag == 10) {
          if (path.size() >= 32) return false;
          path.push_back(usages.empty() ? usage_min : usages.front());
        } else if (tag == 12) {
          if (path.empty()) return false;
          path.pop_back();
        } else if (tag == 8 || tag == 9 || tag == 11) {
          uint8_t report_type = tag == 8 ? 1 : (tag == 9 ? 2 : 3);
          uint16_t key = uint16_t(report_type) << 8 | g.id;
          if (!bits.count(key)) bits[key] = g.id ? 8 : 0;
          if (g.size > 32 || g.count > 1024 || bits[key] + g.size * g.count > 16384) return false;
          for (uint32_t i = 0; i < g.count; ++i) {
            uint32_t usage = 0;
            if (!usages.empty()) usage = usages[i < usages.size() ? i : usages.size()-1];
            else if (usage_min && usage_min + i <= usage_max) usage = usage_min + i;
            if (!(value & 1) && (value & 2) && usage && report_type != 2) {
              if (fields.size() >= 1024) return false;
              fields.push_back({usage, path, report_type, g.id, bits[key], g.size,
                                g.lmin, g.lmax, g.pmin, g.pmax, g.exponent, g.unit});
            }
            bits[key] += g.size;
          }
        }
        usages.clear(); usage_min = usage_max = 0;
      }
    }
    return path.empty() && !fields.empty();
  }
};

}}  // namespace esphome::ups_hid
