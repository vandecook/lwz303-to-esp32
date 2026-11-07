#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include "esphome/components/number/number.h"
#include "selected_fields.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/core/preferences.h"
#include "esphome/components/select/select.h"

namespace esphome::lwz2 {

enum class FirmwareProfile : uint8_t {
  FW_206 = 0,
  FW_214 = 1,
  FW_214J = 2,
};

enum class CommState : uint8_t {
  IDLE,
  WAIT_WAKE_ACK,
  WAIT_CMD_ACK_1,
  WAIT_CMD_ACK_2,
  READ_FRAME,
  RETRY_WAIT,
};

class LWZ2Component : public Component, public uart::UARTDevice {
 public:
  void set_v04_enabled(bool v) { v04_enabled_ = v; }
  void set_preferences_key(uint32_t v) { preferences_key_ = v; }
  void set_device_model(const std::string &v);
  void set_fan_speed_source(const std::string &v);
  void set_diagnostic_max_age(uint32_t v) { diagnostic_max_age_ms_ = v; }
  bool request_fan_stage(uint8_t index, float value);
  bool request_clock_mode(const std::string &mode);
  void confirm_clock_mode(bool summer);
  void reset_fan_faults();  // HA acknowledgement; no command is sent to the LWZ.
  void set_fan_stage_number(uint8_t i, number::Number *n) { if (i < 2) fan_stages_[i] = n; }
  void set_clock_mode_select(select::Select *s) { clock_mode_select_ = s; }
  void set_v04_sensor(const std::string &key, sensor::Sensor *s) { v04_sensors_[key] = s; }
  void set_v04_text(const std::string &key, text_sensor::TextSensor *s) { v04_text_[key] = s; }
  void set_v04_binary(const std::string &key, binary_sensor::BinarySensor *s) { v04_binary_[key] = s; }
  void set_v03_enabled(bool enabled) { v03_enabled_ = enabled; }
  void set_hc1_layout(const std::string &profile);
  void set_selected_sensor(const std::string &key, sensor::Sensor *s) { selected_sensors_[key] = s; }
  void set_selected_binary_sensor(const std::string &key, binary_sensor::BinarySensor *s) { selected_binary_[key] = s; }
  void set_selected_text_sensor(const std::string &key, text_sensor::TextSensor *s) { selected_text_[key] = s; }
  void set_setpoint_number(uint8_t index, number::Number *n) { if (index < 4) setpoints_[index] = n; }
  void set_writes_enabled(bool enabled);
  bool request_setpoint(uint8_t index, float value);
  bool request_clock_shift(int hours);
  void set_write_status_text_sensor(text_sensor::TextSensor *s) { write_status_ = s; }
  // Local controls only; none writes LWZ parameters.
  void set_dynamic_interval(uint32_t v) { dynamic_interval_ms_ = v; reschedule_poll_(); }
  void set_config_interval(uint32_t v) { config_interval_ms_ = v; next_config_ms_ = millis() + v; }
  void set_dynamic_enabled(bool v) { dynamic_enabled_ = v; reschedule_poll_(); }
  void set_trigger_compressor(bool v) { trigger_compressor_ = v; reschedule_poll_(); }
  void set_trigger_heating(bool v) { trigger_heating_ = v; reschedule_poll_(); }
  void set_trigger_dhw(bool v) { trigger_dhw_ = v; reschedule_poll_(); }
  uint32_t effective_interval() const;
  void refresh_config() { config_pending_ = true; }
  void reset_counters();
  void reinitialize();
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_firmware(const std::string &fw);
  void set_status_interval(uint32_t v) { status_interval_ms_ = v; reschedule_poll_(); }
  void set_heating_interval(uint32_t v) { heating_interval_ms_ = v; }
  void set_fan_interval(uint32_t v) { fan_interval_ms_ = v; }
  void set_history_interval(uint32_t v) { history_interval_ms_ = v; }
  void set_fan_config_interval(uint32_t v) { fan_config_interval_ms_ = v; }
  void set_stale_after(uint32_t v) { stale_after_ms_ = v; }

#define LWZ2_SENSOR_SETTER(name) \
  void set_##name##_sensor(sensor::Sensor *s) { name##_sensor_ = s; }

