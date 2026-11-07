"""Compile the actual component against a small fake ESPHome/UART runtime.

Usage: python tests/run_host_tests.py --compiler g++
   or: python tests/run_host_tests.py --compiler path/to/zig.exe --zig
No hardware or Python packages required. Build files stay in .validation/host.
"""
import argparse
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / '.validation' / 'host'

STUB = r'''
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <cstring>
namespace esphome {
inline uint32_t fake_millis = 100;
inline uint32_t millis() { return fake_millis; }
namespace setup_priority { constexpr float DATA = 600; }
class Component { public: virtual void setup() {} virtual void loop() {}
virtual void dump_config() {} virtual float get_setup_priority() const { return 0; } };
namespace sensor { class Sensor { public: float state = 0; void publish_state(float v) { state=v; } }; }
namespace binary_sensor { class BinarySensor { public: bool state=false; bool valid=false; void publish_state(bool v) { state=v; valid=true; } void invalidate_state() {valid=false;} }; }
namespace number { class Number { public: float state=0; void publish_state(float v) {state=v;} virtual void control(float)=0; }; }
namespace select { class Select { public: std::string state; void publish_state(const std::string &v) {state=v;} protected: virtual void control(const std::string &) {} }; }
inline std::map<uint32_t,std::vector<uint8_t>> fake_storage;
inline bool fake_storage_ok=true;
class ESPPreferenceObject { public: uint32_t key=0;
 template<typename T> bool save(const T *s) { if(!fake_storage_ok)return false; auto p=reinterpret_cast<const uint8_t*>(s);fake_storage[key]={p,p+sizeof(T)};return true; }
 template<typename T> bool load(T *s) {auto i=fake_storage.find(key);if(i==fake_storage.end()||i->second.size()!=sizeof(T))return false;std::memcpy(s,i->second.data(),sizeof(T));return true;}
};
class ESPPreferences { public: template<typename T> ESPPreferenceObject make_preference(uint32_t k) {ESPPreferenceObject p;p.key=k;return p;} bool sync(){return fake_storage_ok;} };
inline ESPPreferences fake_preferences;
inline ESPPreferences *global_preferences=&fake_preferences;
namespace text_sensor { class TextSensor { public: std::string state; void publish_state(const std::string &v) { state=v; } }; }
namespace uart {
class UARTComponent { public: int reloads=0; void load_settings(bool = true) { reloads++; } };
class UARTDevice { public:
 UARTComponent *parent_{nullptr}; std::deque<uint8_t> rx; std::vector<uint8_t> tx;
 size_t available() { return rx.size(); }
 bool read_byte(uint8_t *v) { if(rx.empty()) return false; *v=rx.front(); rx.pop_front(); return true; }
 void write_byte(uint8_t b) { tx.push_back(b); }
}; }
}
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGV(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGCONFIG(...) ((void)0)
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--zig', action='store_true')
    args = parser.parse_args()
    for path in ['core/component.h', 'core/log.h', 'components/uart/uart.h',
                 'components/sensor/sensor.h', 'components/binary_sensor/binary_sensor.h',
                 'components/text_sensor/text_sensor.h', 'components/number/number.h',
                 'components/select/select.h', 'core/preferences.h']:
        p = BUILD / 'esphome' / path
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text('#include "runtime.h"\n')
    (BUILD / 'runtime.h').write_text(STUB)
    exe = BUILD / 'host_tests.exe'
    command = [args.compiler] + (['c++'] if args.zig else [])
    command += ['-std=c++17', '-I'+str(BUILD), '-I'+str(ROOT/'components/lwz2'),
                str(ROOT/'components/lwz2/lwz2.cpp'), str(ROOT/'components/lwz2/v03.cpp'), str(ROOT/'components/lwz2/v04.cpp'),
                str(ROOT/'tests/test_component.cpp'), '-o', str(exe)]
    subprocess.run(command, check=True, cwd=ROOT)
    subprocess.run([str(exe)], check=True, cwd=ROOT)

if __name__ == '__main__':
    main()
