// Verify the real generic writer, source layouts, retained observations and model gates.
void v07_tests() {
  const auto previous = fake_millis;
  struct Params : Test {
    using LWZ2Component::WriteStage;
    using LWZ2Component::write_stage_;
    using LWZ2Component::write_command_;
    using LWZ2Component::write_expected_;
    using LWZ2Component::write_response_;
    using LWZ2Component::write_failed_;
    using LWZ2Component::parameters_publish_;
    using LWZ2Component::parameter_layout_;
    using LWZ2Component::parameters_schedule_;
    using LWZ2Component::parameters_result_;
    using LWZ2Component::parameter_due_;
    using LWZ2Component::config_interval_ms_;
    using LWZ2Component::mark_success_;
  };
  for (const auto *profile : {"2.06","2.14","2.14j","2.16"}) {
    unsigned writes=0;
    for (const auto &base : PARAMETER_RULES) {
      if (!base.writable) continue;
      fake_millis=1000;
      Params t; t.set_firmware(profile); t.set_device_model("SOL"); t.set_parameter_overview(true);
      text_sensor::TextSensor status;
      t.set_write_status_text_sensor(&status);
      assert(!t.request_parameter(base.id,base.lower));
      t.set_writes_enabled(true);
      assert(!t.request_parameter(base.id,NAN));
      assert(!t.request_parameter(base.id,base.lower-base.step));
      assert(!t.request_parameter(base.id,base.upper+base.step));
      assert(!t.request_parameter(base.id,base.lower+base.step/2));
      const auto r=t.parameter_layout_(base);
      for (float value : {r.lower,r.upper}) {
        assert(t.request_parameter(r.id,value));
        assert(!t.request_parameter(r.id,value)); // Never replace a pending write.
        t.set_firmware("2.06"); // Decoder change must not alter an in-flight layout.
        t.set_device_model("i");
        std::vector<uint8_t> data(r.minimum+3,0x10); // Include retained tail/escaping bytes.
        data.back()=0x2b;
        auto expected=data;
        const auto raw=uint16_t(std::lround(value*r.divisor));
        if(r.length==2)expected[r.offset]=raw>>8;
        expected[r.offset+r.length-1]=raw&255;
        t.write_response_(frame(r.command,data));
        assert(t.write_stage_==Params::WriteStage::SET && t.write_expected_==expected);
        t.write_response_(frame(r.command,{}));
        assert(t.write_stage_==Params::WriteStage::VERIFY);
        t.write_response_(frame(r.command,expected));
        assert(t.write_stage_==Params::WriteStage::NONE && status.state.find("verified:")==0);
      }
      // Partial reads never create a SET, even when the target byte is present.
      assert(t.request_parameter(r.id,r.lower));
      t.write_response_(frame(r.command,std::vector<uint8_t>(r.minimum-1)));
      assert(t.write_stage_==Params::WriteStage::NONE && status.state.find("rejected:")==0);
      // Lost acknowledgement is followed by a readback; mismatch never rewrites.
      assert(t.request_parameter(r.id,r.lower));
      t.write_response_(frame(r.command,std::vector<uint8_t>(r.minimum,0)));
      t.write_failed_("test ACK loss");
      assert(t.write_stage_==Params::WriteStage::VERIFY);
      auto mismatch=t.write_expected_;mismatch.back()^=0xff;
      t.write_response_(frame(r.command,mismatch));
      assert(t.write_stage_==Params::WriteStage::NONE && status.state.find("FAILED:")==0);
      assert(t.tx.empty()); // Unit-level decisions themselves send no UART frames.
      writes++;
    }
    assert(writes==31);
  }
  {
    Params t; t.set_parameter_overview(true); t.set_device_model("SOL");
    sensor::Sensor negative,solar,hyst;
    LWZParameterNumber water;
    t.set_parameter_sensor(78,&negative);t.set_parameter_sensor(80,&solar);
    t.set_parameter_sensor(25,&hyst);t.set_parameter_number(32,&water);
    std::vector<uint8_t> heat(23);heat[20]=0xff;heat[21]=0x9c;heat[4]=15;
    t.parse_frame_(6,frame(6,heat));assert(negative.state==-10 && hyst.state==1.5f);
    t.parse_frame_(7,frame(7,std::vector<uint8_t>(10,30)));assert(water.state==3);
    fake_millis+=30001;t.parameters_publish_();assert(water.state==3);
    t.reinitialize();assert(water.state==3);
    t.parse_frame_(7,frame(7,{40}));assert(water.state==4);
    t.parse_frame_(8,frame(8,std::vector<uint8_t>(10,1)));assert(solar.state==1);
    t.set_device_model("i");assert(std::isnan(solar.state) && std::isnan(hyst.state));
    t.parse_frame_(8,frame(8,std::vector<uint8_t>(10,1)));assert(std::isnan(solar.state));
    t.set_firmware("2.06");assert(std::isnan(water.state));
    t.set_writes_enabled(true);
    assert(!t.request_parameter(86,1) && !t.request_parameter(25,1));
    assert(!t.request_parameter(78,1)); // Reading does not grant writing.
  }
  {
    // A held display must never make the complete parameter catalogue look fresh.
    fake_millis=1000;
    Params t;t.set_parameter_overview(true);t.set_device_model("SOL");t.set_firmware("2.16");
    t.set_config_interval(3600000); // Shipped 60-minute setting; parameter blocks expire after 3 intervals.
    binary_sensor::BinarySensor valid;text_sensor::TextSensor data;LWZParameterNumber water;
    t.set_parameters_valid(&valid);t.set_parameters_status(&data);t.set_parameter_number(32,&water);
    t.parameters_publish_();assert(!valid.state);
    for(uint8_t command:{0x01,0x03,0x05,0x06,0x07,0x08,0x0A,0x0E,0x10,0x17}) {
      size_t minimum=0;
      for(const auto &r:PARAMETER_RULES)if(r.command==command)minimum=std::max(minimum,size_t(r.minimum));
      t.parse_frame_(command,frame(command,std::vector<uint8_t>(minimum,30)));
    }
    assert(valid.state && data.state.find("70/70")==0 && water.state==3);
    fake_millis+=180001;t.parameters_publish_();assert(valid.state); // 0.9.0: 17 is a parameter block.
    fake_millis+=3*3600000;t.parameters_publish_();assert(!valid.state && water.state==3);
    for(uint8_t command:{0x01,0x03,0x05,0x06,0x07,0x08,0x0A,0x0E,0x10}) {
      size_t minimum=0;
      for(const auto &r:PARAMETER_RULES)if(r.command==command)minimum=std::max(minimum,size_t(r.minimum));
      t.parse_frame_(command,frame(command,std::vector<uint8_t>(minimum,30)));
    }
    assert(!valid.state); // Block 17 alone is still missing.
    t.parse_frame_(0x17,frame(0x17,std::vector<uint8_t>(20,30)));assert(valid.state);
    t.parse_frame_(0x07,frame(0x07,{40}));assert(!valid.state && water.state==4);
    t.reinitialize();assert(!valid.state && water.state==4);
  }
  {
    // Exercise the shared UART state machine with a newly added command (07).
    Params t; t.set_parameter_overview(true); t.set_device_model("SOL");t.setup();
    text_sensor::TextSensor status;t.set_write_status_text_sensor(&status);t.set_writes_enabled(true);
    assert(t.request_parameter(32,4.5f));t.loop();
    auto respond=[&](std::vector<uint8_t> data,bool set=false) {
      assert(t.current_command_==7);
      t.inject({0x10});t.inject({0x10,2});
      auto response=frame(7,data);
      if(set) {response[1]=0x80;response[2]=0;for(size_t i=0;i<response.size();i++)if(i!=2)response[2]+=response[i];}
      for(auto b:response) {t.rx.push_back(b);if(b==0x10)t.rx.push_back(0x10);if(b==0x2b)t.rx.push_back(0x18);}
      t.rx.push_back(0x10);t.rx.push_back(3);t.loop();
    };
    std::vector<uint8_t> before(10,0x10);before[9]=0x2b;
    respond(before);assert(t.write_stage_==Params::WriteStage::SET);
    auto target=before;target[0]=45;
    assert(t.write_expected_==target);
    fake_millis+=1001;t.loop();respond({},true);
    assert(t.write_stage_==Params::WriteStage::VERIFY);
    fake_millis+=1001;t.loop();respond(target);
    assert(status.state.find("verified:")==0);
    unsigned set_count=0;for(size_t i=1;i<t.tx.size();i++)if(t.tx[i-1]==1&&t.tx[i]==0x80)set_count++;
    assert(set_count==1);
  }
  {
    Params t;t.set_parameter_overview(true);t.set_device_model("i");
    t.config_interval_ms_=60000;
    for(auto &due:t.parameter_due_)due=millis();
    for(uint8_t command:{0x03,0x06,0x07,0x0A,0x0E,0x10}) {
      assert(t.parameters_schedule_() && t.current_command_==command);
      t.parameters_result_(command);
    }
    assert(!t.parameters_schedule_());
    t.set_device_model("SOL");
    assert(t.parameters_schedule_() && t.current_command_==0x06);t.parameters_result_(6);
    assert(t.parameters_schedule_() && t.current_command_==0x08);
    // Unknown/truncated parameter frames are rejected by transport validation.
    t.set_firmware("2.16");
    std::string reason;
    for(const auto &r:PARAMETER_RULES) {
      t.current_command_=r.command;
      auto length=r.command==1?size_t(12):r.command==5?size_t(4):size_t(r.minimum);
      assert(t.validate_frame_(frame(r.command,std::vector<uint8_t>(length)),reason));
      assert(!t.validate_frame_(frame(r.command,std::vector<uint8_t>(length-1)),reason));
    }
  }
  fake_millis=previous;
  {
    // Saturated bus (fast 5 s status interval): rare blocks must not starve behind routine polls.
    Params t;t.set_parameter_overview(true);t.set_device_model("SOL");t.setup();
    t.set_status_interval(5000);
    std::set<uint8_t> polled;
    const uint32_t start=fake_millis;
    while (fake_millis-start<300000) {
      const size_t before=t.tx.size();
      fake_millis+=1001;t.loop();
      if (t.tx.size()==before) continue;
      const uint8_t command=t.current_command_;polled.insert(command);
      t.inject({0x10});t.inject({0x10,2});
      for(auto b:frame(command,std::vector<uint8_t>(40,0))) {t.rx.push_back(b);if(b==0x10)t.rx.push_back(0x10);if(b==0x2b)t.rx.push_back(0x18);}
      t.rx.push_back(0x10);t.rx.push_back(3);t.loop();
    }
    for (uint8_t command:{0x01,0x09,0x03,0x06,0x07,0x08,0x0A,0x0E,0x10,0xFB,0xF2,0xF6}) assert(polled.count(command));
    // Polling classes: live data follows the status interval, 17/09/01 the parameter interval,
    // the clock a fixed 5 minutes.
    t.set_config_interval(3600000);
    t.current_command_=0xFC;t.mark_success_();assert(t.v03_due_[2]==millis()+300000);
    t.current_command_=0x17;t.mark_success_();assert(t.v03_due_[3]==millis()+3600000);
    t.current_command_=0x09;t.mark_success_();assert(t.next_09_ms_==millis()+3600000);
    t.current_command_=0xF4;t.mark_success_();assert(t.next_f4_ms_==millis()+5000);
    t.current_command_=0xF2;t.mark_success_();assert(t.v04_due_[0]==millis()+5000);
    // Manual refresh: each block once, before routine polls, with progress and result.
    auto serve=[&](Params &p)->int {
      const size_t before=p.tx.size();
      fake_millis+=1001;p.loop();
      if (p.tx.size()==before) return -1;
      const uint8_t command=p.current_command_;
      p.inject({0x10});p.inject({0x10,2});
      for(auto b:frame(command,std::vector<uint8_t>(40,0))) {p.rx.push_back(b);if(b==0x10)p.rx.push_back(0x10);if(b==0x2b)p.rx.push_back(0x18);}
      p.rx.push_back(0x10);p.rx.push_back(3);p.loop();
      return command;
    };
    assert(t.manual_refresh_status()=="keine manuelle Aktualisierung");
    t.refresh_parameters();t.refresh_parameters(); // Double click adds nothing.
    assert(t.manual_refresh_status()=="laeuft: 0/11 Bloecke gelesen");
    std::set<int> manual;
    for(int i=0;i<11;i++) manual.insert(serve(t));
    for(int command:{0x05,0x17,0x09,0x01,0x03,0x06,0x07,0x08,0x0A,0x0E,0x10}) assert(manual.count(command));
    assert(t.manual_refresh_status()=="fertig: 11/11 Bloecke gelesen");
    t.refresh_measurements();assert(serve(t)==0xFB);
    assert(t.manual_refresh_status()=="laeuft: 1/8 Bloecke gelesen");
    for(int i=0;i<7;i++) serve(t);
    assert(t.manual_refresh_status()=="fertig: 8/8 Bloecke gelesen");
    // 0.9.2: "HK1 jetzt aktualisieren" reads block 05 only; parameter schedule stays untouched.
    const uint32_t due03=t.parameter_due_[0];
    t.refresh_config();assert(t.manual_refresh_status()=="laeuft: 0/1 Bloecke gelesen");
    assert(serve(t)==0x05 && t.manual_refresh_status()=="fertig: 1/1 Bloecke gelesen");
    assert(t.parameter_due_[0]==due03);
    fake_millis=previous;  // Later fixtures assume an early boot clock.
  }
  std::cout << "PASS v0.7: 31 parameter writers x4 layouts, retained fields, ranges, short reads, ACK loss/readback, model gates, polling and signed values\n";
}
