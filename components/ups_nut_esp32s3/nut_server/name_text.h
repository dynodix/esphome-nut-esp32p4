#pragma once
#ifdef USE_TEXT
#include "nut_server.h"
#include "esphome/components/text/text.h"
#include "esphome/core/preferences.h"
#include <cstring>

namespace esphome { namespace nut_server {
class NutUpsNameText : public text::Text, public Component {
 public:
  void set_parent(NutServerComponent *parent) { parent_ = parent; }
  void set_initial_value(const std::string &value) { initial_ = value; }
  void set_restore_value(bool value) { restore_ = value; }
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void setup() override {
    std::string value = initial_;
    if (restore_) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      uint32_t key = this->get_preference_hash() ^ 0x4E55544E;
#pragma GCC diagnostic pop
      pref_ = global_preferences->make_preference<StoredName>(key);
      StoredName saved{};
      if (pref_.load(&saved)) {
        saved.value[32] = '\0';
        if (valid(saved.value)) value = saved.value;
      }
    }
    apply(value, false);
  }
 protected:
  struct StoredName { char value[33]{}; };
  NutServerComponent *parent_{nullptr};
  std::string initial_{"ups"};
  bool restore_{true};
  ESPPreferenceObject pref_;
  static bool valid(const std::string &value) {
    if (value.empty() || value.size() > 32) return false;
    for (unsigned char c : value)
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')) return false;
    return true;
  }
  void apply(const std::string &value, bool save) {
    if (!valid(value)) {
      ESP_LOGW("nut_settings", "Invalid UPS name; keeping the current name and saved value");
      return;
    }
    parent_->set_ups_name(value);
    this->publish_state(value);
    if (save && restore_) {
      StoredName stored{};
      memcpy(stored.value, value.data(), value.size());
      pref_.save(&stored);
    }
  }
  void control(const std::string &value) override { apply(value, true); }
};
}}
#endif