  LWZ2_SENSOR_SETTER(hc1_gradient)
  LWZ2_SENSOR_SETTER(hc1_low_end)
  LWZ2_SENSOR_SETTER(successful_queries)
  LWZ2_SENSOR_SETTER(failed_queries)
  LWZ2_SENSOR_SETTER(timeouts)
  LWZ2_SENSOR_SETTER(retries)
  LWZ2_SENSOR_SETTER(outside_temperature)
  LWZ2_SENSOR_SETTER(flow_temperature)
  LWZ2_SENSOR_SETTER(return_temperature)
  LWZ2_SENSOR_SETTER(hot_gas_temperature)
  LWZ2_SENSOR_SETTER(dhw_temperature)
  LWZ2_SENSOR_SETTER(flow_temperature_hc2)
  LWZ2_SENSOR_SETTER(inside_temperature)
  LWZ2_SENSOR_SETTER(evaporator_temperature)
  LWZ2_SENSOR_SETTER(condenser_temperature)
  LWZ2_SENSOR_SETTER(supply_fan_power)
  LWZ2_SENSOR_SETTER(exhaust_fan_power)
  LWZ2_SENSOR_SETTER(main_fan_power)
  LWZ2_SENSOR_SETTER(supply_fan_speed_percent)
  LWZ2_SENSOR_SETTER(exhaust_fan_speed_percent)
  LWZ2_SENSOR_SETTER(main_fan_speed_percent)
  LWZ2_SENSOR_SETTER(supply_fan_speed)
  LWZ2_SENSOR_SETTER(exhaust_fan_speed)
  LWZ2_SENSOR_SETTER(supply_fan_airflow_set)
  LWZ2_SENSOR_SETTER(exhaust_fan_airflow_set)
  LWZ2_SENSOR_SETTER(supply_fan_speed_target)
  LWZ2_SENSOR_SETTER(exhaust_fan_speed_target)
  LWZ2_SENSOR_SETTER(supply_stage1_airflow)
  LWZ2_SENSOR_SETTER(supply_stage2_airflow)
  LWZ2_SENSOR_SETTER(supply_stage3_airflow)
  LWZ2_SENSOR_SETTER(exhaust_stage1_airflow)
  LWZ2_SENSOR_SETTER(exhaust_stage2_airflow)
  LWZ2_SENSOR_SETTER(exhaust_stage3_airflow)
  LWZ2_SENSOR_SETTER(operating_hours_1)
  LWZ2_SENSOR_SETTER(operating_hours_2)
  LWZ2_SENSOR_SETTER(heating_hours)
  LWZ2_SENSOR_SETTER(dhw_hours)
  LWZ2_SENSOR_SETTER(cooling_hours)

#undef LWZ2_SENSOR_SETTER

#define LWZ2_BINARY_SETTER(name) \
  void set_##name##_binary_sensor(binary_sensor::BinarySensor *s) { name##_binary_sensor_ = s; }

  LWZ2_BINARY_SETTER(compressor)
  LWZ2_BINARY_SETTER(heating_circuit_pump)
  LWZ2_BINARY_SETTER(dhw_pump)
  LWZ2_BINARY_SETTER(solar_pump)
  LWZ2_BINARY_SETTER(booster_stage1)
  LWZ2_BINARY_SETTER(booster_stage2)
  LWZ2_BINARY_SETTER(diverter_valve)
  LWZ2_BINARY_SETTER(heat_pipe_valve)
  LWZ2_BINARY_SETTER(heating_active)
  LWZ2_BINARY_SETTER(dhw_active)
  LWZ2_BINARY_SETTER(communication_ok)

#undef LWZ2_BINARY_SETTER

#define LWZ2_TEXT_SETTER(name) \
  void set_##name##_text_sensor(text_sensor::TextSensor *s) { name##_text_sensor_ = s; }

  LWZ2_TEXT_SETTER(last_success)
  LWZ2_TEXT_SETTER(config_updated)
  LWZ2_TEXT_SETTER(firmware_version)
  LWZ2_TEXT_SETTER(communication_status)
  LWZ2_TEXT_SETTER(last_error)
  LWZ2_TEXT_SETTER(raw_fb)
  LWZ2_TEXT_SETTER(raw_f4)
  LWZ2_TEXT_SETTER(raw_e8)
  LWZ2_TEXT_SETTER(raw_01)
  LWZ2_TEXT_SETTER(raw_09)

#undef LWZ2_TEXT_SETTER

