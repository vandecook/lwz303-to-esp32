// Provisional FW 2.16: synthetic fixtures, not captured device responses.
void fw216_tests() {
  struct FW216 : Test {
    using LWZ2Component::v04_speed_;
    using LWZ2Component::zero_pending_;
  };
  FW216 t;
  sensor::Sensor supply, exhaust, supply_target, exhaust_target, inside, room, integral;
  sensor::Sensor supply200, exhaust200, stage_airflow;
  t.set_supply_fan_speed_sensor(&supply); t.set_exhaust_fan_speed_sensor(&exhaust);
  t.set_supply_fan_speed_target_sensor(&supply_target); t.set_exhaust_fan_speed_target_sensor(&exhaust_target);
  t.set_selected_sensor("s_h_c1_inside_temp", &inside);
  t.set_selected_sensor("s_h_c1_room_set_temp", &room);
  t.set_selected_sensor("s_h_c1_integral_heat", &integral);
  t.set_selected_sensor("s_fan_supply_fan_speed200", &supply200);
  t.set_selected_sensor("s_fan_exhaust_fan_speed200", &exhaust200);
  t.set_supply_stage1_airflow_sensor(&stage_airflow);
  std::vector<uint8_t> e8(30, 0); e8[11]=10; e8[13]=20; e8[19]=30; e8[21]=40;
  e8[27]=31; e8[28]=0x12;
  t.set_firmware("2.06"); t.parse_frame_(0xE8,frame(0xE8,e8));
  assert(supply.state==10 && exhaust.state==20 && supply_target.state==30 && exhaust_target.state==40);
  t.set_firmware("2.16");
  assert(t.firmware_==FirmwareProfile::FW_216);
  assert(std::isnan(supply.state) && std::isnan(exhaust.state));
  t.parse_frame_(0xE8,frame(0xE8,e8));
  assert(std::isnan(supply.state) && std::isnan(exhaust.state));
  assert(std::isnan(supply_target.state) && std::isnan(exhaust_target.state));
  assert(std::fabs(supply200.state-3.1f)<0.001f && std::fabs(exhaust200.state-460.8f)<0.001f);
  // 01 retains the 2.06 uint16 layout and minimum body length.
  t.parse_frame_(0x01,frame(0x01,{1,44,0,0,0,0,0,0,0,0,0,0})); assert(stage_airflow.state==300);
  auto short01=frame(0x01,{1,44,0,0,0,0});
  std::string reason; t.current_command_=0x01; assert(!t.validate_frame_(short01,reason));
  // F4 common fields retain the selected layout, exception stays unavailable even with 2.14j selected.
  std::vector<uint8_t> f4(43,0); f4[6]=0xff; f4[7]=0xff; f4[26]=0; f4[27]=214;
  t.set_hc1_layout("2.06"); t.parse_frame_(0xF4,frame(0xF4,f4));
  assert(integral.state==-1 && room.state==21.4f && std::isnan(inside.state));
  t.set_hc1_layout("2.14j"); t.parse_frame_(0xF4,frame(0xF4,f4)); assert(std::isnan(inside.state));
  // An excluded E8 speed cannot trigger F5/F6, while valid FB relative speeds remain usable.
   t.set_fan_speed_source("sFan");
  t.parse_frame_(0xE8,frame(0xE8,e8));
  assert(std::isnan(t.v04_speed_[0]) && std::isnan(t.v04_speed_[1]));
  assert(!t.zero_pending_[0] && !t.zero_pending_[1]);
  t.set_fan_speed_source("sGlobal"); std::vector<uint8_t> fb(32,0);fb[26]=18;fb[27]=32;
  t.parse_frame_(0xFB,frame(0xFB,fb)); assert(t.v04_speed_[0]==18 && t.v04_speed_[1]==32);
  binary_sensor::BinarySensor f5, f6;
  t.set_v04_binary("exhaust_fault", &f5); t.set_v04_binary("supply_fault", &f6);
  t.setup(); t.set_fan_speed_source("sFan");
  auto feed = [&]() {
    std::vector<uint8_t> p(20,0), ee(19,0), sys(18,0);
    p[12]=1; ee[0]=1; ee[5]=1;
    t.parse_frame_(0x17,frame(0x17,p)); t.parse_frame_(0xEE,frame(0xEE,ee));
    t.parse_frame_(0xF6,frame(0xF6,sys));
    t.parse_frame_(0xE8,frame(0xE8,std::vector<uint8_t>(30,0)));
    fb[26]=0; fb[27]=32; t.parse_frame_(0xFB,frame(0xFB,fb));
  };
  feed();
  for (int i=0;i<13;i++) {fake_millis+=10000; feed(); assert(!f5.state && !f6.state);}
  t.set_fan_speed_source("sGlobal"); feed();
  for (int i=0;i<11;i++) {fake_millis+=10000; feed(); assert(!f5.state);}
  fake_millis+=9999; feed(); assert(!f5.state);
  fake_millis+=1; feed(); assert(f5.state && !f6.state);
  // Switching back restores the older decoder without changing its rules.
  t.set_firmware("2.06"); t.parse_frame_(0xE8,frame(0xE8,e8)); assert(supply.state==10 && exhaust.state==20);
}
