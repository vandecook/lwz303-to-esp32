# Validierung v0.6.0

Stand: 13.11.2026. Zielgerät ESP32-C3 / LWZ 303 SOL, reale Geräte-Firmware laut Nutzer 2.16. Der Nutzer bestätigt den Betrieb der bisherigen Funktionen. Für die neuen v0.6.0-Funktionen wurden keine Geräteaktionen, kein Flashen und kein GitHub-Publish ausgeführt.

## Erfolgreiche Softwareprüfungen

- Die echten C++-Komponentenquellen wurden mit Zig gegen den vorhandenen synthetischen ESPHome-/UART-/Preferences-Prüfrahmen kompiliert und ausgeführt. Alle bisherigen Tests für Transport, Prüfsummen, Timeout/Retry, vier Decoderprofile, Heizkurvenprofile, Uhrkorrektur, Schreibfreigabe und unabhängige Rückleseprüfung bestehen weiterhin.
- F5/F6-Tests bestehen weiterhin: frische Quellen, 120 Sekunden, Datenlücken, unabhängige Lüfter, gespeicherte Fehler und Neustarts. Gehaltene Anzeigewerte setzen den Gültigkeitsnachweis nicht fort.
- Neue v0.6-Tests: initial unbekannte Werte, Erhalt von Temperatur/Status/F2 bei Datenalterung, getrennte Frischeindikatoren, unabhängige History-Alterung, Wiederverbindung, serielle Wiederinitialisierung, Verwerfen alter Interpretation bei Decoderwechsel und Millisekundenüberlauf.
- Betriebsstundenprüfung: bekannte 16-Bit-Big-Endian-Werte 4660, 65535, 123 und 256 unverändert als Stunden dekodiert. `cooling_hours` ist in beiden ausgelieferten YAMLs nicht enthalten.
- Zweckprüfung: Heizung/Warmwasser/Abtauen anhand übereinstimmender F2-Anforderungscodes und frischem Verdichterstatus; widersprüchliche/unbekannte Codes und Code 5 bei laufendem Verdichter ergeben keine gültige Zweckzuordnung. Bei Datenlücken bleibt die letzte Anzeige erhalten und deren Gültigkeit wird zurückgenommen. Gleichzeitig angeforderte Stufen werden separat ausgewiesen.
- Beide Konfigurationen bestehen die Auswahl-/Konfigurationstests. ESPHome 2026.10.2 validiert und generiert beide Konfigurationen mit lokaler Komponentenquelle, Profil 2.16 und synthetischen Zugangsdaten erfolgreich. Projektangaben 0.6.0; neue Text-/Binärsensoren und F2-Rohcodes enthalten.
- Alle Entitätsnamen in beiden Hauptkonfigurationen sind ohne Schrägstrich; der vorgemerkte Profil-Editorstatus wurde umbenannt.

## Vollständiger ESP32-C3-Build

Vollständiger ESP32-C3-Build mit ESPHome 2026.10.2 und ESP-IDF 5.5.6 erfolgreich. Die sechs kompilierten Komponentenquellen wurden bytegenau mit dem ausgelieferten Quellstand abgeglichen. Keine Compilerwarnungen oder Fehler im abschließenden Build.

- RAM: **112197 / 321296 Byte**.
- Programm-Flash: **1079799 / 1835008 Byte**.
- Build: `config_hash=0xb2ee50f0 build_time_str=2026-11-13 11:31:20 +0100`.

Der Build verwendet synthetische Zugangsdaten. Prüfbinaries werden nicht mit dem Quellpaket ausgeliefert und wurden nicht geflasht. Das Paket enthält beide regulären YAMLs und lokale Varianten, einen Änderungsverlauf, SHA-256-Prüfsummen und unveränderte FHEM-/Herstellerreferenzen. Archivintegrität und Dateiprüfsummen werden beim Packen geprüft.

## Grenzen und noch offene Geräteprüfung

- Die F2-Zweckzuordnung stammt aus FHEM-Kommentaren und ist als vorläufig markiert; reale Umschaltungen und Rohcodes auf FW 2.16 prüfen. Gleichzeitig angeforderte Stufen beweisen keine gleichzeitige Wärmelieferung.
- Die eigentliche Ursache der gemeldeten seriellen Kommunikationsprobleme wurde durch diese Anzeigeänderung nicht behoben. Ausfälle der Verbindung ESP32/HA können weiterhin zu „nicht verfügbar“ führen.
- Neue Frischeanzeigen und Rückkehr nach realen Datenlücken im Geräteweb und in HA prüfen. HA-Auswertungen müssen die neue Gültigkeit beachten, statt aus gehaltenen Statuswerten Laufzeit oder Starts abzuleiten.
- Betriebsstundenzähler 1/2 sind absichtlich neutral wie in FHEM benannt. Keine echten Verdichterstart-/Zusatzheizungszähler ohne belegte Quelle ergänzt.
- Keine neuen Temperatur-/Sprung-/Stillstandsgrenzen festgelegt. Negative WP-Spreizung allein wird nicht als Fehler gewertet. Weitere Plausibilitätsprüfungen bleiben für spätere 0.6.x offen.
- Vorhandene HA-Entitätsregistrierung nach Umbenennung des Profil-Editorstatus prüfen. Bestehende Speicherformate sind unverändert.

[Bedienung und Datenquellen](BETRIEBSDATEN-v0.6.md). Historischer Prüfstand: [v0.5](VALIDATION-v0.5.md).
