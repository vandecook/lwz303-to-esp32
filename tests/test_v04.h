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
  {
    V04 probe; sensor::Sensor shown; text_sensor::TextSensor details, data;
      probe.set_firmware("2.06");
    probe.set_preferences_key(777); probe.set_v04_sensor("current_fan_stage",&shown);
    probe.set_v04_text("fan_stage_diagnostics",&details); probe.set_v04_text("exhaust_data_status",&data);
    probe.setup(); assert(std::isnan(shown.state) && details.state.find("EE=fehlt")!=std::string::npos);
    feed(probe); assert(shown.state==1 && probe.current_fan_stage_()==1);
    fake_millis+=31000; probe.v04_tick_();
    assert(shown.state==1 && probe.current_fan_stage_()==-1 && data.state=="Unbekannt");
    assert(details.state.find("veraltet/31s")!=std::string::npos && details.state.find("unbestaetigt")!=std::string::npos);
    fake_millis+=120000; probe.v04_tick_(); assert(shown.state==1 && !probe.saved_.fans[0].active);
    // Refreshing only EE cannot validate missing F6/settings or restart a fault interval.
    std::vector<uint8_t> ee(19); ee[0]=1;ee[5]=1;
    probe.parse_frame_(0xEE,frame(0xEE,ee)); assert(shown.state==1 && probe.current_fan_stage_()==-1);
    assert(details.state.find("EE=frisch/0s")!=std::string::npos);
    feed(probe,2); assert(shown.state==0 && probe.current_fan_stage_()==0);
    feed(probe,9); assert(shown.state==0 && probe.current_fan_stage_()==-1);
    assert(details.state.find("ProgFAN=9")!=std::string::npos && details.state.find("unbestaetigt")!=std::string::npos);
    probe.set_firmware("2.16"); assert(std::isnan(shown.state)); // Clear when decoder is changed.
  }
  fake_storage.clear(); fake_millis=1000;
  V04 t;t.set_device_model("SOL");
  binary_sensor::BinarySensor f5,f6;t.set_v04_binary("exhaust_fault",&f5);t.set_v04_binary("supply_fault",&f6);
  t.setup();feed(t);
  t.set_status_interval(300000);assert(t.effective_interval()==10000);t.set_status_interval(10000);
  for(int i=1;i<12;i++){fake_millis+=10000;feed(t);assert(!f5.state);}
  fake_millis+=9999;feed(t);assert(!f5.state);
  fake_millis+=1;feed(t);assert(f5.state && !f6.state && t.saved_.fans[0].count==1);
  fake_millis+=10000;feed(t,1,1,1,0,18,32);assert(f5.state); // Latch even after recovery.
  fake_millis+=10000;feed(t,2);assert(f5.state && t.current_fan_stage_()==0);
  // ESP reboot preserves latch and history, but never continues a pending timer.
  V04 reboot;reboot.setup();assert(reboot.saved_.fans[0].active && reboot.saved_.fans[0].count==1);
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
  // P59 restart before setback end (ProgStateFAN 4/5): early running is OK, standstill no fault.
  {
    text_sensor::TextSensor reason, ex_status, sup_status;
    t.set_v04_text("fan_run_reason",&reason);t.set_v04_text("exhaust_status",&ex_status);
    t.set_v04_text("supply_status",&sup_status);
    feed(t,1);assert(reason.state=="Zeitprogramm: Normalbetrieb (P07)");
    feed(t,2);assert(reason.state=="Zeitprogramm: Absenkbetrieb (P08)" && ex_status.state=="Aus");
    assert(sup_status.state=="Laeuft ohne Anforderung");
    for (int code : {4,5}) {
      t.reset_fan_faults();
      for (int i=0;i<15;i++) {fake_millis+=10000;feed(t,code,1,1,0,0,32);}
      assert(!f5.state && !f6.state && t.current_fan_stage_()==-1);
      assert(reason.state=="Wiederanlauf vor Absenkende (P59)");
      assert(ex_status.state=="Aus (Wiederanlauf P59 moeglich)" && sup_status.state=="Laeuft (Wiederanlauf P59)");
    }
    feed(t,4,0);assert(reason.state=="Handbetrieb (P12)");
    feed(t,4,1,1,0,0,0,2,10);assert(reason.state=="Zeitweise Uebersteuerung");
    feed(t,9);assert(reason.state=="Unbekannt (Programmcode 9)");
    fake_millis+=31000;t.v04_tick_();assert(reason.state=="Unbekannt (Programmzustand nicht frisch)");
    t.reset_fan_faults();
  }
  // Alternative E8 source: source change abandons the previous timer.
  t.set_fan_speed_source("sFan");feed(t);t.v04_tick_();
  std::vector<uint8_t> e8(22);e8[11]=50;e8[13]=40;
  t.parse_frame_(0xE8,frame(0xE8,e8));assert(!f5.state && !f6.state);
  // Raw F2 fields independent of global profile, labels only use documented SOL table.
  sensor::Sensor hc,dhw;text_sensor::TextSensor ht,dt;
  t.set_v04_sensor("hc_stage",&hc);t.set_v04_sensor("dhw_stage",&dhw);
  t.set_v04_text("hc_stage_text",&ht);t.set_v04_text("dhw_stage_text",&dt);
  for(auto profile:{"2.06","2.14","2.14j","2.16"}) {
    t.set_firmware(profile);t.parse_frame_(0xF2,frame(0xF2,{2,2,4,3,4,0}));
    assert(hc.state==4 && dhw.state==3 && ht.state.find("2 angefordert")!=std::string::npos);
    assert(dt.state.find("Maximale")!=std::string::npos);
    const char *hc_labels[]{"Aus", "Solar angefordert", "Verdichter angefordert",
      "Nacherwaermstufe 1 angefordert", "Nacherwaermstufe 2 angefordert", "Nacherwaermstufe 3 angefordert"};
    for (uint8_t code=0;code<=6;code++) {
      t.parse_frame_(0xF2,frame(0xF2,{0,0,code,code,0,0}));
      assert(ht.state==(code<6 ? hc_labels[code] : "Rohcode 6"));
      assert(dt.state==(code<3 ? hc_labels[code] : code==3 ?
        "Maximale freigegebene Nacherwaermung angefordert" : "Rohcode " + std::to_string(code)));
    }
  }
  t.set_device_model("i");t.parse_frame_(0xF2,frame(0xF2,{2,2,4,3,4,0}));assert(ht.state=="Rohcode 4");
  // P07/P08: only one byte changes, integer 0..3, no optimistic confirmation.
  for(unsigned i=0;i<2;i++) {
    V04 w;w.set_preferences_key(20+i);w.setup();w.set_writes_enabled(true);
    assert(!w.request_fan_stage(i,1.5f));assert(!w.request_fan_stage(i,4));assert(w.request_fan_stage(i,2));
    std::vector<uint8_t> p(20,0x10);w.write_response_(frame(0x17,p));
    auto expected=p;expected[12+i]=2;assert(w.write_expected_==expected);
    w.write_response_(frame(0x17,{}));w.write_response_(frame(0x17,expected));
    assert(w.write_stage_==V04::WriteStage::NONE);
  }
  // Seasonal selector: persist uncertainty before SET, never replay across reboot.
  fake_millis=1000000;
  V04 clock;clock.set_preferences_key(100);select::Select mode;
  clock.set_clock_mode_select(&mode);clock.setup();clock.set_writes_enabled(true);
  assert(mode.state=="Winterzeit" && clock.request_clock_mode("Winterzeit"));
  assert(clock.write_stage_==V04::WriteStage::NONE);
  assert(clock.request_clock_mode("Sommerzeit"));
  const auto fc=frame(0xFC,{0,4,12,0,0,26,0,11,6});clock.write_response_(fc);
  assert(mode.state=="Unbekannt");
  V04 interrupted;interrupted.set_preferences_key(100);interrupted.setup();
  assert(interrupted.saved_.clock_mode==-1 && !interrupted.request_clock_mode("Sommerzeit"));
  clock.write_response_(frame(0xFC,{}));clock.write_response_(frame(0xFC,{0,4,13,0,0,26,0,11,6}));
  assert(mode.state=="Sommerzeit" && clock.saved_.clock_mode==1);
  assert(clock.request_clock_mode("Sommerzeit") && clock.write_stage_==V04::WriteStage::NONE);
  V04 restored;restored.set_preferences_key(100);restored.setup();assert(restored.saved_.clock_mode==1);
  // Calendar crossing is rejected before changing saved seasonal metadata.
  fake_millis+=31000;assert(clock.request_clock_mode("Winterzeit"));
  clock.write_response_(frame(0xFC,{0,4,0,5,0,26,0,11,6}));assert(clock.saved_.clock_mode==1);
  // Storage failure must reject seasonal SET before it can reach the device.
  fake_millis+=31000;assert(clock.request_clock_mode("Winterzeit"));fake_storage_ok=false;
  clock.write_response_(fc);assert(clock.write_stage_==V04::WriteStage::NONE);
  fake_storage_ok=true;
  // Monotonic wrap does not shorten a 120s window.
  fake_millis=0xffff0000U;V04 wrap;wrap.set_preferences_key(200);wrap.setup();feed(wrap);
  for(int i=0;i<12;i++){fake_millis+=10000;feed(wrap);}
  assert(wrap.saved_.fans[0].active);
  std::cout << "PASS v0.4: F5/F6 timing/latches/gaps/history, program/override, F2 models, fan writes, clock persistence/wrap\n";
}