 protected:
  struct FanFaultRecord {
    uint32_t count{0};
    bool active{false};
    char begin[24]{}, end[24]{};
  };
  struct V04Saved {
    uint32_t magic{0x4C575A04};
    int8_t clock_mode{0};  // 0 winter (user-confirmed initial state), 1 summer, -1 uncertain
    FanFaultRecord fans[2]{};  // 0 exhaust/F5, 1 supply/F6
    char reset_reason[64]{};
  } saved_;
  bool v04_enabled_{false};
  uint32_t preferences_key_{0x4C575A04}, diagnostic_max_age_ms_{30000};
  ESPPreferenceObject v04_preference_;
  bool persistence_ok_{true};
  std::string device_model_{"unknown"}, speed_source_{"sGlobal"};
  int pending_clock_mode_{-2};
  select::Select *clock_mode_select_{nullptr};
  number::Number *fan_stages_[2]{nullptr, nullptr};
  std::map<std::string, sensor::Sensor *> v04_sensors_;
  std::map<std::string, text_sensor::TextSensor *> v04_text_;
  std::map<std::string, binary_sensor::BinarySensor *> v04_binary_;
  uint32_t v04_due_[2]{0, 0}, zero_since_[2]{0, 0};
  bool zero_pending_[2]{false, false};
  float v04_speed_[2]{NAN, NAN};
  uint32_t speed_ms_{0}, program_ms_{0}, settings_ms_{0}, override_ms_{0}, clock_ms_{0};
  bool speed_seen_{false}, program_seen_{false}, settings_seen_{false}, override_seen_{false}, clock_seen_{false};
  int fan_day_{-1}, fan_night_{-1}, fan_standby_{-1}, fan_manual_{-1};
  int fan_program_{-1}, program_mode_{-1}, override_stage_{-1}, override_remaining_{-1};
  std::string clock_stamp_{"unbekannt"};
  void v04_setup_();
  void v04_parse_(uint8_t command, const uint8_t *data, size_t len);
  void v04_tick_(bool speed_observation = false);
  void v04_invalidate_();
  bool v04_save_();
  int current_fan_stage_() const;
  void v04_history_();
  void clock_mode_publish_();
  void v04_number_(const std::string &key, float value);
  void v04_publish_text_(const std::string &key, const std::string &value);
  enum class WriteStage : uint8_t { NONE, READ, SET, VERIFY };
  bool v03_enabled_{false}, writes_enabled_{false}, sending_set_{false}, write_transaction_{false};
  FirmwareProfile hc1_layout_{FirmwareProfile::FW_206};
  WriteStage write_stage_{WriteStage::NONE};
  uint8_t write_command_{0}, write_index_{0};
  int clock_shift_{0};
  float write_value_{0};
  uint32_t write_read_ms_{0}, last_clock_request_ms_{0};
  bool clock_request_seen_{false};
  std::vector<uint8_t> write_expected_;
  number::Number *setpoints_[4]{nullptr, nullptr, nullptr, nullptr};
  text_sensor::TextSensor *write_status_{nullptr};
  std::map<std::string, sensor::Sensor *> selected_sensors_;
  std::map<std::string, binary_sensor::BinarySensor *> selected_binary_;
  std::map<std::string, text_sensor::TextSensor *> selected_text_;
  uint32_t v03_due_[4]{0, 0, 0, 0};
  uint32_t block_success_[256]{};
  bool block_stale_[256]{};
  void parse_selected_(uint8_t command, const uint8_t *data, size_t len, bool invalidate = false);
  void v03_stale_();
  void invalidate_block_(uint8_t command);
  bool v03_schedule_();
  void v03_schedule_result_(uint8_t command);
  void write_response_(const std::vector<uint8_t> &frame);
  void write_failed_(const std::string &reason);
  void finish_write_(const std::string &status);
  void publish_write_status_(const std::string &status);
  const SelectedField *selected_rule_(const std::string &key, uint8_t command, bool &belongs) const;
  float selected_value_(const SelectedField &rule, const uint8_t *data) const;
  static constexpr uint32_t ACK_TIMEOUT_MS = 1800;
  static constexpr uint32_t FRAME_TIMEOUT_MS = 1800;
  static constexpr uint32_t RETRY_DELAY_MS = 250;
  static constexpr uint32_t MIN_REQUEST_GAP_MS = 250;
  static constexpr uint8_t MAX_ATTEMPTS = 3;

  FirmwareProfile firmware_{FirmwareProfile::FW_214J};
  CommState state_{CommState::IDLE};

  uint32_t status_interval_ms_{10000};
  uint32_t heating_interval_ms_{10000};
  uint32_t fan_interval_ms_{30000};
  uint32_t history_interval_ms_{300000};
  uint32_t fan_config_interval_ms_{300000};
  uint32_t stale_after_ms_{30000};

