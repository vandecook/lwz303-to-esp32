#include "lwz2.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace esphome;
using namespace esphome::lwz2;

struct Test : LWZ2Component {
  using LWZ2Component::validate_frame_;
  using LWZ2Component::deescape_2b18_;
  using LWZ2Component::parse_fb_;
  using LWZ2Component::parse_frame_;
  using LWZ2Component::current_command_;
  using LWZ2Component::state_;
  using LWZ2Component::successes_;
  using LWZ2Component::timeouts_;
  using LWZ2Component::retries_;
  using LWZ2Component::total_errors_;
  using LWZ2Component::next_fb_ms_;
  using LWZ2Component::next_config_ms_;
  using LWZ2Component::config_pending_;
  using LWZ2Component::firmware_pending_;
  using LWZ2Component::last_fb_success_ms_;
  using LWZ2Component::consecutive_errors_;
  using LWZ2Component::mark_success_;
  using LWZ2Component::update_communication_state_;
  using LWZ2Component::next_allowed_request_ms_;
  using LWZ2Component::stale_published_;
  using LWZ2Component::time_reached_;
  void inject(std::initializer_list<uint8_t> bytes) { rx.insert(rx.end(),bytes); loop(); }
};

std::vector<uint8_t> frame(uint8_t command, std::vector<uint8_t> data) {
  std::vector<uint8_t> result{1,0,0,command};
  result.insert(result.end(),data.begin(),data.end());
  for (size_t i=0;i<result.size();i++) if(i!=2) result[2]+=result[i];
  return result;
}

#include "test_v03.h"

