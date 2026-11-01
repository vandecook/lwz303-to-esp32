# ESPHome LWZ 2.x read-only component — V0.1

Initial read-only ESPHome external component for the older Stiebel Eltron / Tecalor LWZ/THZ 2.x protocol family.

## Supported firmware profiles

- 2.06
- 2.14
- 2.14j

No 4.x or 5.x support is included by design.

## Scope of V0.1

Read-only polling of these blocks:

| Block | Purpose | Default interval |
|---|---|---:|
| `FB` | Common 2.x live values and compressor/pump state | 5 s |
| `F4` | Heating-circuit data; firmware-specific layout | 10 s |
| `E8` | Detailed fan state | 30 s |
| `01` | Fan configuration / airflow stages | 5 min |
| `09` | Operating-hour counters | 5 min |

The component uses a non-blocking communication state machine so UART timeouts do not deliberately block ESPHome/Wi-Fi/OTA for seconds at a time.

## Communication recovery

- Wakeup/ACK, command ACK and response frame each have a timeout.
- A failed transaction is retried up to three times.
- After repeated failures, requests are backed off (10 s, then 30 s).
- Numeric live values are published as `NaN` after `stale_after` (default 30 s).
- Binary process states are **not forced to OFF** on a communication failure. Use `Communication OK` / `Communication Status` to determine whether their last state is trustworthy.
- A successful frame automatically returns the component to normal polling.

## Firmware selection

The example YAML exposes `LWZ Firmware Profile` as a persistent ESPHome template select. It appears in Web Server v3 and Home Assistant.

Changing the profile calls `set_firmware()` at runtime, clears the serial state and immediately starts a fresh poll sequence.

The mappings are intentionally firmware-aware:

- 2.06: `FBglob206`, `F4hc1`, `01pxx206`
- 2.14: `FBglob214`, `F4hc1214`, `01pxx214`
- 2.14j: `FBglob214`, `F4hc1214j`, `01pxx214`
- all three: `E8fan206`, `09his206`

## Heating / DHW status

For V0.1:

- `Heating Active` follows the **heating circuit pump** bit.
- `DHW Active` follows the **DHW pump** bit.

This is deliberately conservative. It avoids incorrectly treating `compressor = ON` as space heating while the compressor is actually producing DHW.

`FB` is polled every 5 seconds because it contains these state bits across the supported 2.x profiles. Firmware 2.14j additionally exposes comparable actuator bits in its special F4 layout.

## Fan values

Three different kinds of fan information are kept separate:

1. `FB`: live fan power / speed percentages.
2. `E8`: detailed technical fan values such as actual speed, airflow setpoint and speed target.
3. `01`: configured supply/exhaust airflow for fan stages 1..3.

The `01` layout differs materially between 2.06 and 2.14/2.14j, which is why the selected firmware profile matters.

## Serial connection

The legacy interface documentation describes 9600 baud, 8 data bits, no parity, 1 stop bit. The example uses that setting.

**Do not connect RS232 voltage levels directly to an ESP32.** Use a MAX3232 or equivalent RS232-to-3.3-V-TTL transceiver.

Typical ESP32 example:

- TX: GPIO17
- RX: GPIO16
- Logger serial output disabled (`logger.baud_rate: 0`)

Adjust pins to your board.

## Installation

Place the directory next to the ESPHome YAML:

```text
<config>/
├── lwz303-sol-example.yaml
└── components/
    └── lwz2/
        ├── __init__.py
        ├── lwz2.h
        └── lwz2.cpp
```

The example loads it with:

```yaml
external_components:
  - source: components
    components: [lwz2]
```

## Commissioning recommendation

For the first connection set:

```yaml
logger:
  level: VERBOSE
  baud_rate: 0
```

Every successfully decoded frame is logged as hex at VERBOSE level. Optional `raw_*` text sensors are also included in the component schema but commented out in the example because a 5-second raw FB entity creates unnecessary Home Assistant history churn.

## Status of this version

V0.1 is based on the supplied FHEM `00_THZ.pm` protocol/parser definitions and the LWZ DLE handshake reference. It has been structured for real-device commissioning but has **not yet been verified against the user's physical LWZ 303 SOL / FW 2.14j**. Raw-frame comparison during the first test is therefore intentional.
