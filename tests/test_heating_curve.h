// P13/P14/P15: actual RMW code, synthetic FHEM-layout payloads, no device writes.
void heating_curve_tests() {
  struct Curve : Test {
    using LWZ2Component::write_expected_;
    using LWZ2Component::write_response_;
    using LWZ2Component::write_failed_;
    using LWZ2Component::write_stage_;
    using LWZ2Component::WriteStage;
    using LWZ2Component::invalidate_block_;
    using LWZ2Component::v03_stale_;
  };
  const float maxima[]{1,5,10};
  for (auto profile : {"2.06","2.14","2.14j","2.16"}) {
    for (unsigned index=0; index<3; index++) {
      Curve t; LWZHeatingCurveNumber controls[3]; text_sensor::TextSensor status;
      t.set_firmware(profile); t.set_write_status_text_sensor(&status);
      for (unsigned i=0;i<3;i++) t.set_heating_curve_number(i,&controls[i]);
      t.setup(); assert(std::isnan(controls[index].state));
      assert(!t.request_heating_curve(index,0.1f)); // Permission starts off.
      t.set_writes_enabled(true);
      for (float invalid : {-0.1f,maxima[index]+0.1f,0.15f,NAN,INFINITY})
        assert(!t.request_heating_curve(index,invalid));
      for (float target : {0.0f,0.1f,maxima[index]}) {
        std::vector<uint8_t> block(24,0xAA); // Tail and HC2 fields must survive.
        block[0]=0;block[1]=3;block[2]=0;block[3]=10;block[4]=0;
        t.parse_frame_(5,frame(5,block));
        const float old=controls[index].state;
        assert(t.request_heating_curve(index,target));
        assert(!t.request_heating_curve(index,target)); // One outstanding command.
        assert(controls[index].state==old); // No optimistic slider state.
        t.write_response_(frame(5,block));
        auto expected=block; const uint16_t raw=std::lround(target*10);
        if(index<2) {expected[index*2]=raw>>8;expected[index*2+1]=raw&255;}
        else expected[4]=raw;
        assert(t.write_expected_==expected && t.write_stage_==Curve::WriteStage::SET);
        t.write_failed_("lost SET response"); // Single SET, then GET; never SET again.
        assert(t.write_stage_==Curve::WriteStage::VERIFY);
        t.parse_frame_(5,frame(5,expected)); t.write_response_(frame(5,expected));
        assert(controls[index].state==target && status.state.find("verified:")==0);
      }
      assert(t.request_heating_curve(index,0.1f));
      t.write_response_(frame(5,std::vector<uint8_t>(21,0)));
      assert(t.write_stage_==Curve::WriteStage::NONE && status.state.find("rejected:")==0);
      assert(t.request_heating_curve(index,0.1f));
      t.write_response_(frame(5,std::vector<uint8_t>(22,0)));
      auto wrong=t.write_expected_; wrong.back()^=1;
      t.write_response_(frame(5,{})); t.write_response_(frame(5,wrong));
      assert(t.write_stage_==Curve::WriteStage::NONE && status.state.find("FAILED:")==0);
      t.parse_frame_(5,frame(5,{0,3,0,10})); // Legacy short read: P15 unavailable.
      assert(controls[0].state==0.3f && controls[1].state==1 && std::isnan(controls[2].state));
      t.invalidate_block_(5); for(auto &n:controls) assert(std::isnan(n.state));
      t.parse_frame_(5,frame(5,std::vector<uint8_t>(22,0)));
      t.reinitialize(); for(auto &n:controls) assert(n.state==0); // Display retained; profile freshness reset.
    }
  }
  std::cout << "PASS heating curve: P13/P14/P15 bounds/grid, byte preservation, readback, uncertainty, stale/reset\n";
}
