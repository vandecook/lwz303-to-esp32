# Changelog

## 0.3.0 – 2026-11-06

- Nach Nutzerkorrektur OUT063/OUT064 (Zuluft/Abluft Speed200) als Ja standardmäßig aktiv ergänzt: FHEM uint16 BE /10, eine Nachkommastelle, keine unbestätigte physikalische Einheit.

- Benutzergeänderte Auswahl übernommen: 33 Ja und 47 Optional; Dateihash und 80 Einträge in `components/lwz2/selection.json`.
- F3, EE, FC und 17 in die serielle Abfrageplanung aufgenommen; ausgewählte F4-/FB-Felder mit profilierten FHEM-Offsets ergänzt. Vorhandene E8-/09-/05-Parser weiterverwendet.
- Optionale Entitäten vollständig mit erzeugt, in Home Assistant zunächst deaktiviert; Basis-/Expert-Gruppen im Geräteweb. Nicht gewählte Entitäten aus den beiden Beispiel-/Nutzerkonfigurationen entfernt.
- Unabhängige Global- und HK1-Decoderlayouts; Raum-/Stufenwerte ohne belegtes Layout bleiben ungültig. FB bleibt alleinige Quelle gemeinsamer Mess-/Aktorzustände; kein F4-Überschreiben.
- Pro Block Ablauf-/Längenprüfung für neue Werte, separate Innentemperaturquellen und Unknown statt erfundener Nullwerte bei fehlenden Daten.
- Vier Number-Sollwerte P01/P02/P04/P05 und manuelle Uhrbuttons ±1h; Schreibfreigabe nach Boot aus. Frischer Read–Modify–Write, vollständiges Rücklesen, kein blindes SET-Retry und keine doppelte Zeitverschiebung.
- UI-Grenzen 15–25 °C Raum und 35–55 °C WW ausdrücklich Vorschläge. Kalender-/Tageswechsel abgelehnt; weitere Schreibbereiche unverändert ausgeschlossen.
- Hosttests für Decodermappings, Blockalterung, alle vier Schreibfelder, ACK-Verlust, Fremdfeldänderung, Uhrvergleich, Tageswechsel und Freigaberücknahme ergänzt. Hardwaretest und kompletter ESP32-Build weiter offen.

## 0.2.0 – 2026-11-03

### Detaillierte Deltas v0.1 → v0.2

- Minimalbeispiel auf vier Temperaturen und drei Zustände reduziert; Heizbetrieb/WW folgen weiterhin den Pumpenbits. Vorhandene optionale Entitäten bleiben im Schema.
- FB-Normalintervall 5 → 10 s, dynamisch 5 s mit ODER-Triggern Kompressor/Heizen/WW. Beide Intervalle, Trigger und Dynamik ein/aus über persistente Template-Entitäten konfigurierbar.
- Lesender Block 05 für HK1 P13 Gradient/P14 Fußpunkt ergänzt: alle drei Profile, Big Endian / 10, Boot + 60 min, Intervall einstellbar und manueller Refresh mit Warteschlange.
- Erfolgs-, endgültiger Fehler-, Timeout- und Retryzähler ergänzt, lokaler Reset ohne Einfluss auf Recovery/Backoff. Letzter Erfolg und HK1-Aktualisierung als Zeitpunkte seit Boot.
- FD-Firmwareabfrage ausschließlich für das quellenseitig belegte Profil 2.06, einmal bei Boot/Profilwechsel. Keine unbestätigte Abfrage bei 2.14/2.14j; manuelle persistente Auswahl bleibt maßgeblich.
- Lokal eingebetteter ESPHome Web Server v3 mit Authentifizierung, gecachten Werten, Diagnose, Uptime, ESP-Uhrzeit nach SNTP und Live-Logs. Keine seriellen Abfragen durch Seitenaufrufe.
- RS232-Neuinitialisierung und ESP-Neustart ergänzt. Neustart benötigt separate Freigabe, die nach 30 s verfällt.
- UART-Beispiel RX16/TX17 → RX5/TX6 bei 9600 8N1. Beispielboard esp32dev → esp32-c3-devkitm-1 gemäß vom Anwender bestätigtem ESP32-C3-Aufbau.
- F4/E8/01/09 nur noch bei passenden konfigurierten Verbrauchern; bestehende Intervalloptionen bleiben verfügbar. Minimalbeispiel fragt keinen dieser Blöcke ab.
- DLE/2B18-Transport, drei Versuche, 1800-ms-Timeouts, 250-ms-Retryabstand und Fehler-Backoff erhalten.
- Empfangsframe auf 512 Byte und UART-Verarbeitung pro Schleife auf 512 Byte begrenzt; checksum-gültige verkürzte Payloads werden abgewiesen.
- Stale-Erkennung anhand FB-Alter, mit mindestens dreifachem effektivem Intervall. Konfigurationserfolg stellt FB-Kommunikationsstatus nicht wieder her.
- README aktualisiert; ROADMAP 0.1–0.11, VALIDATION und Softwaretests ergänzt. Neue Hardwarefunktionen ausdrücklich noch nicht am Gerät getestet.
- Keine LWZ-Parameterschreibfunktion; solche Funktionen frühestens ab v0.5.



## 0.1.0

- Initial read-only release.
- Firmware profiles 2.06, 2.14 and 2.14j.
- Non-blocking DLE communication state machine.
- FB 5 s status poll with pump/compressor states.
- Firmware-specific F4 parsing.
- E8 detailed fan parsing.
- Firmware-specific 01 fan configuration parsing.
- 09 operating-hour parsing.
- Retry, timeout, backoff and stale-data handling.
- Optional raw-frame text sensors and VERBOSE raw logging.
- Runtime firmware dropdown via ESPHome template select.
