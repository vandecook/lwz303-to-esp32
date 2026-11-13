// v0.6: held display values are never evidence of fresh communication.
void v06_tests() {
  struct V06 : Test {
    using LWZ2Component::v03_stale_;
    using LWZ2Component::v06_publish_;
    using LWZ2Component::block_fresh_;
    using LWZ2Component::block_stale_;
    using LWZ2Component::profile_fresh_;
  };
  fake_millis = 1000;
  V06 t;
  sensor::Sensor temp, h1, h2, heat, dhw, stage;
  binary_sensor::BinarySensor compressor, fb_fresh, f2_fresh, purpose_valid;
  text_sensor::TextSensor quality, purpose, status, demand;
  t.set_device_model("SOL");
  t.set_outside_temperature_sensor(&temp);
  t.set_compressor_binary_sensor(&compressor);
  t.set_operating_hours_1_sensor(&h1); t.set_operating_hours_2_sensor(&h2);
  t.set_heating_hours_sensor(&heat); t.set_dhw_hours_sensor(&dhw);
  t.set_v04_sensor("hc_stage", &stage);
  t.set_v04_binary("block_FB_fresh", &fb_fresh);
  t.set_v04_binary("block_F2_fresh", &f2_fresh);
  t.set_v04_binary("compressor_purpose_valid", &purpose_valid);
  t.set_v04_text("data_quality", &quality);
  t.set_v04_text("compressor_purpose", &purpose);
  t.set_v04_text("compressor_purpose_data_status", &status);
  t.set_v04_text("simultaneous_demand", &demand);
  t.setup();
  assert(std::isnan(temp.state) && !fb_fresh.state && !purpose_valid.state);
  assert(quality.state.find("FB=fehlt") != std::string::npos);
  std::vector<uint8_t> fb(32); fb[2]=0; fb[3]=123; fb[20]=0x10;
  auto running = [&]() { t.parse_frame_(0xFB, frame(0xFB, fb)); };
  auto request = [&](uint8_t a, uint8_t b) { t.parse_frame_(0xF2, frame(0xF2,{a,b,2,2,0,0})); };
  running(); request(2,2);
  assert(temp.state==12.3f && compressor.valid && compressor.state && fb_fresh.state);
  assert(purpose.state=="Heizung (vorlaeufig)" && purpose_valid.state);
  assert(demand.state=="Heizung und Warmwasser angefordert");
  request(0,0); assert(purpose.state=="Warmwasser (vorlaeufig)");
  request(6,6); assert(purpose.state=="Abtauen (vorlaeufig)");
  request(0,2); assert(!purpose_valid.state && purpose.state=="Nicht eindeutig zuordenbar");
  request(77,77); assert(!purpose_valid.state);
  request(5,5); assert(!purpose_valid.state); // Requested OFF does not prove a stopped compressor.
  request(2,2);
  t.parse_frame_(9,frame(9,{0x12,0x34,0xff,0xff,0,123,1,0,0,0}));
  assert(h1.state==4660 && h2.state==65535 && heat.state==123 && dhw.state==256);
  fake_millis+=31000;
  t.v03_stale_(); t.update_communication_state_(); t.v06_publish_();
  assert(temp.state==12.3f && compressor.valid && compressor.state && stage.state==2);
  assert(!fb_fresh.state && !f2_fresh.state && !purpose_valid.state);
  assert(purpose.state=="Heizung (vorlaeufig)" && quality.state.find("FB=veraltet/31s")!=std::string::npos);
  assert(h1.state==4660 && t.block_fresh_(9)); // History has its own, longer interval.
  request(0,0); assert(!purpose_valid.state && purpose.state=="Heizung (vorlaeufig)");
  running(); assert(purpose.state=="Warmwasser (vorlaeufig)" && purpose_valid.state);
  // Fresh compressor OFF is sufficient, even if no F2 is currently available.
  fake_millis+=31000; fb[20]=0; running();
  assert(purpose.state=="Aus" && purpose_valid.state);
  t.parse_frame_(5,frame(5,{0,3,0,10,0}));
  assert(t.profile_fresh_());
  t.reinitialize();
  assert(temp.state==12.3f && compressor.valid && h1.state==4660);
  assert(!fb_fresh.state && !purpose_valid.state && !t.block_fresh_(9));
  assert(purpose.state=="Aus");
  assert(!t.profile_fresh_()); // Retained profile values cannot authorize copying stale data.
  // Decoder changes invalidate the old interpretation rather than preserving it as evidence.
  t.set_firmware("2.16");
  assert(std::isnan(temp.state) && !compressor.valid);
  // Millisecond wraparound does not produce fresh evidence from a missing block.
  fake_millis=0xfffffff0U; V06 wrap; wrap.setup();
  wrap.parse_frame_(0xFB,frame(0xFB,fb)); fake_millis=20;
  assert(wrap.block_fresh_(0xFB) && !wrap.block_fresh_(9));
  std::cout << "PASS v0.6: retained values, block freshness/recovery/reinit, FHEM hours, purpose/requests, wraparound\n";
}
