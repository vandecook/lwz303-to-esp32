// Direct F2 defrost observation is independent of compressor state and request hints.
void v062_tests() {
  const uint32_t previous_millis = fake_millis;
  struct Defrost : Test {
    using LWZ2Component::v06_publish_;
  };
  fake_millis = 1000;
  Defrost t;
  sensor::Sensor raw;
  binary_sensor::BinarySensor active, valid;
  text_sensor::TextSensor state, data, comparison;
  t.set_v04_sensor("defrost_evaporator_raw", &raw);
  t.set_v04_binary("defrost_evaporator_active", &active);
  t.set_v04_binary("defrost_evaporator_valid", &valid);
  t.set_v04_text("defrost_evaporator_status", &state);
  t.set_v04_text("defrost_evaporator_data_status", &data);
  t.set_v04_text("defrost_request_comparison", &comparison);
  t.setup();
  assert(std::isnan(raw.state) && !active.valid && !valid.state);
  auto feed = [&](uint8_t code, uint8_t req=6, uint8_t req2=6) {
    t.parse_frame_(0xF2, frame(0xF2,{req,req2,2,2,0,0,0,0,code}));
  };
  feed(0x30); // No FB observation: still valid, from the direct F2 field.
  assert(raw.state==48 && active.valid && active.state && valid.state);
  assert(state.state=="Ja");
  assert(data.state.find("0x30")!=std::string::npos && comparison.state=="Stimmig mit Betriebsanforderung");
  std::vector<uint8_t> fb(32); fb[20]=0;
  t.parse_frame_(0xFB,frame(0xFB,fb)); // Explicitly stopped compressor does not mask defrost.
  assert(active.state && valid.state);
  feed(0x10,2,2);
  assert(state.state=="Nein");
  assert(raw.state==16 && active.valid && !active.state && valid.state);
  assert(comparison.state=="Stimmig mit Betriebsanforderung");
  t.parse_frame_(0xF2,frame(0xF2,{2,2,0,0,0,0,0,0x30,0x10}));
  assert(!active.state && valid.state); // Only payload byte 8 is the field.
  feed(0x10); assert(!active.state && valid.state && comparison.state.find("Widerspruch:")==0);
  feed(0x30,2,2); assert(active.state && valid.state && comparison.state.find("Widerspruch:")==0);
  feed(0x30,6,2); assert(active.state && valid.state && comparison.state.find("Widerspruch zwischen")==0);
  feed(0x30,77,77); assert(active.state && valid.state && comparison.state.find("Nicht vergleichbar")==0);
  // Unknown combinations are not decoded with an invented bit mask and never mean OFF.
  for (uint8_t code : {0x00,0x11,0x20,0x31,0xFF}) {
    feed(code);
    assert(raw.state==code && !active.valid && !valid.state);
    assert(state.state.find("Unbekannt")==0 && comparison.state.find("Nicht vergleichbar")==0);
  }
  feed(0x30);
  t.parse_frame_(0xF2,frame(0xF2,{6,6,2,2,0,0,0,0}));
  assert(raw.state==48 && active.valid && active.state && !valid.state);
  assert(data.state.find("Rohbyte fehlt")!=std::string::npos);
  feed(0x30);
  fake_millis+=30001; t.v06_publish_();
  assert(active.valid && active.state && raw.state==48 && !valid.state);
  assert(data.state.find("veraltet")!=std::string::npos && comparison.state.find("Nicht vergleichbar")==0);
  t.parse_frame_(0xFB,frame(0xFB,fb)); assert(!valid.state);
  feed(0x10,0,0); assert(active.valid && !active.state && valid.state);
  feed(0x30); t.reinitialize();
  assert(active.valid && active.state && raw.state==48 && !valid.state);
  t.set_firmware("2.16");
  assert(!active.valid && std::isnan(raw.state) && !valid.state);
  for (const char *profile : {"2.06","2.14","2.14j","2.16"}) {
    t.set_firmware(profile); feed(0x30);
    assert(active.valid && active.state && valid.state && raw.state==48);
  }
  assert(t.tx.empty()); // New diagnostics do not send any commands or change the device.
  fake_millis=0xfffffff0U; feed(0x30); fake_millis=20; t.v06_publish_();
  assert(valid.state && active.state);
  fake_millis+=30001; t.v06_publish_(); assert(!valid.state && active.state);
  fake_millis = previous_millis;
  std::cout << "PASS v0.6.2: direct F2 defrost/raw, compressor-independent state, conflicts/unknowns, gaps/recovery/reinit/profiles/wrap\n";
}
