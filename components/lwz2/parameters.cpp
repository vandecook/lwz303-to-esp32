#include "lwz2.h"
#include <algorithm>

namespace esphome::lwz2 {

const ParameterRule *LWZ2Component::parameter_rule_(uint8_t id) const {
  for (const auto &r : PARAMETER_RULES) if (r.id == id) return &r;
  return nullptr;
}
ParameterRule LWZ2Component::parameter_layout_(const ParameterRule &rule) const {
  auto r = rule;
  if (r.command == 0x01 && !uses_206_layout_()) {
    // FHEM 01pxx214 stores P37..42 in bytes, P43..46 in words, P75 in one byte.
    if (r.id >= 37 && r.id <= 42) { r.offset = r.id - 37; r.length = 1; }
    else if (r.id >= 43 && r.id <= 46) r.offset = 6 + (r.id - 43) * 2;
    else if (r.id == 75) { r.offset = 14; r.length = 1; }
    // Last source field: hex position 32, length 2 -> payload length 15.
    r.minimum = r.minimum_214;
  }
  return r;
}
bool LWZ2Component::parameter_supported_(const ParameterRule &r) const {
  return !r.sol_only || device_model_ == "SOL";
}
void LWZ2Component::parameters_invalidate_() {
  for (auto &entry : parameter_numbers_) entry.second->publish_state(NAN);
  for (auto &entry : parameter_sensors_) entry.second->publish_state(NAN);
  std::fill(std::begin(parameter_lengths_), std::end(parameter_lengths_), 0);
  if (parameters_valid_) parameters_valid_->publish_state(false);
}
void LWZ2Component::parameters_parse_(uint8_t command, const uint8_t *data, size_t len) {
  parameter_lengths_[command] = len;
  for (const auto &base : PARAMETER_RULES) {
    const auto r = parameter_layout_(base);
    if (r.command != command) continue;
    const bool supported = parameter_supported_(r);
    // Gaps preserve the last observation; model-inapplicable fields are never presented as data.
    if (supported && len < size_t(r.offset + r.length)) continue;
    uint16_t raw = 0;
    if (supported) for (uint8_t i=0; i<r.length; i++) raw = (raw << 8) | data[r.offset+i];
    const float value = !supported ? NAN : (r.is_signed ? float(int16_t(raw)) : float(raw)) / r.divisor;
    auto n = parameter_numbers_.find(r.id);
    if (n != parameter_numbers_.end()) n->second->publish_state(value);
    auto s = parameter_sensors_.find(r.id);
    if (s != parameter_sensors_.end()) s->second->publish_state(value);
  }
}
void LWZ2Component::parameters_publish_() {
  if (!parameters_enabled_) return;
  unsigned ready=0, total=0;
  for (const auto &base : PARAMETER_RULES) {
    const auto r = parameter_layout_(base);
    if (!parameter_supported_(r)) continue;
    total++;
    if (block_fresh_(r.command) && parameter_lengths_[r.command] >= size_t(r.offset+r.length)) ready++;
  }
  if (parameters_valid_) parameters_valid_->publish_state(ready == total && total != 0);
  if (parameters_status_) parameters_status_->publish_state(std::to_string(ready)+"/"+std::to_string(total)+
      " Parameter frisch; P86: kein belegter 2.x-Zugriff" +
      (device_model_ == "unknown" ? "; Modell unbekannt, SOL-Felder ausgeblendet" : ""));
}
bool LWZ2Component::parameters_schedule_(uint32_t late_ms) {
  if (!parameters_enabled_) return false;
  for (unsigned i=0; i<7; i++) {
    if (PARAMETER_COMMANDS[i] == 0x08 && device_model_ != "SOL") continue;
    if (time_reached_(millis(), parameter_due_[i] + late_ms)) {
      start_transaction_(PARAMETER_COMMANDS[i]); return true;
    }
  }
  return false;
}
void LWZ2Component::parameters_result_(uint8_t command) {
  for (unsigned i=0; i<7; i++) if (command == PARAMETER_COMMANDS[i])
    parameter_due_[i] = millis() + config_interval_ms_;
}
bool LWZ2Component::request_parameter(uint8_t id, float value) {
  const auto *base = parameter_rule_(id);
  if (!parameters_enabled_ || !base || !base->writable || !parameter_supported_(*base) ||
      !writes_enabled_ || write_stage_ != WriteStage::NONE || !std::isfinite(value)) {
    publish_write_status_("rejected: parameter unavailable, disabled, busy or invalid"); return false;
  }
  const auto r = parameter_layout_(*base);
  const float scaled = value * r.divisor;
  if (value < r.lower || value > r.upper || std::fabs(scaled - std::round(scaled)) > 0.001f ||
      scaled < 0 || scaled > (r.length == 1 ? 255 : 65535)) {
    publish_write_status_("rejected: parameter range or step"); return false;
  }
  write_parameter_ = id; write_value_ = value; write_command_ = r.command;
  write_stage_ = WriteStage::READ;
  publish_write_status_("pending: fresh parameter block read"); return true;
}
}  // namespace esphome::lwz2
