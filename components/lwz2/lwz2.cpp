#include "lwz2.h"

#include <cmath>
#include <cstdio>

namespace esphome::lwz2 {

static const char *const TAG = "lwz2";

void LWZ2Component::setup() {
  ESP_LOGI(TAG, "Starting LWZ 2.x read-only component");
  this->frame_.reserve(128);
  this->startup_ms_ = millis();
  this->setup_complete_ = true;
  this->reset_schedule_();
  if (this->communication_status_text_sensor_ != nullptr)
    this->communication_status_text_sensor_->publish_state("starting");
  if (this->last_error_text_sensor_ != nullptr)
    this->last_error_text_sensor_->publish_state("none");
}

void LWZ2Component::dump_config() {
  ESP_LOGCONFIG(TAG, "LWZ 2.x:");
  const char *fw = this->firmware_ == FirmwareProfile::FW_206 ? "2.06" :
                   this->firmware_ == FirmwareProfile::FW_214 ? "2.14" : "2.14j";
  ESP_LOGCONFIG(TAG, "  Firmware profile: %s", fw);
  ESP_LOGCONFIG(TAG, "  Status (FB) interval: %u ms", this->status_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Heating (F4) interval: %u ms", this->heating_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Fan (E8) interval: %u ms", this->fan_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Fan config (01) interval: %u ms", this->fan_config_interval_ms_);
  ESP_LOGCONFIG(TAG, "  History (09) interval: %u ms", this->history_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Stale timeout: %u ms", this->stale_after_ms_);
}

void LWZ2Component::set_firmware(const std::string &fw) {
  FirmwareProfile next = FirmwareProfile::FW_214J;
  if (fw == "2.06")
    next = FirmwareProfile::FW_206;
  else if (fw == "2.14")
    next = FirmwareProfile::FW_214;
  else if (fw == "2.14j")
    next = FirmwareProfile::FW_214J;
  else {
    ESP_LOGW(TAG, "Unsupported firmware profile '%s'; keeping current profile", fw.c_str());
    return;
  }

  if (next == this->firmware_ && this->last_success_any_ms_ != 0)
    return;

  this->firmware_ = next;
  if (!this->setup_complete_)
    return;
  ESP_LOGI(TAG, "Firmware profile changed to %s", fw.c_str());
  this->state_ = CommState::IDLE;
  this->flush_rx_();
  this->consecutive_errors_ = 0;
  this->last_success_any_ms_ = 0;
  this->stale_published_ = false;
  this->reset_schedule_();
  if (this->communication_status_text_sensor_ != nullptr)
    this->communication_status_text_sensor_->publish_state("starting");
}

void LWZ2Component::loop() {
  this->process_uart_();

  const uint32_t now = millis();
  if ((this->state_ == CommState::WAIT_WAKE_ACK || this->state_ == CommState::WAIT_CMD_ACK_1 ||
       this->state_ == CommState::WAIT_CMD_ACK_2 || this->state_ == CommState::READ_FRAME) &&
      this->time_reached_(now, this->state_deadline_ms_)) {
    this->handle_timeout_();
  }

  if (this->state_ == CommState::RETRY_WAIT && this->time_reached_(now, this->retry_at_ms_))
    this->restart_transaction_();

  this->update_communication_state_();
  this->scheduler_();
}

void LWZ2Component::reset_schedule_() {
  const uint32_t now = millis();
  this->next_fb_ms_ = now + 500;
  this->next_f4_ms_ = now + 1200;
  this->next_e8_ms_ = now + 2000;
  this->next_01_ms_ = now + 3000;
  this->next_09_ms_ = now + 4000;
  this->next_allowed_request_ms_ = now;
}

void LWZ2Component::scheduler_() {
  if (this->state_ != CommState::IDLE)
    return;

  const uint32_t now = millis();
  if (!this->time_reached_(now, this->next_allowed_request_ms_))
    return;

  // FB is the common 2.x source for compressor/pump state, so it gets highest priority.
  if (this->time_reached_(now, this->next_fb_ms_)) {
    this->start_transaction_(0xFB);
    return;
  }
  if (this->time_reached_(now, this->next_f4_ms_)) {
    this->start_transaction_(0xF4);
    return;
  }
  if (this->time_reached_(now, this->next_e8_ms_)) {
    this->start_transaction_(0xE8);
    return;
  }
  if (this->time_reached_(now, this->next_01_ms_)) {
    this->start_transaction_(0x01);
    return;
  }
  if (this->time_reached_(now, this->next_09_ms_)) {
    this->start_transaction_(0x09);
    return;
  }
}

void LWZ2Component::start_transaction_(uint8_t command) {
  this->current_command_ = command;
  this->attempt_ = 1;
  this->restart_transaction_();
}

void LWZ2Component::restart_transaction_() {
  this->flush_rx_();
  this->frame_.clear();
  this->frame_dle_ = false;
  this->write_byte(0x02);
  this->state_ = CommState::WAIT_WAKE_ACK;
  this->state_deadline_ms_ = millis() + ACK_TIMEOUT_MS;
  ESP_LOGV(TAG, "TX wakeup for command 0x%02X (attempt %u/%u)", this->current_command_, this->attempt_, MAX_ATTEMPTS);
}

void LWZ2Component::send_command_frame_() {
  const uint8_t checksum = static_cast<uint8_t>((0x01 + this->current_command_) & 0xFF);
  const uint8_t body[] = {0x01, 0x00, checksum, this->current_command_};

  for (uint8_t b : body) {
    if (b == 0x10) {
      this->write_byte(0x10);
      this->write_byte(0x10);
    } else if (b == 0x2B) {
      // FHEM's encoder/decoder uses the 2B 18 escaping for the protocol stream.
      this->write_byte(0x2B);
      this->write_byte(0x18);
    } else {
      this->write_byte(b);
    }
  }
  this->write_byte(0x10);
  this->write_byte(0x03);

  this->state_ = CommState::WAIT_CMD_ACK_1;
  this->state_deadline_ms_ = millis() + ACK_TIMEOUT_MS;
  ESP_LOGV(TAG, "TX request 0x%02X checksum 0x%02X", this->current_command_, checksum);
}

void LWZ2Component::process_uart_() {
  uint8_t b;
  while (this->available()) {
    if (!this->read_byte(&b))
      break;
    this->handle_byte_(b);
  }
}

void LWZ2Component::handle_byte_(uint8_t b) {
  switch (this->state_) {
    case CommState::WAIT_WAKE_ACK:
      if (b == 0x10) {
        this->send_command_frame_();
      } else if (b == 0x15) {
        this->fail_attempt_("NAK during wakeup");
      }
      break;

    case CommState::WAIT_CMD_ACK_1:
      if (b == 0x10) {
        this->state_ = CommState::WAIT_CMD_ACK_2;
        this->state_deadline_ms_ = millis() + ACK_TIMEOUT_MS;
      } else if (b == 0x02) {
        // FHEM also accepts a lone 0x02 here.
        this->write_byte(0x10);
        this->frame_.clear();
        this->frame_dle_ = false;
        this->state_ = CommState::READ_FRAME;
        this->state_deadline_ms_ = millis() + FRAME_TIMEOUT_MS;
      } else if (b == 0x15) {
        this->fail_attempt_("NAK after command");
      }
      break;

    case CommState::WAIT_CMD_ACK_2:
      if (b == 0x02) {
        this->write_byte(0x10);
        this->frame_.clear();
        this->frame_dle_ = false;
        this->state_ = CommState::READ_FRAME;
        this->state_deadline_ms_ = millis() + FRAME_TIMEOUT_MS;
      } else if (b == 0x15) {
        this->fail_attempt_("NAK while waiting for STX");
      }
      break;

    case CommState::READ_FRAME:
      if (this->frame_dle_) {
        if (b == 0x03) {
          this->frame_dle_ = false;
          this->complete_frame_();
        } else if (b == 0x10) {
          this->frame_.push_back(0x10);
          this->frame_dle_ = false;
        } else {
          this->frame_dle_ = false;
          this->fail_attempt_("invalid DLE escape in response");
        }
      } else if (b == 0x10) {
        this->frame_dle_ = true;
      } else {
        this->frame_.push_back(b);
      }
      break;

    default:
      break;
  }
}

void LWZ2Component::handle_timeout_() {
  const char *where = "communication";
  if (this->state_ == CommState::WAIT_WAKE_ACK)
    where = "wakeup ACK";
  else if (this->state_ == CommState::WAIT_CMD_ACK_1)
    where = "command ACK";
  else if (this->state_ == CommState::WAIT_CMD_ACK_2)
    where = "command STX";
  else if (this->state_ == CommState::READ_FRAME)
    where = "response frame";
  this->fail_attempt_(std::string("timeout waiting for ") + where);
}

void LWZ2Component::fail_attempt_(const std::string &reason) {
  ESP_LOGW(TAG, "Command 0x%02X attempt %u failed: %s", this->current_command_, this->attempt_, reason.c_str());
  if (this->attempt_ < MAX_ATTEMPTS) {
    this->attempt_++;
    this->state_ = CommState::RETRY_WAIT;
    this->retry_at_ms_ = millis() + RETRY_DELAY_MS;
    return;
  }
  this->finalize_failure_(reason);
}

void LWZ2Component::finalize_failure_(const std::string &reason) {
  const uint32_t now = millis();
  this->total_errors_++;
  this->consecutive_errors_++;
  this->last_error_message_ = reason;
  this->state_ = CommState::IDLE;
  this->flush_rx_();

  uint32_t backoff = 1000;
  if (this->consecutive_errors_ >= 6)
    backoff = 30000;
  else if (this->consecutive_errors_ >= 3)
    backoff = 10000;
  this->next_allowed_request_ms_ = now + backoff;

  // Re-schedule the failed block after the backoff rather than immediately hammering the controller.
  switch (this->current_command_) {
    case 0xFB: this->next_fb_ms_ = now + backoff; break;
    case 0xF4: this->next_f4_ms_ = now + backoff; break;
    case 0xE8: this->next_e8_ms_ = now + backoff; break;
    case 0x01: this->next_01_ms_ = now + backoff; break;
    case 0x09: this->next_09_ms_ = now + backoff; break;
    default: break;
  }

  if (this->last_error_text_sensor_ != nullptr)
    this->last_error_text_sensor_->publish_state(reason);
  if (this->communication_status_text_sensor_ != nullptr)
    this->communication_status_text_sensor_->publish_state(this->consecutive_errors_ >= 3 ? "degraded" : "warning");
}

void LWZ2Component::complete_frame_() {
  // FHEM 2.x transport acknowledges the completed response with one DLE (0x10).
  // The next transaction starts with its own STX (0x02).
  this->write_byte(0x10);

  const std::vector<uint8_t> clean = this->deescape_2b18_(this->frame_);
  std::string reason;
  if (!this->validate_frame_(clean, reason)) {
    this->fail_attempt_(reason);
    return;
  }

  this->publish_raw_(this->current_command_, clean);
  this->parse_frame_(this->current_command_, clean);
  this->mark_success_();
}

void LWZ2Component::mark_success_() {
  const uint32_t now = millis();
  this->last_success_any_ms_ = now;
  this->consecutive_errors_ = 0;
  this->stale_published_ = false;
  this->last_error_message_ = "none";
  this->state_ = CommState::IDLE;
  this->next_allowed_request_ms_ = now + MIN_REQUEST_GAP_MS;

  switch (this->current_command_) {
    case 0xFB:
      this->last_fb_success_ms_ = now;
      this->next_fb_ms_ = now + this->status_interval_ms_;
      break;
    case 0xF4:
      this->last_f4_success_ms_ = now;
      this->next_f4_ms_ = now + this->heating_interval_ms_;
      break;
    case 0xE8:
      this->last_e8_success_ms_ = now;
      this->next_e8_ms_ = now + this->fan_interval_ms_;
      break;
    case 0x01:
      this->last_01_success_ms_ = now;
      this->next_01_ms_ = now + this->fan_config_interval_ms_;
      break;
    case 0x09:
      this->last_09_success_ms_ = now;
      this->next_09_ms_ = now + this->history_interval_ms_;
      break;
    default:
      break;
  }

  if (this->communication_ok_binary_sensor_ != nullptr)
    this->communication_ok_binary_sensor_->publish_state(true);
  if (this->communication_status_text_sensor_ != nullptr)
    this->communication_status_text_sensor_->publish_state("connected");
  if (this->last_error_text_sensor_ != nullptr)
    this->last_error_text_sensor_->publish_state("none");
}

void LWZ2Component::flush_rx_() {
  uint8_t dummy;
  while (this->available())
    this->read_byte(&dummy);
}

std::vector<uint8_t> LWZ2Component::deescape_2b18_(const std::vector<uint8_t> &in) const {
  std::vector<uint8_t> out;
  out.reserve(in.size());
  for (size_t i = 0; i < in.size(); i++) {
    if (in[i] == 0x2B && i + 1 < in.size() && in[i + 1] == 0x18) {
      out.push_back(0x2B);
      i++;
    } else {
      out.push_back(in[i]);
    }
  }
  return out;
}

bool LWZ2Component::validate_frame_(const std::vector<uint8_t> &frame, std::string &reason) const {
  if (frame.size() < 4) {
    reason = "response too short";
    return false;
  }
  if (frame[0] != 0x01) {
    reason = "invalid response header";
    return false;
  }
  if (frame[1] != 0x00) {
    char tmp[48];
    std::snprintf(tmp, sizeof(tmp), "LWZ error response 0x%02X", frame[1]);
    reason = tmp;
    return false;
  }
  if (frame[3] != this->current_command_) {
    char tmp[64];
    std::snprintf(tmp, sizeof(tmp), "response command mismatch: got 0x%02X expected 0x%02X", frame[3], this->current_command_);
    reason = tmp;
    return false;
  }
  uint16_t sum = 0;
  for (size_t i = 0; i < frame.size(); i++) {
    if (i != 2)
      sum += frame[i];
  }
  if (static_cast<uint8_t>(sum & 0xFF) != frame[2]) {
    char tmp[64];
    std::snprintf(tmp, sizeof(tmp), "checksum mismatch: calc 0x%02X got 0x%02X",
                  static_cast<unsigned>(sum & 0xFF), frame[2]);
    reason = tmp;
    return false;
  }
  return true;
}

std::string LWZ2Component::frame_to_hex_(const std::vector<uint8_t> &frame) const {
  static const char HEX[] = "0123456789ABCDEF";
  std::string s;
  s.reserve(frame.size() * 2);
  for (uint8_t b : frame) {
    s.push_back(HEX[(b >> 4) & 0x0F]);
    s.push_back(HEX[b & 0x0F]);
  }
  return s;
}

void LWZ2Component::publish_raw_(uint8_t command, const std::vector<uint8_t> &frame) {
  const std::string raw = this->frame_to_hex_(frame);
  ESP_LOGV(TAG, "RX 0x%02X: %s", command, raw.c_str());
  text_sensor::TextSensor *target = nullptr;
  if (command == 0xFB) target = this->raw_fb_text_sensor_;
  else if (command == 0xF4) target = this->raw_f4_text_sensor_;
  else if (command == 0xE8) target = this->raw_e8_text_sensor_;
  else if (command == 0x01) target = this->raw_01_text_sensor_;
  else if (command == 0x09) target = this->raw_09_text_sensor_;
  if (target != nullptr)
    target->publish_state(raw);
}

void LWZ2Component::parse_frame_(uint8_t command, const std::vector<uint8_t> &frame) {
  if (frame.size() <= 4)
    return;
  const uint8_t *data = frame.data() + 4;
  const size_t len = frame.size() - 4;

  if (command == 0xFB) this->parse_fb_(data, len);
  else if (command == 0xF4) this->parse_f4_(data, len);
  else if (command == 0xE8) this->parse_e8_(data, len);
  else if (command == 0x01) this->parse_01_(data, len);
  else if (command == 0x09) this->parse_09_(data, len);
}

void LWZ2Component::parse_fb_(const uint8_t *d, size_t len) {
#define PUB_TEMP(member, offset) do { if (len >= (offset) + 2 && member##_sensor_ != nullptr) member##_sensor_->publish_state(temp10_(d + (offset))); } while (0)
  PUB_TEMP(outside_temperature, 2);
  PUB_TEMP(flow_temperature, 4);
  PUB_TEMP(return_temperature, 6);
  PUB_TEMP(hot_gas_temperature, 8);
  PUB_TEMP(dhw_temperature, 10);
  PUB_TEMP(flow_temperature_hc2, 12);
  PUB_TEMP(inside_temperature, 14);
  PUB_TEMP(evaporator_temperature, 16);
  PUB_TEMP(condenser_temperature, 18);
#undef PUB_TEMP

  if (len > 20) {
    const uint8_t upper = (d[20] >> 4) & 0x0F;
    const uint8_t lower = d[20] & 0x0F;
    const bool compressor = upper & 0x01;
    const bool booster1 = upper & 0x02;
    const bool solar = this->firmware_ == FirmwareProfile::FW_206 ? false : (upper & 0x04);
    const bool booster2 = this->firmware_ == FirmwareProfile::FW_206 ? (upper & 0x04) : (upper & 0x08);
    const bool heating_pump = lower & 0x01;
    const bool dhw_pump = lower & 0x02;
    const bool diverter = lower & 0x04;
    const bool heat_pipe = lower & 0x08;
    this->publish_status_bits_(compressor, heating_pump, dhw_pump, solar, booster1, booster2, diverter, heat_pipe);
  }

  if (len > 22 && this->exhaust_fan_power_sensor_ != nullptr)
    this->exhaust_fan_power_sensor_->publish_state(d[22]);
  if (len > 23 && this->supply_fan_power_sensor_ != nullptr)
    this->supply_fan_power_sensor_->publish_state(d[23]);
  if (len > 24 && this->main_fan_power_sensor_ != nullptr)
    this->main_fan_power_sensor_->publish_state(static_cast<float>(d[24]) / 2.55f);
  if (len > 26 && this->exhaust_fan_speed_percent_sensor_ != nullptr)
    this->exhaust_fan_speed_percent_sensor_->publish_state(d[26]);
  if (len > 27 && this->supply_fan_speed_percent_sensor_ != nullptr)
    this->supply_fan_speed_percent_sensor_->publish_state(d[27]);
  if (len > 28 && this->main_fan_speed_percent_sensor_ != nullptr)
    this->main_fan_speed_percent_sensor_->publish_state(d[28]);
}

void LWZ2Component::parse_f4_(const uint8_t *d, size_t len) {
  // F4's later offsets differ between 2.06 / 2.14 / 2.14j.
  if (this->firmware_ == FirmwareProfile::FW_214J) {
    if (len >= 43 && this->inside_temperature_sensor_ != nullptr)
      this->inside_temperature_sensor_->publish_state(temp10_(d + 41));
    else if (len >= 37 && this->inside_temperature_sensor_ != nullptr)
      this->inside_temperature_sensor_->publish_state(temp10_(d + 35));

    // 2.14j mirrors the useful actuator bits in F4 at hex positions 40/41.
    if (len > 18) {
      const uint8_t upper = (d[18] >> 4) & 0x0F;
      const uint8_t lower = d[18] & 0x0F;
      this->publish_status_bits_(upper & 0x01, lower & 0x01, lower & 0x02, upper & 0x04,
                                 upper & 0x02, upper & 0x08, lower & 0x04, lower & 0x08);
    }
  } else {
    if (len >= 34 && this->inside_temperature_sensor_ != nullptr)
      this->inside_temperature_sensor_->publish_state(temp10_(d + 32));
  }
}

void LWZ2Component::parse_e8_(const uint8_t *d, size_t len) {
  if (len >= 12 && this->supply_fan_speed_sensor_ != nullptr)
    this->supply_fan_speed_sensor_->publish_state(be16u_(d + 10));
  if (len >= 14 && this->exhaust_fan_speed_sensor_ != nullptr)
    this->exhaust_fan_speed_sensor_->publish_state(be16u_(d + 12));
  if (len >= 16 && this->supply_fan_airflow_set_sensor_ != nullptr)
    this->supply_fan_airflow_set_sensor_->publish_state(be16u_(d + 14));
  if (len >= 18 && this->exhaust_fan_airflow_set_sensor_ != nullptr)
    this->exhaust_fan_airflow_set_sensor_->publish_state(be16u_(d + 16));
  if (len >= 20 && this->supply_fan_speed_target_sensor_ != nullptr)
    this->supply_fan_speed_target_sensor_->publish_state(be16u_(d + 18));
  if (len >= 22 && this->exhaust_fan_speed_target_sensor_ != nullptr)
    this->exhaust_fan_speed_target_sensor_->publish_state(be16u_(d + 20));
}

void LWZ2Component::parse_01_(const uint8_t *d, size_t len) {
  if (this->firmware_ == FirmwareProfile::FW_206) {
    if (len >= 2 && this->supply_stage1_airflow_sensor_ != nullptr) this->supply_stage1_airflow_sensor_->publish_state(be16u_(d + 0));
    if (len >= 4 && this->supply_stage2_airflow_sensor_ != nullptr) this->supply_stage2_airflow_sensor_->publish_state(be16u_(d + 2));
    if (len >= 6 && this->supply_stage3_airflow_sensor_ != nullptr) this->supply_stage3_airflow_sensor_->publish_state(be16u_(d + 4));
    if (len >= 8 && this->exhaust_stage1_airflow_sensor_ != nullptr) this->exhaust_stage1_airflow_sensor_->publish_state(be16u_(d + 6));
    if (len >= 10 && this->exhaust_stage2_airflow_sensor_ != nullptr) this->exhaust_stage2_airflow_sensor_->publish_state(be16u_(d + 8));
    if (len >= 12 && this->exhaust_stage3_airflow_sensor_ != nullptr) this->exhaust_stage3_airflow_sensor_->publish_state(be16u_(d + 10));
  } else {
    if (len >= 1 && this->supply_stage1_airflow_sensor_ != nullptr) this->supply_stage1_airflow_sensor_->publish_state(d[0]);
    if (len >= 2 && this->supply_stage2_airflow_sensor_ != nullptr) this->supply_stage2_airflow_sensor_->publish_state(d[1]);
    if (len >= 3 && this->supply_stage3_airflow_sensor_ != nullptr) this->supply_stage3_airflow_sensor_->publish_state(d[2]);
    if (len >= 4 && this->exhaust_stage1_airflow_sensor_ != nullptr) this->exhaust_stage1_airflow_sensor_->publish_state(d[3]);
    if (len >= 5 && this->exhaust_stage2_airflow_sensor_ != nullptr) this->exhaust_stage2_airflow_sensor_->publish_state(d[4]);
    if (len >= 6 && this->exhaust_stage3_airflow_sensor_ != nullptr) this->exhaust_stage3_airflow_sensor_->publish_state(d[5]);
  }
}

void LWZ2Component::parse_09_(const uint8_t *d, size_t len) {
  if (len >= 2 && this->operating_hours_1_sensor_ != nullptr) this->operating_hours_1_sensor_->publish_state(be16u_(d + 0));
  if (len >= 4 && this->operating_hours_2_sensor_ != nullptr) this->operating_hours_2_sensor_->publish_state(be16u_(d + 2));
  if (len >= 6 && this->heating_hours_sensor_ != nullptr) this->heating_hours_sensor_->publish_state(be16u_(d + 4));
  if (len >= 8 && this->dhw_hours_sensor_ != nullptr) this->dhw_hours_sensor_->publish_state(be16u_(d + 6));
  if (len >= 10 && this->cooling_hours_sensor_ != nullptr) this->cooling_hours_sensor_->publish_state(be16u_(d + 8));
}

void LWZ2Component::publish_status_bits_(bool compressor, bool heating_pump, bool dhw_pump, bool solar_pump,
                                         bool booster1, bool booster2, bool diverter, bool heat_pipe) {
  if (this->compressor_binary_sensor_ != nullptr) this->compressor_binary_sensor_->publish_state(compressor);
  if (this->heating_circuit_pump_binary_sensor_ != nullptr) this->heating_circuit_pump_binary_sensor_->publish_state(heating_pump);
  if (this->dhw_pump_binary_sensor_ != nullptr) this->dhw_pump_binary_sensor_->publish_state(dhw_pump);
  // FHEM marks solarPump in FBglob206 as n.a.; don't invent a state for 2.06.
  if (this->firmware_ != FirmwareProfile::FW_206 && this->solar_pump_binary_sensor_ != nullptr)
    this->solar_pump_binary_sensor_->publish_state(solar_pump);
  if (this->booster_stage1_binary_sensor_ != nullptr) this->booster_stage1_binary_sensor_->publish_state(booster1);
  if (this->booster_stage2_binary_sensor_ != nullptr) this->booster_stage2_binary_sensor_->publish_state(booster2);
  if (this->diverter_valve_binary_sensor_ != nullptr) this->diverter_valve_binary_sensor_->publish_state(diverter);
  if (this->heat_pipe_valve_binary_sensor_ != nullptr) this->heat_pipe_valve_binary_sensor_->publish_state(heat_pipe);

  // Read-only semantic states: for these 2.x profiles the circuit pumps are the least ambiguous source.
  if (this->heating_active_binary_sensor_ != nullptr) this->heating_active_binary_sensor_->publish_state(heating_pump);
  if (this->dhw_active_binary_sensor_ != nullptr) this->dhw_active_binary_sensor_->publish_state(dhw_pump);
}

void LWZ2Component::update_communication_state_() {
  const uint32_t now = millis();
  if (this->last_success_any_ms_ == 0) {
    if ((now - this->startup_ms_) > this->stale_after_ms_ && !this->stale_published_) {
      this->stale_published_ = true;
      if (this->communication_ok_binary_sensor_ != nullptr)
        this->communication_ok_binary_sensor_->publish_state(false);
      if (this->communication_status_text_sensor_ != nullptr)
        this->communication_status_text_sensor_->publish_state("disconnected");
    }
    return;
  }

  if ((now - this->last_success_any_ms_) > this->stale_after_ms_) {
    if (!this->stale_published_) {
      this->stale_published_ = true;
      if (this->communication_ok_binary_sensor_ != nullptr)
        this->communication_ok_binary_sensor_->publish_state(false);
      if (this->communication_status_text_sensor_ != nullptr)
        this->communication_status_text_sensor_->publish_state("disconnected");
      this->publish_stale_numeric_states_();
    }
  }
}

void LWZ2Component::publish_stale_numeric_states_() {
#define INVALIDATE_NUMERIC(name) do { if (name##_sensor_ != nullptr) name##_sensor_->publish_state(NAN); } while (0)
  INVALIDATE_NUMERIC(outside_temperature);
  INVALIDATE_NUMERIC(flow_temperature);
  INVALIDATE_NUMERIC(return_temperature);
  INVALIDATE_NUMERIC(hot_gas_temperature);
  INVALIDATE_NUMERIC(dhw_temperature);
  INVALIDATE_NUMERIC(flow_temperature_hc2);
  INVALIDATE_NUMERIC(inside_temperature);
  INVALIDATE_NUMERIC(evaporator_temperature);
  INVALIDATE_NUMERIC(condenser_temperature);
  INVALIDATE_NUMERIC(supply_fan_power);
  INVALIDATE_NUMERIC(exhaust_fan_power);
  INVALIDATE_NUMERIC(main_fan_power);
  INVALIDATE_NUMERIC(supply_fan_speed_percent);
  INVALIDATE_NUMERIC(exhaust_fan_speed_percent);
  INVALIDATE_NUMERIC(main_fan_speed_percent);
  INVALIDATE_NUMERIC(supply_fan_speed);
  INVALIDATE_NUMERIC(exhaust_fan_speed);
  INVALIDATE_NUMERIC(supply_fan_airflow_set);
  INVALIDATE_NUMERIC(exhaust_fan_airflow_set);
  INVALIDATE_NUMERIC(supply_fan_speed_target);
  INVALIDATE_NUMERIC(exhaust_fan_speed_target);
#undef INVALIDATE_NUMERIC
}

int16_t LWZ2Component::be16s_(const uint8_t *p) {
  return static_cast<int16_t>((static_cast<uint16_t>(p[0]) << 8) | p[1]);
}

uint16_t LWZ2Component::be16u_(const uint8_t *p) {
  return static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | p[1]);
}

float LWZ2Component::temp10_(const uint8_t *p) {
  return static_cast<float>(be16s_(p)) / 10.0f;
}

}  // namespace esphome::lwz2
