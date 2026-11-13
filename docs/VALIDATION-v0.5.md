# Softwarevalidierung v0.5.0

Ergänzung 09.11.2026: aus jeder regulären YAML wurden 154 Gerätenamenpräfixe in Entitäts-Anzeigenamen entfernt. Explizite interne IDs, ESPHome-Gerätename und Geräte-Anzeigename vor/nach Änderung verglichen und unverändert. Auswahl-/Konfigurationstest und Codegenerierung für beide YAMLs prüfen die neue Benennung. Der unten dokumentierte vollständige Build stammt vor dieser reinen Namensänderung; kein zusätzlicher vollständiger Build. Die tatsächliche HA-Registrierung/Anzeige bleibt am System zu prüfen.

Ergänzung fester Standard: P13=0,3, P14=1,0 K, P15=0 vom Nutzer bestätigt. Der zusätzliche unveränderbare Eintrag wurde auf Erkennung, Bearbeitungssperre, Speichersperre, Übernahmesperre, Anwendung nur mit Schreibfreigabe und erhaltene Auswahl nach Neustart geprüft. Das gespeicherte Fünf-Profil-Datenformat wurde mit dem bisherigen v0.5-Paket verglichen und ist unverändert. Der aktualisierte vollständige ESP32-C3-Build und die Beispiel-Codegenerierung sind erfolgreich; physische Anwendung weiterhin offen.

Stand 08.11.2026. Zielhardware ESP32-C3, LWZ-Geräte-Firmware 2.x. Bisherige reale Nutzerbeobachtungen ausschließlich an LWZ 303 SOL mit FW 2.16 und deaktivierter Solarfunktion. Die neuen v0.5-Prüfungen erfolgen ohne angeschlossene Wärmepumpe, ohne Flashen, GitHub-Veröffentlichung oder Änderungen an Home Assistant.

## Ausgeführte Softwareprüfungen

- Die echten C++-Dateien `lwz2.cpp`, `v03.cpp`, `v04.cpp` und `profiles.cpp` werden gegen den lokalen UART-/Preferences-/ESPHome-Prüfrahmen kompiliert und ausgeführt.
- Bestehende Prüfungen für alle vier Decoderprofile, FD-Abfrage, Transport/Prüfsumme/Timeouts, Wiederholungen, 1-s-Pausen, Rückleseprüfung der Einzelwerte und Uhrzeitbedienung bleiben erfolgreich.
- F5/F6-Prüfungen bleiben erfolgreich: unabhängige 120-s-Nullphasen, Datenlücken, gespeicherte Fehler, Neustart/Historie, HA-Rücksetzung, Stufenprogramm und Übersteuerungsdaten. Die letzte angezeigte Lüfterstufe bleibt von der frischen Diagnosegrundlage getrennt.
- Neue Profilprüfungen: fünf Plätze, Vorschläge ohne automatischen Schreibauftrag, leere eigene Plätze, Entwurf/Verwerfen/Speichern, eindeutige Namen, Grenzen und 0,1-Raster, gespeicherte Auswahl und Profile nach Neustart.
- Erkennung aus tatsächlichen P13/P14/P15-Rohwerten: Änderung nur von P15 ergibt Benutzerdefiniert, passende Namen werden erkannt, mehrere gleiche Kombinationen gemeinsam angezeigt. Fehlende/unvollständige Werte sind unbekannt; veraltete Werte behalten ausdrücklich nur die letzte gelesene Zuordnung.
- Profilanwendung ändert P13/P14/P15 gemeinsam in einem frisch gelesenen Nutzdatenblock und erhält alle übrigen Bytes. Ein ungespeicherter Entwurf, fehlende Schreibfreigabe, leerer Platz, Speicherfehler oder paralleler Schreibvorgang verhindert die Anwendung.
- Der vollständige simulierte UART-Austausch GET → ein SET → GET bestätigt genau einen SET und drei erfolgreiche Transaktionen. Entzug der Schreibfreigabe vor SET bricht die Anwendung ab. Unsichere SET-Antwort führt zum GET, nicht zum erneuten SET. Unvollständige Parameterblöcke und abweichende Rücklesewerte bestätigen keine Anwendung. Die letzte erfolgreich gespeicherte Anwendung wird getrennt von der Erkennung geführt.
- Neustart während einer begonnenen Anwendung löst keinen weiteren SET aus. Profilspeicher und bisherige Uhr-/Fehlerhistorie bleiben über getrennte Schlüssel erhalten. Ungültiger Profilspeicher wird nicht ungeprüft angewendet.
- Speicherung des Profils und Speicherung einer erfolgreichen Anwendung werden auf Fehler geprüft; ein erfolgreicher LWZ-Readback bei anschließendem Speicherfehler wird gesondert gemeldet.
- Auswahl-/Konfigurationsvertrag: ursprünglicher 80-Einträge-Katalog erhalten, aktuelle Optional-Auswahl und Basis-/Expert-Zuordnung unverändert, Entitäts-Anzeigenamen ohne manuelles Gerätenamenpräfix (Änderung 09.11.2026), beide YAMLs auf 0.5.0 und ohne `enable_v03`/`enable_v04`.
- Migration geprüft: alte Versions-Aktivierungsoptionen werden vom Schema ausdrücklich abgewiesen.
- ESPHome 2026.10.2: beide YAML-Konfigurationen mit lokaler Komponentenquelle, synthetischen Zugangsdaten und ausdrücklich gewähltem Decoderprofil 2.16 validiert; C++-Codegenerierung einschließlich nativer Number-, Select- und Text-Entitäten erfolgreich.

