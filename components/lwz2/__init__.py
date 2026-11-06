import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor, text_sensor, uart
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_HOUR,
    UNIT_PERCENT,
)

CODEOWNERS = []
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "binary_sensor", "text_sensor"]

CONF_FIRMWARE = "firmware"
CONF_STATUS_INTERVAL = "status_interval"
CONF_HEATING_INTERVAL = "heating_interval"
CONF_FAN_INTERVAL = "fan_interval"
CONF_HISTORY_INTERVAL = "history_interval"
CONF_FAN_CONFIG_INTERVAL = "fan_config_interval"
CONF_STALE_AFTER = "stale_after"

CONF_OUTSIDE_TEMPERATURE = "outside_temperature"
CONF_FLOW_TEMPERATURE = "flow_temperature"
CONF_RETURN_TEMPERATURE = "return_temperature"
CONF_HOT_GAS_TEMPERATURE = "hot_gas_temperature"
CONF_DHW_TEMPERATURE = "dhw_temperature"
CONF_FLOW_TEMPERATURE_HC2 = "flow_temperature_hc2"
CONF_INSIDE_TEMPERATURE = "inside_temperature"
CONF_EVAPORATOR_TEMPERATURE = "evaporator_temperature"
CONF_CONDENSER_TEMPERATURE = "condenser_temperature"

CONF_SUPPLY_FAN_POWER = "supply_fan_power"
CONF_EXHAUST_FAN_POWER = "exhaust_fan_power"
CONF_MAIN_FAN_POWER = "main_fan_power"
CONF_SUPPLY_FAN_SPEED_PERCENT = "supply_fan_speed_percent"
CONF_EXHAUST_FAN_SPEED_PERCENT = "exhaust_fan_speed_percent"
CONF_MAIN_FAN_SPEED_PERCENT = "main_fan_speed_percent"

CONF_SUPPLY_FAN_SPEED = "supply_fan_speed"
CONF_EXHAUST_FAN_SPEED = "exhaust_fan_speed"
CONF_SUPPLY_FAN_AIRFLOW_SET = "supply_fan_airflow_set"
CONF_EXHAUST_FAN_AIRFLOW_SET = "exhaust_fan_airflow_set"
CONF_SUPPLY_FAN_SPEED_TARGET = "supply_fan_speed_target"
CONF_EXHAUST_FAN_SPEED_TARGET = "exhaust_fan_speed_target"

CONF_SUPPLY_STAGE1_AIRFLOW = "supply_stage1_airflow"
CONF_SUPPLY_STAGE2_AIRFLOW = "supply_stage2_airflow"
CONF_SUPPLY_STAGE3_AIRFLOW = "supply_stage3_airflow"
CONF_EXHAUST_STAGE1_AIRFLOW = "exhaust_stage1_airflow"
CONF_EXHAUST_STAGE2_AIRFLOW = "exhaust_stage2_airflow"
CONF_EXHAUST_STAGE3_AIRFLOW = "exhaust_stage3_airflow"

CONF_OPERATING_HOURS_1 = "operating_hours_1"
CONF_OPERATING_HOURS_2 = "operating_hours_2"
CONF_HEATING_HOURS = "heating_hours"
CONF_DHW_HOURS = "dhw_hours"
CONF_COOLING_HOURS = "cooling_hours"

CONF_COMPRESSOR = "compressor"
CONF_HEATING_CIRCUIT_PUMP = "heating_circuit_pump"
CONF_DHW_PUMP = "dhw_pump"
CONF_SOLAR_PUMP = "solar_pump"
CONF_BOOSTER_STAGE1 = "booster_stage1"
CONF_BOOSTER_STAGE2 = "booster_stage2"
CONF_DIVERTER_VALVE = "diverter_valve"
CONF_HEAT_PIPE_VALVE = "heat_pipe_valve"
CONF_HEATING_ACTIVE = "heating_active"
CONF_DHW_ACTIVE = "dhw_active"
CONF_COMMUNICATION_OK = "communication_ok"

CONF_COMMUNICATION_STATUS = "communication_status"
CONF_LAST_ERROR = "last_error"
CONF_RAW_FB = "raw_fb"
CONF_RAW_F4 = "raw_f4"
CONF_RAW_E8 = "raw_e8"
CONF_RAW_01 = "raw_01"
CONF_RAW_09 = "raw_09"

lwz2_ns = cg.esphome_ns.namespace("lwz2")
LWZ2Component = lwz2_ns.class_("LWZ2Component", cg.Component, uart.UARTDevice)


