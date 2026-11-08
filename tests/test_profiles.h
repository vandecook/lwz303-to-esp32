// v0.5 profile editor/persistence and complete P13/P14/P15 RMW fixtures.
void profile_tests() {
  struct ProfileTest : Test {
    using LWZ2Component::profiles_;
    using LWZ2Component::profile_draft_;
    using LWZ2Component::profile_dirty_;
    using LWZ2Component::profile_slot_;
    using LWZ2Component::profile_write_;
    using LWZ2Component::write_expected_;
    using LWZ2Component::write_stage_;
    using LWZ2Component::WriteStage;
    using LWZ2Component::write_response_;
    using LWZ2Component::write_failed_;
    using LWZ2Component::profiles_tick_;
    using LWZ2Component::saved_;
    using LWZ2Component::v04_save_;
  };
  fake_storage.clear(); fake_storage_ok=true; fake_millis=1000;
  ProfileTest t; text_sensor::TextSensor recognized, last, action, data;
  t.set_preferences_key(500); t.set_v04_text("profile_recognized",&recognized);
  t.set_v04_text("profile_last_applied",&last); t.set_v04_text("profile_action_status",&action);
  t.set_v04_text("profile_data_status",&data); t.setup();
  assert(t.tx.empty() && t.write_stage_==ProfileTest::WriteStage::NONE);
  assert(recognized.state.find("Unbekannt")==0 && t.profiles_.slots[0].raw[0]==2 && t.profiles_.slots[1].raw[0]==4);
  assert(!t.apply_profile()); // Only explicit write permission allows application.
  std::vector<uint8_t> actual(24,0xAA);actual[0]=0;actual[1]=3;actual[2]=0;actual[3]=10;actual[4]=0;
  t.parse_frame_(5,frame(5,actual));assert(recognized.state=="Standard (bewaehrt)");
  actual[1]=2;actual[3]=20;t.parse_frame_(5,frame(5,actual));assert(recognized.state=="Uebergangszeit");
  actual[4]=1;t.parse_frame_(5,frame(5,actual));assert(recognized.state=="Benutzerdefiniert"); // P15 participates.
  actual[4]=0;t.parse_frame_(5,frame(5,actual));
  fake_millis+=180001;t.profiles_tick_();assert(recognized.state.find("Veraltet:")==0 && data.state.find("veraltet")==0);
  assert(!t.copy_actual_to_profile());
  t.parse_frame_(5,frame(5,{0,2,0,20}));assert(recognized.state.find("Unbekannt")==0);
  t.parse_frame_(5,frame(5,actual));
  // Empty custom slots, editable labels, draft-only changes, explicit durable save.
  assert(t.select_profile_slot("Profil 3") && !t.profiles_.slots[2].used);
  assert(t.edit_profile_name("Mein Test"));
  assert(t.edit_profile_value(0,0.3f) && t.edit_profile_value(1,1.0f) && t.edit_profile_value(2,0.1f));
  assert(t.tx.empty() && t.profile_dirty_ && !t.profiles_.slots[2].used);
  assert(!t.select_profile_slot("Profil 2") && !t.apply_profile());
  for(float bad:{-0.1f,1.1f,0.15f,NAN,INFINITY}) assert(!t.edit_profile_value(0,bad));
  assert(!t.edit_profile_name("") && !t.edit_profile_name(" \n") && !t.edit_profile_name(std::string(32,'x')));
  fake_storage_ok=false;assert(!t.save_profile() && t.profile_dirty_ && !t.profiles_.slots[2].used);
  fake_storage_ok=true;assert(t.save_profile() && !t.profile_dirty_ && t.profiles_.slots[2].used);
  assert(t.profiles_.slots[2].raw[0]==3 && t.profiles_.slots[2].raw[1]==10 && t.profiles_.slots[2].raw[2]==1);
  ProfileTest reboot;reboot.set_preferences_key(500);reboot.setup();
  assert(reboot.profile_slot_==2 && std::string(reboot.profiles_.slots[2].name)=="Mein Test");
  assert(reboot.write_stage_==ProfileTest::WriteStage::NONE && reboot.tx.empty() && !reboot.profiles_.last_applied.used);
  // Duplicate names rejected, draft discard and copying actual values never write the LWZ.
  assert(t.edit_profile_name("Winter") && !t.save_profile()); t.discard_profile_draft();
  assert(std::string(t.profile_draft_.name)=="Mein Test");
  assert(t.copy_actual_to_profile() && t.profile_dirty_ && t.tx.empty());t.discard_profile_draft();
  // Apply all three bytes/words in one preserved full block, verify independently.
  t.set_writes_enabled(true);assert(t.apply_profile());
  assert(!t.edit_profile_value(0,0.4f) && !t.select_profile_slot("Profil 1") && !t.save_profile());
  assert(!t.request_heating_curve(0,0.4f));
  t.write_response_(frame(5,actual));auto expected=actual;expected[1]=3;expected[3]=10;expected[4]=1;
  assert(t.write_expected_==expected && t.write_stage_==ProfileTest::WriteStage::SET);
  ProfileTest interrupted;interrupted.set_preferences_key(500);interrupted.setup();
  assert(interrupted.tx.empty() && interrupted.write_stage_==ProfileTest::WriteStage::NONE && !interrupted.profiles_.last_applied.used);
  t.write_failed_("lost SET ACK");assert(t.write_stage_==ProfileTest::WriteStage::VERIFY);
  t.parse_frame_(5,frame(5,expected));t.write_response_(frame(5,expected));
  assert(recognized.state=="Mein Test" && last.state.find("Mein Test")==0);
  assert(t.profiles_.last_applied.raw[2]==1 && !t.profile_write_);
  // External/manual changes change recognition without editing a saved profile or replaying a SET.
  expected[4]=2;t.parse_frame_(5,frame(5,expected));assert(recognized.state=="Benutzerdefiniert");
  assert(t.profiles_.slots[2].raw[2]==1 && last.state.find("Mein Test")==0);
  const auto tx_size=t.tx.size();t.profiles_tick_();assert(t.tx.size()==tx_size);
  ProfileTest restored;restored.set_preferences_key(500);restored.setup();
  assert(restored.profiles_.last_applied.used && restored.profiles_.last_applied.raw[2]==1 && restored.tx.empty());
  // Failed readback retains previous successful application; no implicit retry.
  assert(t.select_profile_slot("Profil 2") && t.apply_profile());
  t.write_response_(frame(5,actual));auto wrong=t.write_expected_;wrong.back()^=1;
  t.write_response_(frame(5,{}));t.parse_frame_(5,frame(5,wrong));t.write_response_(frame(5,wrong));
  assert(t.write_stage_==ProfileTest::WriteStage::NONE && !t.profile_write_);
  assert(std::string(t.profiles_.last_applied.name)=="Mein Test" && action.state.find("Profil nicht")==0);
  // Short RMW read rejected; stored profiles unchanged. SET is never repeated.
  assert(t.apply_profile());t.write_response_(frame(5,std::vector<uint8_t>(21,0)));
  assert(t.write_stage_==ProfileTest::WriteStage::NONE && !t.profile_write_);
  // Duplicate value combinations are reported together rather than falsely picking one.
  assert(t.select_profile_slot("Profil 4"));assert(t.edit_profile_name("Auch Winter"));
  assert(t.edit_profile_value(0,0.4f) && t.edit_profile_value(1,1) && t.edit_profile_value(2,0) && t.save_profile());
  actual[1]=4;actual[3]=10;actual[4]=0;t.parse_frame_(5,frame(5,actual));
  assert(recognized.state=="Winter / Auch Winter");
  // Old clock/fault storage key remains intact while profiles use a separate key.
  t.saved_.clock_mode=1;t.saved_.fans[0].count=7;assert(t.v04_save_());
  ProfileTest migrated;migrated.set_preferences_key(500);migrated.setup();
  assert(migrated.saved_.clock_mode==1 && migrated.saved_.fans[0].count==7 && migrated.profiles_.slots[3].used);
  // LWZ verification and saving the application record are separate outcomes.
  ProfileTest disk; text_sensor::TextSensor disk_status;disk.set_preferences_key(505);
  disk.set_v04_text("profile_action_status",&disk_status);disk.setup();disk.set_writes_enabled(true);
  assert(disk.apply_profile());disk.write_response_(frame(5,actual));auto verified=disk.write_expected_;
  disk.write_response_(frame(5,{}));disk.parse_frame_(5,frame(5,verified));
  fake_storage_ok=false;disk.write_response_(frame(5,verified));
  assert(!disk.profiles_.last_applied.used && disk_status.state.find("LWZ-Ruecklesen erfolgreich")==0);
  assert(!disk.apply_profile());fake_storage_ok=true;assert(disk.save_profile());
  // Corrupt profile records cannot introduce an out-of-range write after a restart.
  const uint32_t profile_key=505 ^ 0x50524F35U;
  fake_storage[profile_key][0]^=1;
  ProfileTest corrupt;corrupt.set_preferences_key(505);corrupt.setup();corrupt.set_writes_enabled(true);
  assert(!corrupt.apply_profile() && corrupt.tx.empty());assert(corrupt.save_profile());
  // Full UART transaction: GET -> exactly one SET containing the entire preserved block -> GET.
  ProfileTest wire;wire.set_preferences_key(506);wire.setup();wire.set_writes_enabled(true);assert(wire.apply_profile());
  auto answer = [&](const std::vector<uint8_t> &body, bool set=false) {
    wire.inject({0x10});wire.inject({0x10,2});auto reply=frame(5,body);
    if(set) {reply[1]=0x80;reply[2]=0;for(size_t i=0;i<reply.size();i++) if(i!=2)reply[2]+=reply[i];}
    for(auto b:reply) {wire.rx.push_back(b);if(b==0x10)wire.rx.push_back(b);if(b==0x2B)wire.rx.push_back(0x18);}
    wire.rx.push_back(0x10);wire.rx.push_back(3);wire.loop();
  };
  wire.loop();assert(wire.current_command_==5);answer(actual);auto target=wire.write_expected_;
  fake_millis+=1000;wire.loop();answer({},true);
  fake_millis+=1000;wire.loop();answer(target);
  assert(wire.write_stage_==ProfileTest::WriteStage::NONE && wire.profiles_.last_applied.used);
  unsigned writes=0;for(size_t i=1;i<wire.tx.size();i++) if(wire.tx[i-1]==1 && wire.tx[i]==0x80)writes++;
  assert(writes==1 && wire.successes_==3);
  // Permission withdrawal before SET aborts and never confirms application.
  ProfileTest cancel;cancel.set_preferences_key(507);cancel.setup();cancel.set_writes_enabled(true);assert(cancel.apply_profile());
  cancel.write_response_(frame(5,actual));cancel.set_writes_enabled(false);cancel.loop();
  assert(!cancel.profile_write_ && cancel.write_stage_==ProfileTest::WriteStage::NONE && !cancel.profiles_.last_applied.used);
  // Fixed standard is separate from the five persisted/editable slots, preserving their storage layout.
  ProfileTest fixed; text_sensor::TextSensor fixed_match;fixed.set_preferences_key(508);
  fixed.set_v04_text("profile_recognized",&fixed_match);fixed.setup();
  assert(fixed.select_profile_slot("Standard (bewaehrt)"));
  assert(!fixed.edit_profile_name("Changed") && !fixed.edit_profile_value(0,0.5f) && !fixed.save_profile() && !fixed.copy_actual_to_profile());
  assert(!fixed.apply_profile() && fixed.tx.empty());
  fixed.set_writes_enabled(true);assert(fixed.apply_profile());fixed.write_response_(frame(5,actual));
  auto standard=actual;standard[0]=0;standard[1]=3;standard[2]=0;standard[3]=10;standard[4]=0;
  assert(fixed.write_expected_==standard);
  fixed.write_response_(frame(5,{}));fixed.parse_frame_(5,frame(5,standard));fixed.write_response_(frame(5,standard));
  assert(fixed_match.state=="Standard (bewaehrt)" && std::string(fixed.profiles_.last_applied.name)=="Standard (bewaehrt)");
  ProfileTest fixed_reboot;fixed_reboot.set_preferences_key(508);fixed_reboot.setup();
  assert(fixed_reboot.profile_slot_==5 && fixed_reboot.profile_draft_.raw[0]==3 && fixed_reboot.tx.empty());
  assert(fixed.select_profile_slot("Profil 5") && fixed.edit_profile_name("Standard (bewaehrt)") && !fixed.save_profile());
  fake_storage_ok=true;
  std::cout << "PASS v0.5: profile drafts, names, persistence/reboot, recognition incl P15/stale, full-block RMW, failures, legacy preferences\n";
}