## Vollständiger ESP32-C3-Build

Vollständiger ESP32-C3-Build mit ESPHome 2026.10.2, ESP-IDF 5.5.6 und synthetischen Zugangsdaten erfolgreich. Abschließender Build: 08.11.2026, 13:49:47 +0100; Konfigurationshash `0xf90de931`. Die sechs kompilierten Komponentenquellen einschließlich `profiles.cpp` wurden per SHA-256 mit dem aktuellen Quellstand verglichen. Der abschließende Build enthält keine Compilerwarnungen oder Fehler.

- RAM: **111.237 / 321.296 Byte (34,6 %)**.
- Programm-Flash: **1.076.769 / 1.835.008 Byte (58,7 %)**.
- Native Number-, Select- und Text-Entitäten sowie die lesbare Uptime wurden mitkompiliert.

Factory-/OTA-Prüfbinaries und ELF wurden nur lokal mit synthetischen Zugangsdaten erzeugt. Sie werden nicht ausgeliefert, veröffentlicht oder geflasht. Das Quellpaket enthält die beiden regulären YAMLs und eine lokale Beispielkonfiguration. Archivintegrität, SHA-256-Prüfsummen und unveränderte FHEM-/Herstellerreferenzen werden beim Packen geprüft.

## Offene Geräteprüfung

- Keine physische Profilanwendung oder Wirksamkeitsprüfung der Vorschläge durchgeführt. 0,2/2,0 K/0 und 0,4/1,0 K/0 sind Versuchsvorschläge, keine belegte Gebäudeoptimierung.
- P15-0,1-Auflösung und Block-05-Schreibweg sind im FHEM-2.x-Code belegt; erfolgreiche Übernahme durch die reale FW 2.16 bleibt durch Rücklesen und Gerätevergleich zu prüfen.
- Die gemeldeten Handshake-/Kommunikationsfehler sind nicht als behoben nachgewiesen. Die bereits übernommene Anpassung auf 1-s-Mindestpause/Retryabstand muss bei gleicher Betriebsdauer am Gerät verglichen werden. Längere Abfragezyklen oder Ausfälle können zu ausdrücklich veralteten Daten führen.
- Andere Gerätemodelle, aktive Solarfunktionen, F6-Übersteuerungspriorität und vollständige F5/F6-Fehlerfälle bleiben physisch nicht vollständig verifiziert. Die SOL-Stufenanzeige folgt weiterhin den akzeptierten FHEM-Kommentaren.
- Native Profilsteuerung und Speicherung wurden im Prüfrahmen und durch ESPHome-Codegenerierung geprüft; die tatsächliche HA-Oberfläche und das reale Geräteweb sind noch zu prüfen.

[Bedienung und Migration](HEIZKURVENPROFILE-v0.5.md). Historische Prüfungen: [v0.4](VALIDATION-v0.4.md), [v0.3](VALIDATION-v0.3.md).
