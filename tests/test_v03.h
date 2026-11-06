// End-to-end protocol fixtures (synthetic, not hardware captures).
void v03_tests() {
  struct V03 : Test {
    using LWZ2Component::write_stage_;
    using LWZ2Component::WriteStage;
    using LWZ2Component::write_expected_;
    using LWZ2Component::sending_set_;
    using LWZ2Component::v03_stale_;
    using LWZ2Component::parse_selected_;
    using LWZ2Component::next_f4_ms_;
    using LWZ2Component::last_error_message_;
  };
  auto answer = [](V03 &t, uint8_t command, std::vector<uint8_t> data, bool set = false) {
    assert(t.current_command_ == command);
    t.inject({0x10}); t.inject({0x10, 0x02});
    auto f = frame(command, data);
    if (set) {
      f[1] = 0x80; f[2] = 0;
      for (size_t i = 0; i < f.size(); i++) if (i != 2) f[2] += f[i];
    }
    for (auto b : f) {
      t.rx.push_back(b);
      if (b == 0x10) t.rx.push_back(0x10);
      if (b == 0x2B) t.rx.push_back(0x18);
    }
    t.rx.push_back(0x10); t.rx.push_back(3); t.loop();
  };
  auto advance = [](V03 &t) { fake_millis += 1001; t.loop(); };
  auto set_count = [](const std::vector<uint8_t> &tx) {
    unsigned n = 0;
    for (size_t i = 1; i < tx.size(); i++) if (tx[i - 1] == 1 && tx[i] == 0x80) n++;
    return n;
  };
  // Ventilation Speed200 calibration values: distinct from current speed/target,
  // FHEM uint16 BE /10, no inferred physical unit, identical E8 decoder in all profiles.
  for (const auto *profile : {"2.06", "2.14", "2.14j"}) {
    V03 t;sensor::Sensor supply200,exhaust200;
    t.set_firmware(profile);
    t.set_selected_sensor("s_fan_supply_fan_speed200",&supply200);
    t.set_selected_sensor("s_fan_exhaust_fan_speed200",&exhaust200);
    std::vector<uint8_t> e8(30);e8[26]=0;e8[27]=31;e8[28]=0x12;e8[29]=0;
    t.parse_frame_(0xE8,frame(0xE8,e8));
    assert(std::fabs(supply200.state-3.1f)<0.001f && std::fabs(exhaust200.state-460.8f)<0.001f);
    e8[28]=0x11;e8[29]=0;t.parse_frame_(0xE8,frame(0xE8,e8));
    assert(std::fabs(exhaust200.state-435.2f)<0.001f);
    e8.resize(28);t.parse_frame_(0xE8,frame(0xE8,e8));
    assert(std::fabs(supply200.state-3.1f)<0.001f && std::isnan(exhaust200.state));
    e8.resize(22);t.parse_frame_(0xE8,frame(0xE8,e8));
    assert(std::isnan(supply200.state) && std::isnan(exhaust200.state));
  }
  // Selected fields use exact offsets, signed decoding, explicit enums and distinct sources.
  V03 decoder; sensor::Sensor dhw_target, integral, room, inside_fb, inside_f4, fan_speed, fan_percent;
  text_sensor::TextSensor mode, season, weekday;
  binary_sensor::BinarySensor boost3, mixer;
  decoder.set_firmware("2.14j"); decoder.set_hc1_layout("2.06");
  decoder.set_selected_sensor("s_d_h_w_dhw_set_temp", &dhw_target);
  decoder.set_selected_sensor("s_h_c1_integral_heat", &integral);
  decoder.set_selected_sensor("s_h_c1_room_set_temp", &room);
  decoder.set_selected_sensor("s_h_c1_inside_temp", &inside_f4);
  decoder.set_inside_temperature_sensor(&inside_fb);
  decoder.set_supply_fan_speed_sensor(&fan_speed);
  decoder.set_supply_fan_speed_percent_sensor(&fan_percent);
  decoder.set_selected_text_sensor("s_d_h_w_dhw_op_mode", &mode);
  decoder.set_selected_text_sensor("s_h_c1_season_mode", &season);
  decoder.set_selected_text_sensor("s_timedate_weekday", &weekday);
  decoder.set_selected_binary_sensor("s_global_booster_stage3", &boost3);
  decoder.set_selected_binary_sensor("s_global_mixer_open", &mixer);
  std::vector<uint8_t> f3(18); f3[4]=1;f3[5]=0xd6;f3[15]=2;
  decoder.parse_frame_(0xF3,frame(0xF3,f3)); assert(dhw_target.state==47 && mode.state=="setback");
  f3[15]=9;decoder.parse_frame_(0xF3,frame(0xF3,f3));assert(mode.state=="unknown(9)");
  std::vector<uint8_t> fb(32);fb[14]=0;fb[15]=200;fb[21]=2;
  fb[27]=32;
  decoder.parse_frame_(0xFB,frame(0xFB,fb));assert(inside_fb.state==20 && mixer.state && !boost3.valid && fan_percent.state==32);
  auto short_fb=fb;short_fb.resize(21);decoder.parse_frame_(0xFB,frame(0xFB,short_fb));assert(std::isnan(fan_percent.state));
  decoder.set_firmware("2.06");fb[20]=0x80;decoder.parse_frame_(0xFB,frame(0xFB,fb));assert(boost3.valid && boost3.state);
  decoder.set_firmware("2.14j");assert(!boost3.valid);
  decoder.parse_frame_(0xFB,frame(0xFB,fb)); // Fresh live data after the profile change.
  std::vector<uint8_t> f4(43);f4[6]=0xff;f4[7]=0xcf;f4[17]=1;f4[26]=0;f4[27]=214;f4[29]=0;
  decoder.parse_frame_(0xF4,frame(0xF4,f4));assert(integral.state==-49 && room.state==21.4f && season.state=="winter");
  assert(inside_fb.state==20 && std::isnan(inside_f4.state)); // F4 never overwrites FB.
  decoder.set_hc1_layout("2.14j");f4[41]=0;f4[42]=201;
  decoder.parse_frame_(0xF4,frame(0xF4,f4));assert(inside_f4.state==20.1f && room.state==0);
  f4.resize(39);decoder.parse_frame_(0xF4,frame(0xF4,f4));assert(std::isnan(inside_f4.state));
  decoder.parse_frame_(0xFC,frame(0xFC,{0,4,12,15,16,26,0,11,6}));assert(weekday.state=="Friday");
  // Independent block expiry even while FB remains healthy.
  decoder.set_v03_enabled(true);
  std::vector<uint8_t> e8(22); e8[11]=36;decoder.parse_frame_(0xE8,frame(0xE8,e8));assert(fan_speed.state==36);
  fake_millis+=91001;decoder.v03_stale_();
  assert(std::isnan(dhw_target.state) && mode.state=="unavailable" && std::isnan(fan_speed.state));
  // Four targeted writes: preserve every other byte, wait for independent readback, no optimistic success.
  for (unsigned index=0;index<4;index++) {
    V03 t;text_sensor::TextSensor status;t.set_write_status_text_sensor(&status);t.setup();
    assert(!t.request_setpoint(index,21.4f));t.set_writes_enabled(true);
    assert(!t.request_setpoint(index,NAN));assert(!t.request_setpoint(index,99));
    assert(!t.request_setpoint(index,index<2?21.45f:47.45f));
    const float value=index<2?21.4f:47.0f;
    assert(t.request_setpoint(index,value));t.loop();
    std::vector<uint8_t> original(20,0);original[16]=0x10;original[18]=0x2b;
    answer(t,0x17,original);assert(t.write_stage_==V03::WriteStage::SET);
    assert(!t.request_setpoint(index,value)); // One outstanding action only.
    auto expected=original;const unsigned offsets[]{0,2,6,8};auto raw=static_cast<unsigned>(std::lround(value*10));
    expected[offsets[index]]=raw>>8;expected[offsets[index]+1]=raw&255;assert(t.write_expected_==expected);
    advance(t);assert(t.sending_set_);answer(t,0x17,{},true);
    assert(t.write_stage_==V03::WriteStage::VERIFY && status.state.find("verified:")!=0);
    advance(t);answer(t,0x17,expected);assert(status.state.find("verified:")==0);
    assert(t.write_stage_==V03::WriteStage::NONE && set_count(t.tx)==1);
  }
  // A lost SET acknowledgement goes directly to GET verification; never repeats SET.
  {
    V03 t;text_sensor::TextSensor status;t.set_write_status_text_sensor(&status);t.setup();t.set_writes_enabled(true);
    t.request_setpoint(0,22);t.loop();answer(t,0x17,std::vector<uint8_t>(20));auto expected=t.write_expected_;
    advance(t);t.inject({0x10});fake_millis+=1801;t.loop();
    assert(t.write_stage_==V03::WriteStage::VERIFY && t.retries_==0);
    advance(t);answer(t,0x17,expected);assert(status.state.find("verified:")==0 && set_count(t.tx)==1);
  }
  // Verification checks retained fields too, and never automatically repairs a mismatch.
  {
    V03 t;text_sensor::TextSensor status;t.set_write_status_text_sensor(&status);t.setup();t.set_writes_enabled(true);
    t.request_setpoint(0,22);t.loop();answer(t,0x17,std::vector<uint8_t>(20));auto expected=t.write_expected_;
    advance(t);answer(t,0x17,{},true);advance(t);expected[19]=3;answer(t,0x17,expected);
    assert(status.state.find("FAILED:")==0 && set_count(t.tx)==1);
    t.current_command_=0x17;std::string reason;assert(!t.validate_frame_(frame(0x17,std::vector<uint8_t>(19)),reason));
  }
  // Clock shift preserves calendar fields, rejects day boundaries and blocks repeated clicks.
  for(int shift:{-1,1}) {
    V03 t;text_sensor::TextSensor status;t.set_write_status_text_sensor(&status);t.setup();t.set_writes_enabled(true);
    assert(t.request_clock_shift(shift));t.loop();auto clock=std::vector<uint8_t>{0,4,12,15,16,26,0,11,6};
    answer(t,0xFC,clock);auto expected=t.write_expected_;assert(expected[2]==12+shift);
    advance(t);answer(t,0xFC,{},true);advance(t);expected[4]+=2;answer(t,0xFC,expected);
    assert(status.state.find("verified:")==0 && !t.request_clock_shift(shift));
  }
  for(auto clock:{std::vector<uint8_t>{0,4,0,15,16,26,0,11,6},std::vector<uint8_t>{0,4,23,15,16,26,0,11,6}}) {
    V03 t;text_sensor::TextSensor status;t.set_write_status_text_sensor(&status);t.setup();t.set_writes_enabled(true);
    t.request_clock_shift(clock[2]==0?-1:1);t.loop();answer(t,0xFC,clock);
    assert(t.write_stage_==V03::WriteStage::NONE && set_count(t.tx)==0);
  }
  // Revoking permission before SET suppresses the write while still allowing ordinary reads.
  {
    V03 t;t.setup();t.set_writes_enabled(true);t.request_setpoint(0,22);t.loop();
    answer(t,0x17,std::vector<uint8_t>(20));t.set_writes_enabled(false);advance(t);
    assert(t.write_stage_==V03::WriteStage::NONE && set_count(t.tx)==0);
  }
  std::cout << "PASS v0.3: field mappings, independent sources, stale blocks, 4 RMW setpoints, uncertain SET readback, retained fields, clock +/-1h and rollover rejection\n";
}
