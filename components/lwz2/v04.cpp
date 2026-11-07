#include "lwz2.h"
#include <cstdio>
#include <cstring>
#include <limits>

namespace esphome::lwz2 {

void LWZ2Component::v04_number_(const std::string &key, float value) {
  auto it = v04_sensors_.find(key);
  if (it != v04_sensors_.end()) it->second->publish_state(value);
}
void LWZ2Component::v04_publish_text_(const std::string &key, const std::string &value) {
  auto it = v04_text_.find(key);
  if (it != v04_text_.end()) it->second->publish_state(value);
}
bool LWZ2Component::v04_save_() {
  persistence_ok_ = v04_preference_.save(&saved_) && global_preferences->sync();
  v04_publish_text_("persistence_status", persistence_ok_ ? "gespeichert" : "Speicherfehler");
  return persistence_ok_;
}
void LWZ2Component::clock_mode_publish_() {
  if (clock_mode_select_)
    clock_mode_select_->publish_state(saved_.clock_mode == 0 ? "Winterzeit" :
                                     saved_.clock_mode == 1 ? "Sommerzeit" : "Unbekannt");
}
void LWZ2Component::v04_setup_() {
  if (!v04_enabled_) return;
  v04_invalidate_();
  for (auto *n : fan_stages_) if (n) n->publish_state(NAN);
  v04_preference_ = global_preferences->make_preference<V04Saved>(preferences_key_);
  V04Saved loaded{};
  if (v04_preference_.load(&loaded) && loaded.magic == saved_.magic &&
      loaded.clock_mode >= -1 && loaded.clock_mode <= 1) saved_ = loaded;
  for (auto &r : saved_.fans) { r.begin[23] = 0; r.end[23] = 0; }
  saved_.reset_reason[63] = 0;
  clock_mode_publish_();
  v04_history_();
  v04_due_[0] = millis() + 6500;
  v04_due_[1] = millis() + 7000;
  v04_publish_text_("persistence_status", "bereit");
}
void LWZ2Component::set_device_model(const std::string &v) {
  if (v != "i" && v != "SOL" && v != "unknown") return;
  device_model_ = v;
  // Invalidate translated stages; raw codes remain observable.
  v04_publish_text_("hc_stage_text", "warte auf F2");
  v04_publish_text_("dhw_stage_text", "warte auf F2");
  v04_due_[0] = millis();
}
void LWZ2Component::set_fan_speed_source(const std::string &v) {
  if (v != "sGlobal" && v != "sFan") return;
  if (speed_source_ == v) return;
  speed_source_ = v;
  speed_seen_ = false;
  zero_pending_[0] = zero_pending_[1] = false;
}
void LWZ2Component::v04_invalidate_() {
  speed_seen_ = program_seen_ = settings_seen_ = override_seen_ = clock_seen_ = false;
  zero_pending_[0] = zero_pending_[1] = false;
  for (const char *k : {"hc_stage", "dhw_stage", "current_fan_stage"}) v04_number_(k, NAN);
  for (const char *k : {"hc_stage_text", "dhw_stage_text"}) v04_publish_text_(k, "unbekannt");
  // A decoder change/reconnect never clears a latched fault.
}
static bool v04_fresh(bool seen, uint32_t at, uint32_t max_age) {
  return seen && millis() - at <= max_age;
}
int LWZ2Component::current_fan_stage_() const {
  if (!v04_fresh(program_seen_, program_ms_, diagnostic_max_age_ms_) ||
      !v04_fresh(settings_seen_, settings_ms_, diagnostic_max_age_ms_) ||
      !v04_fresh(override_seen_, override_ms_, diagnostic_max_age_ms_)) return -1;
  // Temporary overrides can exist at the LWZ even though v0.4 does not write them.
  if (override_remaining_ > 0) return override_stage_;
  if (override_remaining_ != 0) return -1;
  if (program_mode_ == 0) return fan_manual_;
  if (program_mode_ != 1) return -1;
  if (fan_program_ == 1) return fan_day_;
  if (fan_program_ == 2) return fan_night_;
  if (fan_program_ == 3) return fan_standby_;
  return -1;
}

void LWZ2Component::v04_parse_(uint8_t cmd, const uint8_t *d, size_t len) {
  if (!v04_enabled_) return;
  const uint32_t now = millis();
  if (cmd == 0xF2) {
    for (unsigned i = 0; i < 2; i++) {
      const std::string key = i ? "dhw_stage" : "hc_stage";
      const int code = len > 3 ? d[2 + i] : -1;
      v04_number_(key, code < 0 ? NAN : code);
      std::string label = code < 0 ? "unbekannt" : "Rohcode " + std::to_string(code);
      // Only SOL has a source-backed code table. Do not derive i by subtracting one.
      if (device_model_ == "SOL" && code >= 0) {
        if (code == 0) label = "Aus";
        else if (code == 1) label = "Solar angefordert";
        else if (code == 2) label = "Verdichter angefordert";
        else if (i && code == 3) label = "Maximale freigegebene Nacherwaermung angefordert";
        else if (!i && code >= 3 && code <= 5) label = "Nacherwaermstufe " + std::to_string(code - 2) + " angefordert";
      }
      v04_publish_text_(key + "_text", label);
    }
  }
  if (cmd == 0x17) {
    if (!v04_fresh(settings_seen_, settings_ms_, diagnostic_max_age_ms_))
      zero_pending_[0] = zero_pending_[1] = false;
    auto stage = [&](size_t pos) { return len > pos && d[pos] <= 3 ? int(d[pos]) : -1; };
    fan_day_ = stage(12); fan_night_ = stage(13); fan_standby_ = stage(14); fan_manual_ = stage(19);
    for (unsigned i = 0; i < 2; i++) if (fan_stages_[i])
      fan_stages_[i]->publish_state((i ? fan_night_ : fan_day_) < 0 ? NAN : float(i ? fan_night_ : fan_day_));
    settings_seen_ = true; settings_ms_ = now;
  }
  if (cmd == 0xEE) {
    if (!v04_fresh(program_seen_, program_ms_, diagnostic_max_age_ms_))
      zero_pending_[0] = zero_pending_[1] = false;
    program_mode_ = len ? d[0] : -1;
    fan_program_ = len > 5 ? d[5] : -1;
    program_seen_ = true; program_ms_ = now;
  }
  if (cmd == 0xF6) {
    if (!v04_fresh(override_seen_, override_ms_, diagnostic_max_age_ms_))
      zero_pending_[0] = zero_pending_[1] = false;
    override_stage_ = len > 13 && d[13] <= 3 ? d[13] : -1;
    override_remaining_ = len >= 18 ? be16u_(d + 16) : -1;
    override_seen_ = true; override_ms_ = now;
    if (len >= 4) {
      char raw[16]; std::snprintf(raw, sizeof(raw), "%02X%02X%02X%02X", d[0], d[1], d[2], d[3]);
      v04_publish_text_("lwz_error_memory", std::string("LWZ Fehlerhistorie roh: ") + raw);
    }
  }
  if ((cmd == 0xFB && speed_source_ == "sGlobal") || (cmd == 0xE8 && speed_source_ == "sFan")) {
    // Check the previous observation before replacing its timestamp; gaps reset the timer.
    if (!v04_fresh(speed_seen_, speed_ms_, diagnostic_max_age_ms_))
      zero_pending_[0] = zero_pending_[1] = false;
    if (cmd == 0xFB) {
      v04_speed_[0] = len > 26 ? d[26] : NAN; v04_speed_[1] = len > 27 ? d[27] : NAN;
    } else {
      v04_speed_[0] = len >= 14 ? be16u_(d + 12) : NAN;
      v04_speed_[1] = len >= 12 ? be16u_(d + 10) : NAN;
    }
    speed_seen_ = true; speed_ms_ = now;
    v04_tick_(true);  // Threshold is confirmed only on a fresh speed observation.
  }
  if (cmd == 0xFC) {
    clock_stamp_ = "unbekannt";
    if (len >= 9 && d[2] < 24 && d[3] < 60 && d[4] < 60 && d[5] <= 99 &&
        d[7] >= 1 && d[7] <= 12 && d[8] >= 1 && d[8] <= 31) {
      char stamp[24];
      std::snprintf(stamp, sizeof(stamp), "%04u-%02u-%02u %02u:%02u:%02u", 2000U + d[5], d[7], d[8], d[2], d[3], d[4]);
      clock_stamp_ = stamp;
    }
    clock_seen_ = clock_stamp_ != "unbekannt"; clock_ms_ = now;
  }
  if (cmd == 0x17 || cmd == 0xEE || cmd == 0xF6) v04_tick_();
}

void LWZ2Component::v04_history_() {
  v04_publish_text_("fault_reset_reason", saved_.reset_reason[0] ? saved_.reset_reason : "keine");
  for (unsigned i = 0; i < 2; i++) {
    const std::string key = i ? "supply" : "exhaust";
    const auto &r = saved_.fans[i];
    auto it = v04_binary_.find(key + "_fault");
    if (it != v04_binary_.end()) it->second->publish_state(r.active);
    v04_number_(key + "_fault_count", r.count);
    v04_publish_text_(key + "_fault_begin", r.begin[0] ? r.begin : "unbekannt");
    v04_publish_text_(key + "_fault_end", r.end[0] ? r.end : "unbekannt");
  }
}
void LWZ2Component::v04_tick_(bool speed_observation) {
  if (!v04_enabled_) return;
  const uint32_t now = millis();
  const int stage = current_fan_stage_();
  const bool data_ok = v04_fresh(speed_seen_, speed_ms_, diagnostic_max_age_ms_) && stage >= 0;
  v04_number_("current_fan_stage", stage < 0 ? NAN : float(stage));
  for (unsigned i = 0; i < 2; i++) {
    std::string state = "Unbekannt";
    const bool valid = data_ok && std::isfinite(v04_speed_[i]) && v04_speed_[i] >= 0;
    if (!valid || stage == 0 || v04_speed_[i] > 0) zero_pending_[i] = false;
    if (valid) {
      if (stage == 0) state = "Aus";
      else if (v04_speed_[i] > 0) state = "Laeuft";
      else {
        if (!zero_pending_[i]) { zero_pending_[i] = true; zero_since_[i] = now; }
        state = "Anlauf/Pruefung";
        if (!saved_.fans[i].active && now - zero_since_[i] >= 120000 && speed_observation) {
          auto &r = saved_.fans[i]; r.active = true;
          if (r.count < std::numeric_limits<uint32_t>::max()) r.count++;
          const auto stamp = v04_fresh(clock_seen_, clock_ms_, diagnostic_max_age_ms_) ? clock_stamp_ : "unbekannt";
          std::snprintf(r.begin, sizeof(r.begin), "%s", stamp.c_str());
          r.end[0] = 0;
          v04_save_(); v04_history_();
        }
      }
    }
    const std::string key = i ? "supply" : "exhaust";
    v04_publish_text_(key + "_data_status", state);
    v04_publish_text_(key + "_status", saved_.fans[i].active ? "Stoerung gespeichert" : state);
  }
  if (block_stale_[0xF2] || now - block_success_[0xF2] > diagnostic_max_age_ms_) {
    v04_number_("hc_stage", NAN); v04_number_("dhw_stage", NAN);
    v04_publish_text_("hc_stage_text", "unbekannt"); v04_publish_text_("dhw_stage_text", "unbekannt");
  }
}
void LWZ2Component::reset_fan_faults() {
  if (!v04_enabled_) return;
  const auto stamp = v04_fresh(clock_seen_, clock_ms_, diagnostic_max_age_ms_) ? clock_stamp_ : "unbekannt";
  for (auto &r : saved_.fans) if (r.active) {
    r.active = false; std::snprintf(r.end, sizeof(r.end), "%s", stamp.c_str());
  }
  // Require new observations after the HA acknowledgement.
  std::snprintf(saved_.reset_reason, sizeof(saved_.reset_reason), "%s", "HA/manuelle Ruecksetzung nach LWZ-Neustart");
  v04_invalidate_(); v04_save_(); v04_history_();
  v04_tick_();
}
bool LWZ2Component::request_fan_stage(uint8_t index, float value) {
  if (!v04_enabled_ || !writes_enabled_ || write_stage_ != WriteStage::NONE || index > 1 ||
      !std::isfinite(value) || value < 0 || value > 3 || value != std::floor(value)) {
    publish_write_status_("rejected: fan stage requires permission, idle writer and integer 0..3"); return false;
  }
  write_command_ = 0x17; write_index_ = 4 + index; write_value_ = value; clock_shift_ = 0;
  write_stage_ = WriteStage::READ; publish_write_status_("pending: fresh fan settings read"); return true;
}
bool LWZ2Component::request_clock_mode(const std::string &mode) {
  if (!v04_enabled_ || (mode != "Winterzeit" && mode != "Sommerzeit")) return false;
  const int target = mode == "Sommerzeit" ? 1 : 0;
  if (write_stage_ != WriteStage::NONE) { publish_write_status_("rejected: writer busy"); return false; }
  if (saved_.clock_mode == target) { clock_mode_publish_(); return true; }
  if (saved_.clock_mode < 0) {
    publish_write_status_("rejected: clock mode uncertain; confirm actual mode without shifting first"); return false;
  }
  if (!request_clock_shift(target ? 1 : -1)) return false;
  pending_clock_mode_ = target;
  return true;
}
void LWZ2Component::confirm_clock_mode(bool summer) {
  if (!v04_enabled_ || write_stage_ != WriteStage::NONE || saved_.clock_mode >= 0) return;
  // Explicit recovery of uncertain local metadata only. Never writes the device clock.
  saved_.clock_mode = summer ? 1 : 0;
  if (!v04_save_()) {
    saved_.clock_mode = -1; clock_mode_publish_();
    publish_write_status_("FAILED: cannot persist clock confirmation"); return;
  }
  clock_mode_publish_();
  publish_write_status_("clock mode metadata confirmed; device clock unchanged");
}
}  // namespace esphome::lwz2
