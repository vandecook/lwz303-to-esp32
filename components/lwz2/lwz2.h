#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

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
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_firmware(const std::string &fw);
  void set_status_interval(uint32_t v) { status_interval_ms_ = v; }
  void set_heating_interval(uint32_t v) { heating_interval_ms_ = v; }
  void set_fan_interval(uint32_t v) { fan_interval_ms_ = v; }
  void set_history_interval(uint32_t v) { history_interval_ms_ = v; }
  void set_fan_config_interval(uint32_t v) { fan_config_interval_ms_ = v; }
  void set_stale_after(uint32_t v) { stale_after_ms_ = v; }

#define LWZ2_SENSOR_SETTER(name) \
  void set_##name##_sensor(sensor::Sensor *s) { name##_sensor_ = s; }

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

  LWZ2_TEXT_SETTER(communication_status)
  LWZ2_TEXT_SETTER(last_error)
  LWZ2_TEXT_SETTER(raw_fb)
  LWZ2_TEXT_SETTER(raw_f4)
  LWZ2_TEXT_SETTER(raw_e8)
  LWZ2_TEXT_SETTER(raw_01)
  LWZ2_TEXT_SETTER(raw_09)

#undef LWZ2_TEXT_SETTER

 protected:
  static constexpr uint32_t ACK_TIMEOUT_MS = 1800;
  static constexpr uint32_t FRAME_TIMEOUT_MS = 1800;
  static constexpr uint32_t RETRY_DELAY_MS = 250;
  static constexpr uint32_t MIN_REQUEST_GAP_MS = 250;
  static constexpr uint8_t MAX_ATTEMPTS = 3;

  FirmwareProfile firmware_{FirmwareProfile::FW_214J};
  CommState state_{CommState::IDLE};

  uint32_t status_interval_ms_{5000};
  uint32_t heating_interval_ms_{10000};
  uint32_t fan_interval_ms_{30000};
  uint32_t history_interval_ms_{300000};
  uint32_t fan_config_interval_ms_{300000};
  uint32_t stale_after_ms_{30000};

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