  uint32_t dynamic_interval_ms_{5000};
  uint32_t config_interval_ms_{3600000};
  uint32_t next_config_ms_{0};
  uint32_t last_diagnostics_ms_{0};
  uint32_t successes_{0}, timeouts_{0}, retries_{0};
  bool dynamic_enabled_{true};
  bool trigger_compressor_{true}, trigger_heating_{true}, trigger_dhw_{true};
  bool compressor_{false}, heating_{false}, dhw_{false};
  bool config_pending_{true}, firmware_pending_{true};
  void publish_diagnostics_();
  void reschedule_poll_() {
    // Restoring controls at boot must not postpone the first live-data poll.
    if (setup_complete_ && last_fb_success_ms_ != 0)
      next_fb_ms_ = last_fb_success_ms_ + effective_interval();
  }
  uint32_t next_fb_ms_{0};
  uint32_t next_f4_ms_{0};
  uint32_t next_e8_ms_{0};
  uint32_t next_01_ms_{0};
  uint32_t next_09_ms_{0};
  uint32_t state_deadline_ms_{0};
  uint32_t retry_at_ms_{0};
  uint32_t next_allowed_request_ms_{0};
  uint32_t last_success_any_ms_{0};
  uint32_t last_fb_success_ms_{0};
  uint32_t last_f4_success_ms_{0};
  uint32_t last_e8_success_ms_{0};
  uint32_t last_01_success_ms_{0};
  uint32_t last_09_success_ms_{0};

  uint8_t current_command_{0};
  uint8_t attempt_{0};
  uint32_t total_errors_{0};
  uint32_t consecutive_errors_{0};
  bool frame_dle_{false};
  bool stale_published_{false};
  bool setup_complete_{false};
  uint32_t startup_ms_{0};
  std::vector<uint8_t> frame_;
  std::string last_error_message_{"none"};

  void scheduler_();
  void start_transaction_(uint8_t command);
  void restart_transaction_();
  void send_command_frame_();
  void process_uart_();
  void handle_byte_(uint8_t b);
  void handle_timeout_();
  void complete_frame_();
  void fail_attempt_(const std::string &reason);
  void finalize_failure_(const std::string &reason);
  void mark_success_();
  void reset_schedule_();
  void flush_rx_();

  std::vector<uint8_t> deescape_2b18_(const std::vector<uint8_t> &in) const;
  bool validate_frame_(const std::vector<uint8_t> &frame, std::string &reason) const;
  std::string frame_to_hex_(const std::vector<uint8_t> &frame) const;
  void publish_raw_(uint8_t command, const std::vector<uint8_t> &frame);
  void parse_frame_(uint8_t command, const std::vector<uint8_t> &frame);
  void parse_fb_(const uint8_t *data, size_t len);
  void parse_f4_(const uint8_t *data, size_t len);
  void parse_e8_(const uint8_t *data, size_t len);
  void parse_01_(const uint8_t *data, size_t len);
  void parse_09_(const uint8_t *data, size_t len);
  void publish_status_bits_(bool compressor, bool heating_pump, bool dhw_pump, bool solar_pump,
                            bool booster1, bool booster2, bool diverter, bool heat_pipe);
  void update_communication_state_();
  void publish_stale_numeric_states_();

