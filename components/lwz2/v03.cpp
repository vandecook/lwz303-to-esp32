#include "lwz2.h"
#include <algorithm>
#include <cmath>

namespace esphome::lwz2 {
static constexpr uint8_t EXTRA_COMMANDS[]{0xF3, 0xEE, 0xFC, 0x17};

void LWZ2Component::set_writes_enabled(bool enabled) {
  writes_enabled_ = enabled;
  if (write_stage_ == WriteStage::NONE)
    publish_write_status_(enabled ? "ready: manual writes enabled; hardware validation pending" : "disabled: read only");
}

void LWZ2Component::set_hc1_layout(const std::string &profile) {
  if (write_stage_ != WriteStage::NONE) return;
  if (profile == "2.06") hc1_layout_ = FirmwareProfile::FW_206;
  else if (profile == "2.14") hc1_layout_ = FirmwareProfile::FW_214;
  else if (profile == "2.14j") hc1_layout_ = FirmwareProfile::FW_214J;
  else return;
  parse_selected_(0xF4, nullptr, 0, true);
  next_f4_ms_ = millis();
}

const SelectedField *LWZ2Component::selected_rule_(const std::string &key, uint8_t command, bool &belongs) const {
  const auto profile = command == 0xF4 ? hc1_layout_ : firmware_;
  const auto mask = 1U << static_cast<unsigned>(profile);
  belongs = false;
  for (const auto &r : SELECTED_FIELDS) {
    if (r.command == command && key == r.key) {
      belongs = true;
      if (r.profiles & mask) return &r;
    }
  }
  return nullptr;
}

float LWZ2Component::selected_value_(const SelectedField &r, const uint8_t *data) const {
  const size_t offset = (r.position - 4) / 2;
  uint32_t raw = 0;
  if (r.length == 1) raw = (data[offset] >> (r.position % 2 ? 0 : 4)) & 15;
  else for (size_t i = 0; i < r.length / 2; i++) raw = (raw << 8) | data[offset + i];
  if (r.mode == FieldMode::BIT) return (raw >> r.bit) & 1;
  if (r.mode == FieldMode::SIGNED) return static_cast<int16_t>(raw) / r.divisor;
  return raw / r.divisor;
}

void LWZ2Component::parse_selected_(uint8_t command, const uint8_t *data, size_t len, bool invalidate) {
  auto valid = [&](const SelectedField *r) {
    return !invalidate && r && data && len * 2 + 4 >= r->position + r->length;
  };
  for (const auto &entry : selected_sensors_) {
    bool belongs; const auto *r = selected_rule_(entry.first, command, belongs);
    if (belongs) entry.second->publish_state(valid(r) ? selected_value_(*r, data) : NAN);
  }
  for (const auto &entry : selected_binary_) {
    bool belongs; const auto *r = selected_rule_(entry.first, command, belongs);
    if (!belongs) continue;
    if (valid(r)) entry.second->publish_state(selected_value_(*r, data) != 0);
    else entry.second->invalidate_state();
  }
  for (const auto &entry : selected_text_) {
    bool belongs; const auto *r = selected_rule_(entry.first, command, belongs);
    if (!belongs) continue;
    std::string text = "unavailable";
    if (valid(r)) {
      const int value = static_cast<int>(selected_value_(*r, data));
      text = "unknown(" + std::to_string(value) + ")";
      if (r->mode == FieldMode::PROGRAM && value >= 0 && value <= 1)
        text = value == 0 ? "manual" : "automatic";
      if (r->mode == FieldMode::HCMODE && value >= 1 && value <= 5) {
        static const char *modes[]{"", "normal", "setback", "standby", "restart", "restart"};
        text = modes[value];
      }
      if (r->mode == FieldMode::SEASON && (value == 1 || value == 2)) text = value == 1 ? "winter" : "summer";
      if (r->mode == FieldMode::WEEKDAY && value >= 0 && value <= 6) {
        static const char *days[]{"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
        text = days[value];
      }
    }
    entry.second->publish_state(text);
  }
}

bool LWZ2Component::v03_schedule_() {
  const uint32_t now = millis();
  if (write_stage_ != WriteStage::NONE) {
    if (!writes_enabled_ && write_stage_ != WriteStage::VERIFY) {
      finish_write_("cancelled: write permission disabled");
      return false;
    }
    sending_set_ = write_stage_ == WriteStage::SET;
    start_transaction_(write_command_);
    write_transaction_ = true;
    return true;
  }
  if (!v03_enabled_) return false;
  if (v04_enabled_) for (unsigned i = 0; i < 2; i++) {
    if (time_reached_(now, v04_due_[i])) {
      start_transaction_(i ? 0xF6 : 0xF2);
      return true;
    }
  }
  for (unsigned i = 0; i < 4; i++) {
    if (time_reached_(now, v03_due_[i])) {
      start_transaction_(EXTRA_COMMANDS[i]);
      return true;
    }
  }
  return false;
}

void LWZ2Component::v03_schedule_result_(uint8_t command) {
  for (unsigned i = 0; i < 4; i++) if (command == EXTRA_COMMANDS[i])
    v03_due_[i] = millis() + (v04_enabled_ ? 10000 : command == 0x17 ? 60000 : command == 0xF3 ? status_interval_ms_ : 30000);
  if (v04_enabled_) for (unsigned i = 0; i < 2; i++) if (command == (i ? 0xF6 : 0xF2))
    v04_due_[i] = millis() + 10000;
}

void LWZ2Component::v03_stale_() {
  if (!v03_enabled_) return;
  const uint8_t commands[]{0xFB, 0xF3, 0xF4, 0xE8, 9, 0xEE, 0xFC, 0x17};
  for (auto command : commands) {
    const uint32_t interval = command == 9 ? history_interval_ms_ : command == 0x17 ? 60000 :
        command == 0xF4 ? heating_interval_ms_ : command == 0xE8 ? fan_interval_ms_ :
        (command == 0xFB || command == 0xF3) ? effective_interval() : 30000;
    if (!block_stale_[command] && millis() - block_success_[command] > std::max(stale_after_ms_, interval * 3)) {
      block_stale_[command] = true;
      invalidate_block_(command);
    }
  }
}

void LWZ2Component::invalidate_block_(uint8_t command) {
  parse_selected_(command, nullptr, 0, true);
#define INVALIDATE_SENSOR(name) if (name##_sensor_) name##_sensor_->publish_state(NAN)
#define INVALIDATE_BINARY(name) if (name##_binary_sensor_) name##_binary_sensor_->invalidate_state()
  if (command == 0xFB) {
    INVALIDATE_SENSOR(outside_temperature); INVALIDATE_SENSOR(flow_temperature); INVALIDATE_SENSOR(return_temperature);
    INVALIDATE_SENSOR(hot_gas_temperature); INVALIDATE_SENSOR(dhw_temperature); INVALIDATE_SENSOR(flow_temperature_hc2);
    INVALIDATE_SENSOR(inside_temperature); INVALIDATE_SENSOR(evaporator_temperature); INVALIDATE_SENSOR(condenser_temperature);
    INVALIDATE_SENSOR(supply_fan_power); INVALIDATE_SENSOR(exhaust_fan_power); INVALIDATE_SENSOR(main_fan_power);
    INVALIDATE_SENSOR(supply_fan_speed_percent); INVALIDATE_SENSOR(exhaust_fan_speed_percent); INVALIDATE_SENSOR(main_fan_speed_percent);
    INVALIDATE_BINARY(compressor); INVALIDATE_BINARY(heating_circuit_pump); INVALIDATE_BINARY(dhw_pump);
    INVALIDATE_BINARY(solar_pump); INVALIDATE_BINARY(booster_stage1); INVALIDATE_BINARY(booster_stage2);
    INVALIDATE_BINARY(diverter_valve); INVALIDATE_BINARY(heat_pipe_valve); INVALIDATE_BINARY(heating_active); INVALIDATE_BINARY(dhw_active);
  } else if (command == 0xE8) {
    INVALIDATE_SENSOR(supply_fan_speed); INVALIDATE_SENSOR(exhaust_fan_speed);
    INVALIDATE_SENSOR(supply_fan_airflow_set); INVALIDATE_SENSOR(exhaust_fan_airflow_set);
    INVALIDATE_SENSOR(supply_fan_speed_target); INVALIDATE_SENSOR(exhaust_fan_speed_target);
  } else if (command == 9) {
    INVALIDATE_SENSOR(operating_hours_1); INVALIDATE_SENSOR(operating_hours_2);
    INVALIDATE_SENSOR(heating_hours); INVALIDATE_SENSOR(dhw_hours); INVALIDATE_SENSOR(cooling_hours);
  } else if (command == 5) {
    INVALIDATE_SENSOR(hc1_gradient); INVALIDATE_SENSOR(hc1_low_end);
  } else if (command == 0x17) {
    for (auto *n : setpoints_) if (n) n->publish_state(NAN);
    for (auto *n : fan_stages_) if (n) n->publish_state(NAN);
  }
#undef INVALIDATE_SENSOR
#undef INVALIDATE_BINARY
}

void LWZ2Component::publish_write_status_(const std::string &status) {
  if (write_status_) write_status_->publish_state(status);
}

void LWZ2Component::finish_write_(const std::string &status) {
  if (v04_enabled_ && write_command_ == 0xFC) {
    if (status.rfind("verified:", 0) == 0 && pending_clock_mode_ >= 0) {
      saved_.clock_mode = pending_clock_mode_;
      v04_save_();
    }
    pending_clock_mode_ = -2;
    clock_mode_publish_();
  }
  publish_write_status_(status);
  write_stage_ = WriteStage::NONE;
  sending_set_ = false;
  write_expected_.clear();
}

bool LWZ2Component::request_setpoint(uint8_t index, float value) {
  if (!writes_enabled_ || write_stage_ != WriteStage::NONE || index >= 4 || !std::isfinite(value)) {
    publish_write_status_("rejected: disabled, busy or invalid value"); return false;
  }
  const float lower = index < 2 ? 15.0f : 35.0f, upper = index < 2 ? 25.0f : 55.0f;
  if (value < lower || value > upper || std::fabs(value * 10 - std::round(value * 10)) > 0.001f) {
    publish_write_status_("rejected: outside proposed UI limits / 0.1 C grid"); return false;
  }
  write_command_ = 0x17; write_index_ = index; write_value_ = value; clock_shift_ = 0;
  write_stage_ = WriteStage::READ; publish_write_status_("pending: fresh parameter read"); return true;
}

bool LWZ2Component::request_clock_shift(int hours) {
  if (!writes_enabled_ || write_stage_ != WriteStage::NONE || (hours != 1 && hours != -1) ||
      (clock_request_seen_ && millis() - last_clock_request_ms_ < 30000)) {
    publish_write_status_("rejected: disabled, busy or clock action cooldown"); return false;
  }
  clock_request_seen_ = true; last_clock_request_ms_ = millis(); clock_shift_ = hours;
  pending_clock_mode_ = -2;
  write_command_ = 0xFC; write_stage_ = WriteStage::READ;
  publish_write_status_("pending: fresh device clock read"); return true;
}

void LWZ2Component::write_response_(const std::vector<uint8_t> &frame) {
  if (write_stage_ == WriteStage::NONE) return;
  const std::vector<uint8_t> data(frame.begin() + 4, frame.end());
  if (write_stage_ == WriteStage::SET) {
    sending_set_ = false; write_stage_ = WriteStage::VERIFY;
    publish_write_status_("pending: independent readback"); return;
  }
  if (write_stage_ == WriteStage::READ) {
    write_expected_ = data;
    write_read_ms_ = millis();
    if (write_command_ == 0x17) {
      if (data.size() < 20 || data.size() > 128) { finish_write_("rejected: incomplete parameter block"); return; }
      if (write_index_ < 4) {
        static const uint8_t offsets[]{0, 2, 6, 8};
        const uint16_t raw = static_cast<uint16_t>(std::lround(write_value_ * 10));
        write_expected_[offsets[write_index_]] = raw >> 8;
        write_expected_[offsets[write_index_] + 1] = raw & 255;
      } else {
        write_expected_[12 + write_index_ - 4] = static_cast<uint8_t>(write_value_);
      }
    } else {
      if (data.size() < 9 || data.size() > 128 || data[2] > 23 || data[3] > 59 || data[4] > 59 ||
          data[7] < 1 || data[7] > 12 || data[8] < 1 || data[8] > 31 || (data[1] & 15) > 6) {
        finish_write_("rejected: invalid/incomplete clock"); return;
      }
      const int target = data[2] * 3600 + data[3] * 60 + data[4] + clock_shift_ * 3600;
      // Calendar and weekday rollover are deliberately not guessed. Leave 30 s for the transaction.
      if (target < 30 || target >= 86370) { finish_write_("rejected: date rollover requires verified calendar support"); return; }
      write_expected_[2] = target / 3600;
      // Save uncertainty BEFORE SET. A power loss must never replay a seasonal shift.
      if (v04_enabled_) {
        saved_.clock_mode = -1;
        if (!v04_save_()) { finish_write_("FAILED: cannot persist clock transaction; no SET sent"); return; }
        clock_mode_publish_();
      }
    }
    write_stage_ = WriteStage::SET; publish_write_status_("pending: single targeted write"); return;
  }
  if (write_stage_ == WriteStage::VERIFY) {
    bool match = data == write_expected_;
    if (write_command_ == 0xFC && data.size() == write_expected_.size()) {
      const int actual = data[2] * 3600 + data[3] * 60 + data[4];
      const int target = write_expected_[2] * 3600 + write_expected_[3] * 60 + write_expected_[4];
      const int elapsed = static_cast<int>((millis() - write_read_ms_) / 1000);
      match = data[2] <= 23 && data[3] <= 59 && data[4] <= 59 &&
          std::abs(actual - target - elapsed) <= 5;
      for (size_t i = 0; i < data.size(); i++) if (i < 2 || i > 4) match = match && data[i] == write_expected_[i];
    }
    finish_write_(match ? "verified: target and retained fields match" : "FAILED: readback mismatch; no automatic rewrite");
  }
}

void LWZ2Component::write_failed_(const std::string &reason) {
  if (write_stage_ == WriteStage::SET) {
    // SET may already have reached the controller. Never retransmit it: inspect the result instead.
    sending_set_ = false; write_stage_ = WriteStage::VERIFY;
    publish_write_status_("write response uncertain; checking readback: " + reason);
  } else finish_write_("FAILED: " + reason + "; no automatic rewrite");
}
}  // namespace esphome::lwz2
