#include "lwz2.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace esphome::lwz2 {

bool LWZ2Component::profile_valid_(const HeatingProfile &p) {
  return std::memchr(p.name, 0, sizeof(p.name)) && (!p.used || (p.name[0] &&
      p.raw[0] <= 10 && p.raw[1] <= 50 && p.raw[2] <= 100));
}
bool LWZ2Component::profiles_store_(const ProfileStorage &candidate) {
  profile_store_ok_ = profile_preference_.save(&candidate) && global_preferences->sync();
  if (!profile_store_ok_) {
    v04_publish_text_("profile_action_status", "Speicherfehler; nicht als gespeichert bestaetigt");
    return false;
  }
  profiles_ = candidate;
  return true;
}
void LWZ2Component::profiles_setup_() {
  profile_preference_ = global_preferences->make_preference<ProfileStorage>(preferences_key_ ^ 0x50524F35U);
  std::snprintf(profiles_.slots[0].name, 32, "Uebergangszeit");
  profiles_.slots[0].used = true; profiles_.slots[0].raw[0] = 2; profiles_.slots[0].raw[1] = 20;
  std::snprintf(profiles_.slots[1].name, 32, "Winter");
  profiles_.slots[1].used = true; profiles_.slots[1].raw[0] = 4; profiles_.slots[1].raw[1] = 10;
  for (unsigned i = 2; i < 5; i++) std::snprintf(profiles_.slots[i].name, 32, "Eigenes Profil %u", i - 1);
  ProfileStorage loaded{};
  const bool present = profile_preference_.load(&loaded);
  bool valid = present && loaded.magic == profiles_.magic && loaded.selected_slot < 5 && profile_valid_(loaded.last_applied);
  for (const auto &p : loaded.slots) valid = valid && profile_valid_(p);
  if (valid) profiles_ = loaded;
  else if (present) profile_store_ok_ = false;
  profile_slot_ = profiles_.selected_slot; profile_draft_ = profiles_.slots[profile_slot_]; profile_dirty_ = false;
  profiles_publish_editor_();
  v04_publish_text_("profile_action_status", profile_store_ok_ ? "Bereit; keine automatische Anwendung" : "Profilspeicher ungueltig; Vorschlaege geladen, bitte speichern");
  profiles_tick_();
}
void LWZ2Component::profiles_publish_editor_() {
  if (profile_select_) profile_select_->publish_state("Profil " + std::to_string(profile_slot_ + 1));
  if (profile_name_) profile_name_->publish_state(profile_draft_.name);
  for (unsigned i = 0; i < 3; i++) if (profile_values_[i]) profile_values_[i]->publish_state(profile_draft_.raw[i] / 10.0f);
  v04_publish_text_("profile_selected", "Profil " + std::to_string(profile_slot_ + 1) + ": " + profiles_.slots[profile_slot_].name +
      (profiles_.slots[profile_slot_].used ? "" : " (leer)") + (profile_dirty_ ? "; Entwurf ungespeichert" : ""));
}
bool LWZ2Component::select_profile_slot(const std::string &slot) {
  if (write_stage_ != WriteStage::NONE) { v04_publish_text_("profile_action_status", "Auswahl abgewiesen: Schreibvorgang laeuft"); return false; }
  for (unsigned i = 0; i < 5; i++) if (slot == "Profil " + std::to_string(i + 1)) {
    if (i == profile_slot_) return true;
    if (profile_dirty_) { v04_publish_text_("profile_action_status", "Zuerst Entwurf speichern oder verwerfen"); return false; }
    auto candidate = profiles_; candidate.selected_slot = i;
    if (!profiles_store_(candidate)) return false;
    profile_slot_ = i; profile_draft_ = profiles_.slots[i]; profiles_publish_editor_(); return true;
  }
  return false;
}
bool LWZ2Component::edit_profile_name(const std::string &name) {
  if (write_stage_ != WriteStage::NONE || name.empty() || name.size() > 31 || name.find_first_not_of(' ') == std::string::npos ||
      std::any_of(name.begin(), name.end(), [](unsigned char c) { return c < 32 || c == 127; })) {
    v04_publish_text_("profile_action_status", "Name abgewiesen: 1-31 UTF-8-Bytes, kein Steuerzeichen, kein laufender Schreibvorgang"); return false;
  }
  std::snprintf(profile_draft_.name, 32, "%s", name.c_str()); profile_dirty_ = true; profiles_publish_editor_(); return true;
}
bool LWZ2Component::edit_profile_value(uint8_t i, float value) {
  const float max = i == 0 ? 1.0f : i == 1 ? 5.0f : 10.0f;
  if (i >= 3 || write_stage_ != WriteStage::NONE || !std::isfinite(value) || value < 0 || value > max ||
      std::fabs(value * 10 - std::round(value * 10)) > 0.001f) {
    v04_publish_text_("profile_action_status", "Profilwert abgewiesen: Bereich/Raster oder laufender Schreibvorgang"); return false;
  }
  profile_draft_.raw[i] = std::lround(value * 10); profile_dirty_ = true; profiles_publish_editor_(); return true;
}
bool LWZ2Component::save_profile() {
  if (write_stage_ != WriteStage::NONE) return false;
  auto candidate = profiles_; auto entry = profile_draft_; entry.used = true;
  if (!profile_valid_(entry)) return false;
  for (unsigned i = 0; i < 5; i++) if (i != profile_slot_ && profiles_.slots[i].used && std::strcmp(entry.name, profiles_.slots[i].name) == 0) {
    v04_publish_text_("profile_action_status", "Speichern abgewiesen: Profilname bereits vergeben"); return false;
  }
  candidate.slots[profile_slot_] = entry;
  if (!profiles_store_(candidate)) return false;
  profile_draft_ = entry; profile_dirty_ = false; profiles_publish_editor_(); profiles_tick_();
  v04_publish_text_("profile_action_status", "Profil in ESPHome gespeichert; LWZ unveraendert"); return true;
}
void LWZ2Component::discard_profile_draft() {
  if (write_stage_ != WriteStage::NONE) return;
  profile_draft_ = profiles_.slots[profile_slot_]; profile_dirty_ = false; profiles_publish_editor_();
  v04_publish_text_("profile_action_status", "Entwurf verworfen; LWZ unveraendert");
}
bool LWZ2Component::profile_fresh_() const {
  return profile_read_seen_ && !block_stale_[5] && millis() - profile_read_ms_ <= std::max(stale_after_ms_, config_interval_ms_ * 3);
}
bool LWZ2Component::copy_actual_to_profile() {
  if (write_stage_ != WriteStage::NONE || !profile_fresh_() || profile_actual_[0] > 10 || profile_actual_[1] > 50 || profile_actual_[2] > 100) {
    v04_publish_text_("profile_action_status", "Uebernahme abgewiesen: Anlagenwerte fehlen/veraltet/ausserhalb Profilbereich oder Schreibvorgang laeuft"); return false;
  }
  std::copy(profile_actual_, profile_actual_ + 3, profile_draft_.raw); profile_dirty_ = true; profiles_publish_editor_();
  v04_publish_text_("profile_action_status", "Anlagenwerte im Entwurf; noch nicht gespeichert"); return true;
}
bool LWZ2Component::apply_profile() {
  if (!writes_enabled_ || write_stage_ != WriteStage::NONE || profile_dirty_ || !profiles_.slots[profile_slot_].used || !profile_store_ok_) {
    v04_publish_text_("profile_action_status", "Anwendung abgewiesen: Schreibfreigabe, Speicherstatus, gespeichertes Profil und unveraenderter Entwurf erforderlich"); return false;
  }
  profile_pending_ = profiles_.slots[profile_slot_]; profile_write_ = true;
  write_command_ = 5; write_index_ = 3; write_stage_ = WriteStage::READ;
  publish_write_status_("pending: fresh heating parameter read for complete profile");
  v04_publish_text_("profile_action_status", "Profilanwendung laeuft; warte auf unabhaengiges Ruecklesen"); return true;
}
void LWZ2Component::profiles_read_(const uint8_t *data, size_t len) {
  profile_read_seen_ = len >= 5;
  if (profile_read_seen_) {
    profile_actual_[0] = be16u_(data); profile_actual_[1] = be16u_(data + 2); profile_actual_[2] = data[4]; profile_read_ms_ = millis();
  }
  profiles_tick_();
}
void LWZ2Component::profiles_finish_(bool verified) {
  if (!profile_write_) return;
  profile_write_ = false;
  if (!verified) { v04_publish_text_("profile_action_status", "Profil nicht als angewendet bestaetigt; Schreibstatus pruefen, keine automatische Wiederholung"); return; }
  auto candidate = profiles_; candidate.last_applied = profile_pending_;
  if (profiles_store_(candidate)) v04_publish_text_("profile_action_status", "Profil vollstaendig rueckgelesen und Anwendung gespeichert");
  else v04_publish_text_("profile_action_status", "LWZ-Ruecklesen erfolgreich; Speichern der Anwendung fehlgeschlagen");
  profiles_tick_();
}
void LWZ2Component::profiles_tick_() {
  std::string match = "Unbekannt (Parameter fehlen)";
  if (profile_read_seen_) {
    std::string names;
    for (const auto &p : profiles_.slots) if (p.used && std::equal(p.raw, p.raw + 3, profile_actual_)) {
      if (!names.empty()) names += " / ";
      names += p.name;
    }
    match = names.empty() ? "Benutzerdefiniert" : names;
    if (!profile_fresh_()) match = "Veraltet: " + match + " (letzte gelesene Werte)";
  }
  if (match != profile_match_last_) { profile_match_last_ = match; v04_publish_text_("profile_recognized", match); }
  const std::string data = profile_read_seen_ ? std::string(profile_fresh_() ? "frisch; " : "veraltet; ") +
      "Alter " + std::to_string((millis() - profile_read_ms_) / 1000) + "s; P13/P14/P15=" +
      std::to_string(profile_actual_[0] / 10.0f) + "/" + std::to_string(profile_actual_[1] / 10.0f) + "/" + std::to_string(profile_actual_[2] / 10.0f) : "Block 05 / P13-P15 nicht vollstaendig verfuegbar";
  if (data != profile_status_last_) { profile_status_last_ = data; v04_publish_text_("profile_data_status", data); }
  v04_publish_text_("profile_last_applied", profiles_.last_applied.used ?
      std::string(profiles_.last_applied.name) + " (letzte bestaetigte Anwendung, kein Aktivitaetsnachweis)" : "Noch kein Profil erfolgreich angewendet");
}
}  // namespace esphome::lwz2