  static int16_t be16s_(const uint8_t *p);
  static uint16_t be16u_(const uint8_t *p);
  static float temp10_(const uint8_t *p);
  static bool time_reached_(uint32_t now, uint32_t target) { return static_cast<int32_t>(now - target) >= 0; }

#define LWZ2_SENSOR_MEMBER(name) sensor::Sensor *name##_sensor_{nullptr};
  LWZ2_SENSOR_MEMBER(hc1_gradient)
  LWZ2_SENSOR_MEMBER(hc1_low_end)
  LWZ2_SENSOR_MEMBER(successful_queries)
  LWZ2_SENSOR_MEMBER(failed_queries)
  LWZ2_SENSOR_MEMBER(timeouts)
  LWZ2_SENSOR_MEMBER(retries)
  LWZ2_SENSOR_MEMBER(outside_temperature)
  LWZ2_SENSOR_MEMBER(flow_temperature)
  LWZ2_SENSOR_MEMBER(return_temperature)
  LWZ2_SENSOR_MEMBER(hot_gas_temperature)
  LWZ2_SENSOR_MEMBER(dhw_temperature)
  LWZ2_SENSOR_MEMBER(flow_temperature_hc2)
  LWZ2_SENSOR_MEMBER(inside_temperature)
  LWZ2_SENSOR_MEMBER(evaporator_temperature)
  LWZ2_SENSOR_MEMBER(condenser_temperature)
  LWZ2_SENSOR_MEMBER(supply_fan_power)
  LWZ2_SENSOR_MEMBER(exhaust_fan_power)
  LWZ2_SENSOR_MEMBER(main_fan_power)
  LWZ2_SENSOR_MEMBER(supply_fan_speed_percent)
  LWZ2_SENSOR_MEMBER(exhaust_fan_speed_percent)
  LWZ2_SENSOR_MEMBER(main_fan_speed_percent)
  LWZ2_SENSOR_MEMBER(supply_fan_speed)
  LWZ2_SENSOR_MEMBER(exhaust_fan_speed)
  LWZ2_SENSOR_MEMBER(supply_fan_airflow_set)
  LWZ2_SENSOR_MEMBER(exhaust_fan_airflow_set)
  LWZ2_SENSOR_MEMBER(supply_fan_speed_target)
  LWZ2_SENSOR_MEMBER(exhaust_fan_speed_target)
  LWZ2_SENSOR_MEMBER(supply_stage1_airflow)
  LWZ2_SENSOR_MEMBER(supply_stage2_airflow)
  LWZ2_SENSOR_MEMBER(supply_stage3_airflow)
  LWZ2_SENSOR_MEMBER(exhaust_stage1_airflow)
  LWZ2_SENSOR_MEMBER(exhaust_stage2_airflow)
  LWZ2_SENSOR_MEMBER(exhaust_stage3_airflow)
  LWZ2_SENSOR_MEMBER(operating_hours_1)
  LWZ2_SENSOR_MEMBER(operating_hours_2)
  LWZ2_SENSOR_MEMBER(heating_hours)
  LWZ2_SENSOR_MEMBER(dhw_hours)
  LWZ2_SENSOR_MEMBER(cooling_hours)
#undef LWZ2_SENSOR_MEMBER

#define LWZ2_BINARY_MEMBER(name) binary_sensor::BinarySensor *name##_binary_sensor_{nullptr};
  LWZ2_BINARY_MEMBER(compressor)
  LWZ2_BINARY_MEMBER(heating_circuit_pump)
  LWZ2_BINARY_MEMBER(dhw_pump)
  LWZ2_BINARY_MEMBER(solar_pump)
  LWZ2_BINARY_MEMBER(booster_stage1)
  LWZ2_BINARY_MEMBER(booster_stage2)
  LWZ2_BINARY_MEMBER(diverter_valve)
  LWZ2_BINARY_MEMBER(heat_pipe_valve)
  LWZ2_BINARY_MEMBER(heating_active)
  LWZ2_BINARY_MEMBER(dhw_active)
  LWZ2_BINARY_MEMBER(communication_ok)
#undef LWZ2_BINARY_MEMBER

#define LWZ2_TEXT_MEMBER(name) text_sensor::TextSensor *name##_text_sensor_{nullptr};
  LWZ2_TEXT_MEMBER(last_success)
  LWZ2_TEXT_MEMBER(config_updated)
  LWZ2_TEXT_MEMBER(firmware_version)
  LWZ2_TEXT_MEMBER(communication_status)
  LWZ2_TEXT_MEMBER(last_error)
  LWZ2_TEXT_MEMBER(raw_fb)
  LWZ2_TEXT_MEMBER(raw_f4)
  LWZ2_TEXT_MEMBER(raw_e8)
  LWZ2_TEXT_MEMBER(raw_01)
  LWZ2_TEXT_MEMBER(raw_09)
#undef LWZ2_TEXT_MEMBER
};

}  // namespace esphome::lwz2

namespace esphome::lwz2 {
class LWZSetpointNumber : public number::Number {
 public:
  void set_parent(LWZ2Component *parent, uint8_t index) { parent_ = parent; index_ = index; }
 protected:
  void control(float value) override { if (parent_) parent_->request_setpoint(index_, value); }
  LWZ2Component *parent_{nullptr};
  uint8_t index_{0};
};
class LWZFanStageNumber : public number::Number {
 public:
  void set_parent(LWZ2Component *p, uint8_t i) { parent_ = p; index_ = i; }
 protected:
  void control(float v) override { if (parent_) parent_->request_fan_stage(index_, v); }
  LWZ2Component *parent_{nullptr};
  uint8_t index_{0};
};
class LWZClockModeSelect : public select::Select {
 public:
  void set_parent(LWZ2Component *p) { parent_ = p; }
 protected:
  void control(const std::string &v) override { if (parent_) parent_->request_clock_mode(v); }
  LWZ2Component *parent_{nullptr};
};
}
