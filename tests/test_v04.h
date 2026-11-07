// Synthetic protocol fixtures; no hardware writes or hardware claims.
void v04_tests() {
  struct V04 : Test {
    using LWZ2Component::v04_tick_;
    using LWZ2Component::saved_;
    using LWZ2Component::current_fan_stage_;
    using LWZ2Component::WriteStage;
    using LWZ2Component::write_stage_;
    using LWZ2Component::write_expected_;
    using LWZ2Component::write_response_;
    using LWZ2Component::write_failed_;
  };
  auto feed = [](V04 &t, int program=1, int mode=1, int day=1, int night=0, int exhaust=0, int supply=32, int override_stage=0, int remaining=0) {
    std::vector<uint8_t> p(20);p[12]=day;p[13]=night;p[14]=0;p[19]=3;
    std::vector<uint8_t> ee(19);ee[0]=mode;ee[5]=program;
    std::vector<uint8_t> f6(18);f6[13]=override_stage;f6[16]=remaining>>8;f6[17]=remaining&255;
    std::vector<uint8_t> fb(32);fb[26]=exhaust;fb[27]=supply;
    t.parse_frame_(0x17,frame(0x17,p));t.parse_frame_(0xEE,frame(0xEE,ee));
    t.parse_frame_(0xF6,frame(0xF6,f6));
    t.parse_frame_(0xFC,frame(0xFC,{0,4,12,0,0,26,0,11,6}));
    t.parse_frame_(0xFB,frame(0xFB,fb));
  };
  fake_storage.clear(); fake_millis=1000;
  V04 t;t.set_v04_enabled(true);t.set_v03_enabled(true);t.set_device_model("SOL");
  binary_sensor::BinarySensor f5,f6;t.set_v04_binary("exhaust_fault",&f5);t.set_v04_binary("supply_fault",&f6);
  t.setup();feed(t);
  t.set_status_interval(300000);assert(t.effective_interval()==10000);t.set_status_interval(10000);
  for(int i=1;i<12;i++){fake_millis+=10000;feed(t);assert(!f5.state);}
  fake_millis+=9999;feed(t);assert(!f5.state);
  fake_millis+=1;feed(t);assert(f5.state && !f6.state && t.saved_.fans[0].count==1);
  fake_millis+=10000;feed(t,1,1,1,0,18,32);assert(f5.state); // Latch even after recovery.
  fake_millis+=10000;feed(t,2);assert(f5.state && t.current_fan_stage_()==0);
  // ESP reboot preserves latch and history, but never continues a pending timer.
  V04 reboot;reboot.set_v04_enabled(true);reboot.setup();assert(reboot.saved_.fans[0].active && reboot.saved_.fans[0].count==1);
  t.reset_fan_faults();assert(!f5.state && t.saved_.fans[0].count==1 && t.current_fan_stage_()==-1);
  feed(t,1,1,1,0,18,0);
  for(int i=0;i<12;i++){fake_millis+=10000;feed(t,1,1,1,0,18,0);}
  assert(f6.state && !f5.state && t.saved_.fans[1].count==1);
  // Old data/gaps never accumulate into an error; restart detector does not invent a reset.
  t.reset_fan_faults();feed(t);fake_millis+=40000;feed(t);
  for(int i=0;i<11;i++){fake_millis+=10000;feed(t);}
  assert(!f5.state);fake_millis+=10000;feed(t);assert(f5.state && t.saved_.fans[0].count==2);
  // Night OFF, manual mode and timed overrides determine the actual requested stage.
  t.reset_fan_faults();feed(t,2);assert(t.current_fan_stage_()==0);
  feed(t,2,0);assert(t.current_fan_stage_()==3);
  feed(t,2,1,1,0,0,0,2,10);assert(t.current_fan_stage_()==2);
  feed(t,9);assert(t.current_fan_stage_()==-1);
  for (int i=0;i<15;i++) {fake_millis+=10000;feed(t,2,1,1,0,0,0);}
  assert(!f5.state && !f6.state); // Planned night OFF can last indefinitely.
  // Alternative E8 source: source change abandons the previous timer.
  t.set_fan_speed_source("sFan");feed(t);t.v04_tick_();
  std::vector<uint8_t> e8(22);e8[11]=50;e8[13]=40;
  t.parse_frame_(0xE8,frame(0xE8,e8));assert(!f5.state && !f6.state);
  // Raw F2 fields independent of global profile, labels only use documented SOL table.
  sensor::Sensor hc,dhw;text_sensor::TextSensor ht,dt;
  t.set_v04_sensor("hc_stage",&hc);t.set_v04_sensor("dhw_stage",&dhw);
  t.set_v04_text("hc_stage_text",&ht);t.set_v04_text("dhw_stage_text",&dt);
  for(auto profile:{"2.06","2.14","2.14j"}) {
    t.set_firmware(profile);t.parse_frame_(0xF2,frame(0xF2,{2,2,4,3,4,0}));
    assert(hc.state==4 && dhw.state==3 && ht.state.find("2 angefordert")!=std::string::npos);
    assert(dt.state.find("Maximale")!=std::string::npos);
  }
  t.set_device_model("i");t.parse_frame_(0xF2,frame(0xF2,{2,2,4,3,4,0}));assert(ht.state=="Rohcode 4");
  // P07/P08: only one byte changes, integer 0..3, no optimistic confirmation.
  for(unsigned i=0;i<2;i++) {
    V04 w;w.set_v04_enabled(true);w.set_preferences_key(20+i);w.setup();w.set_writes_enabled(true);
    assert(!w.request_fan_stage(i,1.5f));assert(!w.request_fan_stage(i,4));assert(w.request_fan_stage(i,2));
    std::vector<uint8_t> p(20,0x10);w.write_response_(frame(0x17,p));
    auto expected=p;expected[12+i]=2;assert(w.write_expected_==expected);
    w.write_response_(frame(0x17,{}));w.write_response_(frame(0x17,expected));
    assert(w.write_stage_==V04::WriteStage::NONE);
  }
  // Seasonal selector: persist uncertainty before SET, never replay across reboot.
  fake_millis=1000000;
  V04 clock;clock.set_v04_enabled(true);clock.set_preferences_key(100);select::Select mode;
  clock.set_clock_mode_select(&mode);clock.setup();clock.set_writes_enabled(true);
  assert(mode.state=="Winterzeit" && clock.request_clock_mode("Winterzeit"));
  assert(clock.write_stage_==V04::WriteStage::NONE);
  assert(clock.request_clock_mode("Sommerzeit"));
  const auto fc=frame(0xFC,{0,4,12,0,0,26,0,11,6});clock.write_response_(fc);
  assert(mode.state=="Unbekannt");
  V04 interrupted;interrupted.set_v04_enabled(true);interrupted.set_preferences_key(100);interrupted.setup();
  assert(interrupted.saved_.clock_mode==-1 && !interrupted.request_clock_mode("Sommerzeit"));
  clock.write_response_(frame(0xFC,{}));clock.write_response_(frame(0xFC,{0,4,13,0,0,26,0,11,6}));
  assert(mode.state=="Sommerzeit" && clock.saved_.clock_mode==1);
  assert(clock.request_clock_mode("Sommerzeit") && clock.write_stage_==V04::WriteStage::NONE);
  V04 restored;restored.set_v04_enabled(true);restored.set_preferences_key(100);restored.setup();assert(restored.saved_.clock_mode==1);
  // Calendar crossing is rejected before changing saved seasonal metadata.
  fake_millis+=31000;assert(clock.request_clock_mode("Winterzeit"));
  clock.write_response_(frame(0xFC,{0,4,0,5,0,26,0,11,6}));assert(clock.saved_.clock_mode==1);
  // Storage failure must reject seasonal SET before it can reach the device.
  fake_millis+=31000;assert(clock.request_clock_mode("Winterzeit"));fake_storage_ok=false;
  clock.write_response_(fc);assert(clock.write_stage_==V04::WriteStage::NONE);
  fake_storage_ok=true;
  // Monotonic wrap does not shorten a 120s window.
  fake_millis=0xffff0000U;V04 wrap;wrap.set_v04_enabled(true);wrap.set_preferences_key(200);wrap.setup();feed(wrap);
  for(int i=0;i<12;i++){fake_millis+=10000;feed(wrap);}
  assert(wrap.saved_.fans[0].active);
  std::cout << "PASS v0.4: F5/F6 timing/latches/gaps/history, program/override, F2 models, fan writes, clock persistence/wrap\n";
}
