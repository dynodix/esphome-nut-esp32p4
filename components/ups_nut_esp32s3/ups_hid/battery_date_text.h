#pragma once
#ifdef USE_TEXT
#include "ups_hid.h"
#include "maintenance_values.h"

namespace esphome { namespace ups_hid {
class UpsBatteryDateText : public text::Text, public Component {
 public:
  void set_parent(UpsHidComponent *parent) { parent_ = parent; }
 protected:
  UpsHidComponent *parent_{nullptr};
  void control(const std::string &date) override {
    uint32_t unused;
    if (!encode_apc_battery_date(date,unused)) {
      ESP_LOGW("ups_hid.maintenance", "Enter a valid date as YYYY-MM-DD (1980-2069)");
      return;
    }
    if (!parent_->set_battery_replacement_date(date))
      ESP_LOGW("ups_hid.maintenance", "Battery-date change could not be confirmed; keeping the last UPS reading");
  }
};
}}
#endif
