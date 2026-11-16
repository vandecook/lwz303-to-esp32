# Validierung v0.7.0

Stand: 16.11.2026. Ziel: ESP32-C3 mit ESPHome 2026.10.2 und LWZ303 Firmware 2.16.

## Softwareprüfungen

- Python: bestehende Auswahlprüfung (2 Tests), Webressourcenprüfung (4 Tests) und neue Parameter-/Identitätsprüfung (2 Tests) erfolgreich.
- Alle bestehenden C++-Testgruppen sowie neue Parametertests erfolgreich: 31 Schreibfelder in allen vier Decoderprofilen, untere/obere Grenzen, Schrittweite, ungültige Werte, Auftragskollisionen, vollständiger Erhalt anderer Bytes, kurze Antworten, unklare SET-Antwort mit Rücklesen und Abbruch bei Abweichung.
- Zusätzlich vollständiger synthetischer UART-Ablauf für einen neuen Block-07-Schreibauftrag einschließlich Escaping, ACKs und unabhängigem Rücklesen; genau ein SET.
- Modellabhängige Felder, SOL-Abfrage, P25-Sperre für i, Vorzeichen bei P78, Datenlücken, Wiederinitialisierung und Decoderwechsel geprüft. Nur lesbare Felder und P86 lassen sich nicht über den neuen Schreiber setzen.
- Beide YAML-Konfigurationen mit lokaler Komponentenquelle und synthetischen Zugangsdaten erfolgreich generiert. Isolierter externer Komponenten-Checkout ohne Hauptordner `web` erfolgreich; eingebettete JS-Datei bytegenau verglichen.
- Bestehende Entitätsnamen, explizite IDs, Gerätebezeichnungen, Domänen, Standardaktivierung und Kategorien werden gegen den vor der Erweiterung gesicherten 0.6.2-Bestand geprüft. Nur die Webgruppierung der Parameter wurde geändert.

## Vollständiger ESP32-C3-Build

Vollständiger ESP32-C3-Build mit ESPHome 2026.10.2 und ESP-IDF 5.5.6 erfolgreich. Alle kompilierten Komponentenquellen und Header stimmen bytegenau mit dem ausgelieferten Stand überein. Neue Parameterentitäten und bestehende Abtauentitäten sind im generierten Programm registriert. Die eingebettete Webressource wurde entpackt und bytegenau verglichen.

- RAM: **116133 / 321296 Byte**.
- Programm-Flash: **1101795 / 1835008 Byte**.
- Compilerwarnungen: **0**.
- Build: `config_hash=0x3b4a21ee build_time_str=2026-11-16 20:51:23 +0100`.

Build mit synthetischen Zugangsdaten; kein Flashen und keine Auslieferung dieser Prüfbinaries.

## Grenzen

Die neuen Parameterzugriffe wurden nicht am echten Gerät ausgeführt, nicht geflasht und nicht auf GitHub veröffentlicht. Der Nutzer hat die Abtau-Integration aus 0.6.2 am 16.11.2026 bestätigt; das ist keine pauschale Bestätigung der neuen Parameter-Schreibwege.

**P86 bleibt offen:** Keine belegte Zuordnung im 2.x-Protokoll. Von 32 angeforderten einstellbaren Parametern sind 31 umgesetzt. Weitere nur lesbare Referenzfelder und ihre tatsächliche Verfügbarkeit an FW 2.16 sind am Gerät zu prüfen. Bestehende UI-Grenzen bleiben erhalten; neue Grenzen sind aus der FHEM-Referenz übernommen, P31 konservativ 0–3. Details: [Bedienung und Parameterkatalog](PARAMETER-v0.7.md).

Das Quellpaket enthält synthetisch geprüfte Konfigurationen und Komponenten, keine Test-Firmware mit Platzhalterzugangsdaten. Bestehende Ausgaben bleiben erhalten.