int main() {
  // Decoder fixtures are derived from the documented FHEM offsets, not hardware captures.
  for (const auto *profile : {"2.06", "2.14", "2.14j"}) {
    Test t; sensor::Sensor outside,flow,ret,dhw,gradient,low;
    binary_sensor::BinarySensor compressor,heating,water;
    t.set_firmware(profile);
    t.set_outside_temperature_sensor(&outside); t.set_flow_temperature_sensor(&flow);
    t.set_return_temperature_sensor(&ret); t.set_dhw_temperature_sensor(&dhw);
    t.set_hc1_gradient_sensor(&gradient); t.set_hc1_low_end_sensor(&low);
    t.set_compressor_binary_sensor(&compressor); t.set_heating_active_binary_sensor(&heating);
    t.set_dhw_active_binary_sensor(&water);
    std::vector<uint8_t> data(21);
    data[2]=0xff;data[3]=0xce; // -5.0 C
    data[4]=1;data[5]=0x2c; // 30.0 C
    data[6]=0;data[7]=250;data[10]=1;data[11]=0xc2; // 25,45 C
    data[20]=0x13; // compressor + both pumps
    t.current_command_=0xFB; std::string reason;
    auto fb=frame(0xFB,data); assert(t.validate_frame_(fb,reason)); t.parse_frame_(0xFB,fb);
    assert(outside.state==-5 && flow.state==30 && ret.state==25 && dhw.state==45);
    assert(compressor.state && heating.state && water.state);
    assert(t.effective_interval()==5000);
    // Each independent trigger suffices; disabled dynamic mode overrides all.
    for (int mask=0;mask<8;mask++) {
      t.set_trigger_compressor(mask&1);t.set_trigger_heating(mask&2);t.set_trigger_dhw(mask&4);
      assert(t.effective_interval()==(mask ? 5000u : 10000u));
    }
    t.set_dynamic_enabled(false); assert(t.effective_interval()==10000);
    t.current_command_=5; auto cfg=frame(5,{0,4,0,25});
    assert(t.validate_frame_(cfg,reason));t.parse_frame_(5,cfg);
    assert(std::abs(gradient.state-0.4f)<0.001f && low.state==2.5f);
    cfg=frame(5,{0,4});assert(!t.validate_frame_(cfg,reason));
    cfg=frame(5,{0,4,0,25});cfg[2]++;assert(!t.validate_frame_(cfg,reason));
    cfg=frame(0xFB,data);assert(!t.validate_frame_(cfg,reason));
    t.current_command_=0xFB;assert(!t.validate_frame_(frame(0xFB,{0,0}),reason));
    assert(t.deescape_2b18_({0x2b,0x18,0x42})==std::vector<uint8_t>({0x2b,0x42}));
  }
  // Full wakeup/ACK/escaped-frame transaction and no extra polls from observers.
  Test t; text_sensor::TextSensor updated; sensor::Sensor gradient,success;
  t.set_config_updated_text_sensor(&updated);t.set_hc1_gradient_sensor(&gradient);
  t.set_successful_queries_sensor(&success);t.setup();t.loop();
  assert(t.current_command_==5 && t.tx==std::vector<uint8_t>({2}));
  t.inject({0x10});
  assert(t.tx==std::vector<uint8_t>({2,1,0,6,5,0x10,3})); // read-only 05 request
  t.inject({0x10,2});
  auto cfg=frame(5,{0,4,0,0x10});
  for(auto b:cfg) { t.rx.push_back(b);if(b==0x10)t.rx.push_back(b); }
  t.rx.push_back(0x10);t.rx.push_back(3);t.loop();
  assert(t.successes_==1 && updated.state=="boot+0s");
  auto count=t.tx.size(); t.loop();assert(t.tx.size()==count);
  // Timeout -> two retries -> one final failure, without blocking or resetting recovery on counter reset.
  fake_millis+=10000;t.loop();assert(t.current_command_==0xFB);
  for(int attempt=0;attempt<3;attempt++) {
    fake_millis+=1801;t.loop();
    if(attempt<2) {fake_millis+=251;t.loop();}
  }
  assert(t.timeouts_==3 && t.retries_==2 && t.total_errors_==1);
  auto errors=t.consecutive_errors_;t.reset_counters();
  assert(t.total_errors_==0 && t.successes_==0 && t.consecutive_errors_==errors);
  t.reinitialize();assert(t.state_==CommState::IDLE && t.config_pending_);
  uart::UARTComponent uart;t.parent_=&uart;t.reinitialize();assert(uart.reloads==1);
  // Wraparound-safe scheduling.
  assert(!Test::time_reached_(0xfffffff0,0x10));assert(Test::time_reached_(0x11,0x10));
  // Firmware probing is restricted to 2.06; selecting 2.14j never sends FD.
  Test f;f.set_firmware("2.06");f.setup();f.config_pending_=false;f.next_config_ms_=fake_millis+100000;
  f.loop();assert(f.current_command_==0xFD);
  // A recent configuration success cannot recover expired live values.
  Test stale;binary_sensor::BinarySensor ok;sensor::Sensor temp;
  stale.set_communication_ok_binary_sensor(&ok);stale.set_outside_temperature_sensor(&temp);
  stale.setup();stale.current_command_=0xFB;stale.mark_success_();assert(ok.state);
  fake_millis+=31000;stale.update_communication_state_();assert(!ok.state && std::isnan(temp.state));
  stale.current_command_=5;stale.mark_success_();assert(!ok.state && stale.stale_published_);
  stale.current_command_=0xFB;stale.mark_success_();assert(ok.state);
  // Disabling legacy consumers leaves only configuration + FB (no F4/E8/01/09).
  Test minimal;minimal.setup();minimal.config_pending_=false;
  minimal.next_config_ms_=fake_millis+100000;minimal.next_fb_ms_=fake_millis+100000;
  fake_millis+=5000;minimal.loop();assert(minimal.tx.empty());
  // Oversized unterminated frames are rejected and enter the existing retry path.
  Test oversized;oversized.setup();oversized.loop();oversized.inject({0x10});oversized.inject({2});
  oversized.rx.insert(oversized.rx.end(),520,0);oversized.loop();oversized.loop();
  assert(oversized.retries_==1 && oversized.state_==CommState::RETRY_WAIT);
  v03_tests();
  std::cout << "PASS: 3 profiles, offsets, OR polling, validation, transport, retries, reset, wraparound, firmware gate\n";
}