def temp_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def percent_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def airflow_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement="m³/h",
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def hours_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_HOUR,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    )


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LWZ2Component),
            cv.Optional(CONF_FIRMWARE, default="2.14j"): cv.one_of(
                "2.06", "2.14", "2.14j", lower=False
            ),
            cv.Optional(CONF_STATUS_INTERVAL, default="10s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_HEATING_INTERVAL, default="10s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_FAN_INTERVAL, default="30s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_HISTORY_INTERVAL, default="5min"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_FAN_CONFIG_INTERVAL, default="5min"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_STALE_AFTER, default="30s"): cv.positive_time_period_milliseconds,

            **{cv.Optional(key): sensor.sensor_schema(accuracy_decimals=0, entity_category=ENTITY_CATEGORY_DIAGNOSTIC)
               for key in ("successful_queries", "failed_queries", "timeouts", "retries")},
            cv.Optional("hc1_gradient"): sensor.sensor_schema(accuracy_decimals=1),
            cv.Optional("hc1_low_end"): sensor.sensor_schema(unit_of_measurement="K", accuracy_decimals=1),
            **{cv.Optional(key): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC)
               for key in ("last_success", "config_updated", "firmware_version")},
            cv.Optional(CONF_OUTSIDE_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_FLOW_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_RETURN_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_HOT_GAS_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_DHW_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_FLOW_TEMPERATURE_HC2): temp_sensor_schema(),
            cv.Optional(CONF_INSIDE_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_EVAPORATOR_TEMPERATURE): temp_sensor_schema(),
            cv.Optional(CONF_CONDENSER_TEMPERATURE): temp_sensor_schema(),

            cv.Optional(CONF_SUPPLY_FAN_POWER): percent_sensor_schema(),
            cv.Optional(CONF_EXHAUST_FAN_POWER): percent_sensor_schema(),
            cv.Optional(CONF_MAIN_FAN_POWER): percent_sensor_schema(),
            cv.Optional(CONF_SUPPLY_FAN_SPEED_PERCENT): percent_sensor_schema(),
            cv.Optional(CONF_EXHAUST_FAN_SPEED_PERCENT): percent_sensor_schema(),
            cv.Optional(CONF_MAIN_FAN_SPEED_PERCENT): percent_sensor_schema(),

            cv.Optional(CONF_SUPPLY_FAN_SPEED): sensor.sensor_schema(
                unit_of_measurement="1/s", accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT
            ),
            cv.Optional(CONF_EXHAUST_FAN_SPEED): sensor.sensor_schema(
                unit_of_measurement="1/s", accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT
            ),
            cv.Optional(CONF_SUPPLY_FAN_AIRFLOW_SET): airflow_sensor_schema(),
            cv.Optional(CONF_EXHAUST_FAN_AIRFLOW_SET): airflow_sensor_schema(),
            cv.Optional(CONF_SUPPLY_FAN_SPEED_TARGET): percent_sensor_schema(),
            cv.Optional(CONF_EXHAUST_FAN_SPEED_TARGET): percent_sensor_schema(),

            cv.Optional(CONF_SUPPLY_STAGE1_AIRFLOW): airflow_sensor_schema(),
            cv.Optional(CONF_SUPPLY_STAGE2_AIRFLOW): airflow_sensor_schema(),
            cv.Optional(CONF_SUPPLY_STAGE3_AIRFLOW): airflow_sensor_schema(),
            cv.Optional(CONF_EXHAUST_STAGE1_AIRFLOW): airflow_sensor_schema(),
            cv.Optional(CONF_EXHAUST_STAGE2_AIRFLOW): airflow_sensor_schema(),
            cv.Optional(CONF_EXHAUST_STAGE3_AIRFLOW): airflow_sensor_schema(),

            cv.Optional(CONF_OPERATING_HOURS_1): hours_sensor_schema(),
            cv.Optional(CONF_OPERATING_HOURS_2): hours_sensor_schema(),
            cv.Optional(CONF_HEATING_HOURS): hours_sensor_schema(),
            cv.Optional(CONF_DHW_HOURS): hours_sensor_schema(),
            cv.Optional(CONF_COOLING_HOURS): hours_sensor_schema(),

            cv.Optional(CONF_COMPRESSOR): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_HEATING_CIRCUIT_PUMP): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_DHW_PUMP): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_SOLAR_PUMP): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_BOOSTER_STAGE1): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_BOOSTER_STAGE2): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_DIVERTER_VALVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_HEAT_PIPE_VALVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_HEATING_ACTIVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_DHW_ACTIVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_COMMUNICATION_OK): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),

            cv.Optional(CONF_COMMUNICATION_STATUS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_LAST_ERROR): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_RAW_FB): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_RAW_F4): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_RAW_E8): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_RAW_01): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_RAW_09): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_firmware(config[CONF_FIRMWARE]))
    cg.add(var.set_status_interval(config[CONF_STATUS_INTERVAL].total_milliseconds))
    cg.add(var.set_heating_interval(config[CONF_HEATING_INTERVAL].total_milliseconds))
    cg.add(var.set_fan_interval(config[CONF_FAN_INTERVAL].total_milliseconds))
    cg.add(var.set_history_interval(config[CONF_HISTORY_INTERVAL].total_milliseconds))
    cg.add(var.set_fan_config_interval(config[CONF_FAN_CONFIG_INTERVAL].total_milliseconds))
    cg.add(var.set_stale_after(config[CONF_STALE_AFTER].total_milliseconds))

    sensor_setters = {
        **{key: "set_" + key + "_sensor" for key in
           ("hc1_gradient", "hc1_low_end", "successful_queries", "failed_queries", "timeouts", "retries")},
        CONF_OUTSIDE_TEMPERATURE: "set_outside_temperature_sensor",
        CONF_FLOW_TEMPERATURE: "set_flow_temperature_sensor",
        CONF_RETURN_TEMPERATURE: "set_return_temperature_sensor",
        CONF_HOT_GAS_TEMPERATURE: "set_hot_gas_temperature_sensor",
        CONF_DHW_TEMPERATURE: "set_dhw_temperature_sensor",
        CONF_FLOW_TEMPERATURE_HC2: "set_flow_temperature_hc2_sensor",
        CONF_INSIDE_TEMPERATURE: "set_inside_temperature_sensor",
        CONF_EVAPORATOR_TEMPERATURE: "set_evaporator_temperature_sensor",
        CONF_CONDENSER_TEMPERATURE: "set_condenser_temperature_sensor",
        CONF_SUPPLY_FAN_POWER: "set_supply_fan_power_sensor",
        CONF_EXHAUST_FAN_POWER: "set_exhaust_fan_power_sensor",
        CONF_MAIN_FAN_POWER: "set_main_fan_power_sensor",
        CONF_SUPPLY_FAN_SPEED_PERCENT: "set_supply_fan_speed_percent_sensor",
        CONF_EXHAUST_FAN_SPEED_PERCENT: "set_exhaust_fan_speed_percent_sensor",
        CONF_MAIN_FAN_SPEED_PERCENT: "set_main_fan_speed_percent_sensor",
        CONF_SUPPLY_FAN_SPEED: "set_supply_fan_speed_sensor",
        CONF_EXHAUST_FAN_SPEED: "set_exhaust_fan_speed_sensor",
        CONF_SUPPLY_FAN_AIRFLOW_SET: "set_supply_fan_airflow_set_sensor",
        CONF_EXHAUST_FAN_AIRFLOW_SET: "set_exhaust_fan_airflow_set_sensor",
        CONF_SUPPLY_FAN_SPEED_TARGET: "set_supply_fan_speed_target_sensor",
        CONF_EXHAUST_FAN_SPEED_TARGET: "set_exhaust_fan_speed_target_sensor",
        CONF_SUPPLY_STAGE1_AIRFLOW: "set_supply_stage1_airflow_sensor",
        CONF_SUPPLY_STAGE2_AIRFLOW: "set_supply_stage2_airflow_sensor",
        CONF_SUPPLY_STAGE3_AIRFLOW: "set_supply_stage3_airflow_sensor",
        CONF_EXHAUST_STAGE1_AIRFLOW: "set_exhaust_stage1_airflow_sensor",
        CONF_EXHAUST_STAGE2_AIRFLOW: "set_exhaust_stage2_airflow_sensor",
        CONF_EXHAUST_STAGE3_AIRFLOW: "set_exhaust_stage3_airflow_sensor",
        CONF_OPERATING_HOURS_1: "set_operating_hours_1_sensor",
        CONF_OPERATING_HOURS_2: "set_operating_hours_2_sensor",
        CONF_HEATING_HOURS: "set_heating_hours_sensor",
        CONF_DHW_HOURS: "set_dhw_hours_sensor",
        CONF_COOLING_HOURS: "set_cooling_hours_sensor",
    }
    for key, setter in sensor_setters.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, setter)(sens))

    binary_setters = {
        CONF_COMPRESSOR: "set_compressor_binary_sensor",
        CONF_HEATING_CIRCUIT_PUMP: "set_heating_circuit_pump_binary_sensor",
        CONF_DHW_PUMP: "set_dhw_pump_binary_sensor",
        CONF_SOLAR_PUMP: "set_solar_pump_binary_sensor",
        CONF_BOOSTER_STAGE1: "set_booster_stage1_binary_sensor",
        CONF_BOOSTER_STAGE2: "set_booster_stage2_binary_sensor",
        CONF_DIVERTER_VALVE: "set_diverter_valve_binary_sensor",
        CONF_HEAT_PIPE_VALVE: "set_heat_pipe_valve_binary_sensor",
        CONF_HEATING_ACTIVE: "set_heating_active_binary_sensor",
        CONF_DHW_ACTIVE: "set_dhw_active_binary_sensor",
        CONF_COMMUNICATION_OK: "set_communication_ok_binary_sensor",
    }
    for key, setter in binary_setters.items():
        if key in config:
            bs = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(var, setter)(bs))

    text_setters = {
        **{key: "set_" + key + "_text_sensor" for key in ("last_success", "config_updated", "firmware_version")},
        CONF_COMMUNICATION_STATUS: "set_communication_status_text_sensor",
        CONF_LAST_ERROR: "set_last_error_text_sensor",
        CONF_RAW_FB: "set_raw_fb_text_sensor",
        CONF_RAW_F4: "set_raw_f4_text_sensor",
        CONF_RAW_E8: "set_raw_e8_text_sensor",
        CONF_RAW_01: "set_raw_01_text_sensor",
        CONF_RAW_09: "set_raw_09_text_sensor",
    }
    for key, setter in text_setters.items():
        if key in config:
            ts = await text_sensor.new_text_sensor(config[key])
            cg.add(getattr(var, setter)(ts))
